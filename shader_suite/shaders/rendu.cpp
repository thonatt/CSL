#include "rendu.h"

/**
 Shaders from the Rendu graphics engine, by Simon Rodriguez.
 https://github.com/kosua20/Rendu
 */

csl::glsl::compute_430::Shader scattering_lookup_table()
{
	using namespace csl::glsl::compute_430;
	using namespace csl::swizzles::xyzw;

	Shader shader;

	Qualify<layout<rgba32f, binding<0>>, uniform, image2D> output;

	shader_stage_option<layout<local_size_x<16>, local_size_y<16>>, in>();

	const Float groundRadius = 6371e3;
	const Float topRadius = 6471e3;
	const vec3 kRayleigh = vec3(5.5e-6, 13.0e-6, 22.4e-6) << "kRayleigh";
	const Float heightRayleigh = 8000.0;
	const Float heightMie = 1200.0;
	const Float kMie = Float(21e-6) << "kMie";

	auto intersects = define_function<Bool>("intersects", [&](const vec3 rayOrigin, const vec3 rayDir, Float radius, Qualify<vec2, out> roots) {
		const Float a = dot(rayDir, rayDir);
		const Float b = dot(rayOrigin, rayDir);
		const Float c = dot(rayOrigin, rayOrigin) - radius * radius;
		const Float delta = b * b - a * c;
		// No intersection if the polynome has no real roots.
		CSL_IF(delta < 0.0) {
			CSL_RETURN(false);
		}
		// If it intersects, return the two roots.
		const Float dsqrt = sqrt(delta);
		roots = (-b + vec2(-dsqrt, dsqrt)) / a;
		CSL_RETURN(true);
		});

	shader.main([&] {
		const ivec2 resolution = imageSize(output);

		const Uint samplesCount = Uint(256) << "samplesCount";

		const Int i = gl_GlobalInvocationID(y);
		const Int j = gl_GlobalInvocationID(x);

		// Move to 0,1.
		// No need to take care of the 0.5 shift as we are working with indices
		const Float xf = Float(j) / (Float(resolution(x)) - 1.0);
		const Float yf = Float(i) / (Float(resolution(y)) - 1.0);
		// Position and ray direction.
		// x becomes the height
		// y become the cosine
		const vec3 currPos = vec3(0.0, (topRadius - groundRadius) * xf + groundRadius, 0.0);
		const Float cosA = 2.0 * yf - 1.0;
		const Float sinA = sqrt(1.0 - cosA * cosA);
		const vec3 sunDir = -normalize(vec3(sinA, cosA, 0.0));
		// Check when the ray leaves the atmosphere.
		vec2 interSecondTop;
		const Bool didHitSecondTop = intersects(currPos, sunDir, topRadius, interSecondTop);
		// Divide the distance traveled through the atmosphere in samplesCount parts.
		Float secondStepSize = CSL_TERNARY(didHitSecondTop, interSecondTop(y) / Float(samplesCount), 0.0);

		// Accumulate optical distance for both scatterings.
		Float rayleighSecondDist = 0.0;
		Float mieSecondDist = 0.0;

		// March along the secondary ray.
		CSL_FOR(Uint k = 0; k < samplesCount; ++k) {
			// Compute the current position along the ray, ...
			const vec3 currSecondPos = currPos + (Float(k) + 0.5) * secondStepSize * sunDir;
			// ...and its distance to the ground (as we are in planet space).
			const Float currSecondHeight = length(currSecondPos) - groundRadius;
			// Compute density based on the characteristic height of Rayleigh and Mie.
			const Float rayleighSecondStep = exp(-currSecondHeight / heightRayleigh) * secondStepSize;
			const Float mieSecondStep = exp(-currSecondHeight / heightMie) * secondStepSize;
			// Accumulate optical distances.
			rayleighSecondDist += rayleighSecondStep;
			mieSecondDist += mieSecondStep;
		}

		// Compute associated attenuation.
		const vec3 secondaryAttenuation = exp(-(kMie * mieSecondDist + kRayleigh * rayleighSecondDist));
		imageStore(output, ivec2(j, i), vec4(secondaryAttenuation, 1.0));
		});

	return shader;
}

csl::glsl::frag_420::Shader atmosphere_rendering()
{
	using namespace csl::glsl::frag_420;
	using namespace csl::swizzles::all;

	Shader shader;

	Qualify<in, vec2> uv("uv");

	Qualify<uniform, mat4> clipToWorld = mat4(vec4(1.056, vec3(0.0)), vec4(0.0, 0.792, 0.0, 0.0), vec4(vec3(0.0), -4.995), vec4(0.0, 0.0, -1.0, 5.005)) << "clipToWorld";
	Qualify<uniform, vec3> viewPos = vec3(0.0, 0.0, 0.0); ///< The position in view space.
	Qualify<uniform, vec3> lightDirection = vec3(0.437, 0.082, -0.896) << "sun_direction"; ///< The light direction in world space.
	Qualify<uniform, Float> altitude = 1.0; ///< Origin height above the planet surface.

	Qualify<layout<binding<0>>, uniform, sampler2D> precomputedScattering("precomputedScattering"); ///< Secondary scattering lookup table.
	Qualify<layout<location<0>>, out, vec4> fragColor; ///< Atmosphere color.

	const Float atmosphereGroundRadius = 6371e3; ///< Radius of the planet.
	const Float atmosphereTopRadius = 6471e3; ///< Radius of the atmosphere.
	const Float sunIntensity = 20.0; ///< Sun intensity.
	const vec3 defaultSunColor = vec3(1.474, 1.8504, 1.91198); ///< Sun direct color.
	const vec3 kRayleigh = vec3(5.5e-6, 13.0e-6, 22.4e-6); ///< Rayleigh coefficients.
	const Float kMie = 21e-6; ///< Mie coefficients.
	const Float heightRayleigh = 8000.0; ///< Mie characteristic height.
	const Float heightMie = 1200.0; ///< Mie characteristic height.
	const Float gMie = 0.758; ///< Mie g constant.
	const Float sunAngularRadius = 0.04675; ///< Sun angular radius.
	const Float sunAngularRadiusCos = 0.998; ///< Cosine of the sun angular radius.

	constexpr int SAMPLES_COUNT_ATMO = 16;
	constexpr double PI = 3.1415926536;

	/** Check if a sphere of a given radius is intersected by a ray defined by an
		origin wrt to the sphere center and a normalized direction.
		\param rayOrigin the origin of the ray
		\param rayDir the direction of the ray (normalized)
		\param radius the radius of the sphere to intersect
		\param roots will contain the two roots of the associated polynomial, ordered.
		\return true if there is intersection.
		\warning The intersection can be in the negative direction along the ray. Check the sign of the roots to know.
	*/
	auto intersects = define_function<Bool>("intersects", [&](vec3 rayOrigin, vec3 rayDir, Float radius, Qualify<vec2, out> roots) {
		Float a = dot(rayDir, rayDir);
		Float b = dot(rayOrigin, rayDir);
		Float c = dot(rayOrigin, rayOrigin) - radius * radius;
		Float delta = b * b - a * c;
		// No intersection if the polynome has no real roots.
		CSL_IF(delta < 0.0) {
			CSL_RETURN(false);
		}
		// If it intersects, return the two roots.
		Float dsqrt = sqrt(delta);
		roots = (-b + vec2(-dsqrt, dsqrt)) / a;
		CSL_RETURN(true);
		});

	/** Compute the Rayleigh phase.
		\param cosAngle Cosine of the angle between the ray and the light directions
		\return the phase
	*/
	auto rayleighPhase = define_function<Float>("rayleighPhase", [&](Float cosAngle) {
		const Float k = 1.0 / (4.0 * PI);
		CSL_RETURN(k * 3.0 / 4.0 * (1.0 + cosAngle * cosAngle));
		});

	/** Compute the Mie phase.
		\param cosAngle Cosine of the angle between the ray and the light directions
		\return the phase
	*/
	auto miePhase = define_function<Float>("miePhase", [&](Float cosAngle) {
		const Float k = 1.0 / (4.0 * PI);
		Float g2 = gMie * gMie;
		CSL_RETURN(k * 3.0 * (1.0 - g2) / (2.0 * (2.0 + g2)) * (1.0 + cosAngle * cosAngle) / pow(1.0 + g2 - 2.0 * gMie * cosAngle, 3.0 / 2.0));
		});

	/** Compute the radiance for a given ray, based on the atmosphere scattering model.
		\param rayOrigin the ray origin
		\param rayDir the ray direction
		\param sunDir the light direction
		\param sunColor the color of the sun disk itself
		\param scatterTable the precomputed secondary scattering lookup table
		\return the estimated radiance
	*/
	auto computeAtmosphereRadiance = define_function<vec3>("computeAtmosphereRadiance", [&](vec3 rayOrigin, vec3 rayDir, vec3 sunDir, vec3 sunColor, sampler2D scatterTable) {
		// Check intersection with atmosphere.
		vec2 interTop, interGround;
		Bool didHitTop = intersects(rayOrigin, rayDir, atmosphereTopRadius, interTop);
		// If no intersection with the atmosphere, it's the dark void of space.
		CSL_IF(!didHitTop) {
			CSL_RETURN(vec3(0.0));
		}
		// Now intersect with the planet.
		Bool didHitGround = intersects(rayOrigin, rayDir, atmosphereGroundRadius, interGround);
		// Distance to the closest intersection.
		Float distanceToInter = min(interTop(y), CSL_TERNARY(didHitGround, interGround(x), 0.0));
		// Divide the distance traveled through the atmosphere in SAMPLES_COUNT_ATMO parts.
		Float stepSize = (distanceToInter - interTop(x)) / Float(SAMPLES_COUNT_ATMO);
		// Angle between the sun direction and the ray.
		Float cosViewSun = dot(rayDir, sunDir);

		// Accumulate optical distance for both scatterings.
		Float rayleighDist = 0.0;
		Float mieDist = 0.0;
		// Accumulate contributions for both scatterings.
		vec3 rayleighScatt = vec3(0.0);
		vec3 mieScatt = vec3(0.0);
		vec3 transmittance = vec3(0.0);

		// March along the ray.
		CSL_FOR(Int i = 0; i < SAMPLES_COUNT_ATMO; ++i) {
			// Compute the current position along the ray, ...
			vec3 currPos = rayOrigin + (Float(i) + 0.5) * stepSize * rayDir;
			// ...and its distance to the ground (as we are in planet space).
			Float currHeight = length(currPos) - atmosphereGroundRadius;
			// ... there is an artifact similar to clipping when close to the planet surface if we allow for negative heights.
			CSL_IF(i == (SAMPLES_COUNT_ATMO - 1) && currHeight < 0.0) {
				currHeight = 0.0;
			}
			// Compute density based on the characteristic height of Rayleigh and Mie.
			Float rayleighStep = exp(-currHeight / heightRayleigh) * stepSize;
			Float mieStep = exp(-currHeight / heightMie) * stepSize;
			// Accumulate optical distances.
			rayleighDist += rayleighStep;
			mieDist += mieStep;

			vec3 directAttenuation = exp(-(kMie * (mieDist)+kRayleigh * (rayleighDist)));

			// The secondary attenuation lookup table is parametrized by
			// the height in the atmosphere, and the cosine of the vertical angle with the sun.
			Float relativeHeight = (length(currPos) - atmosphereGroundRadius) / (atmosphereTopRadius - atmosphereGroundRadius);
			Float relativeCosAngle = -0.5 * sunDir(y) + 0.5;
			// Compute UVs, scaled to read at the center of pixels.
			vec2 attenuationUVs = (1.0 - 1.0 / 512.0) * vec2(relativeHeight, relativeCosAngle) + 0.5 / 512.0;
			vec3 secondaryAttenuation = texture(scatterTable, attenuationUVs)(r, g, b);

			// Final attenuation.
			vec3 attenuation = directAttenuation * secondaryAttenuation;
			// Accumulate scatterings.
			rayleighScatt += rayleighStep * attenuation;
			mieScatt += mieStep * attenuation;
			transmittance += directAttenuation;
		}

		// Final scattering participations.
		vec3 rayleighParticipation = kRayleigh * rayleighPhase(cosViewSun) * rayleighScatt;
		vec3 mieParticipation = kMie * miePhase(cosViewSun) * mieScatt;

		// The sun itself if we're looking at it.
		vec3 sunRadiance = vec3(0.0);
		Bool didHitGroundForward = didHitGround && interGround(y) > 0.0;
		CSL_IF(!didHitGroundForward && dot(rayDir, sunDir) > sunAngularRadiusCos) {
			sunRadiance = sunColor / (PI * sunAngularRadius * sunAngularRadius);
		}

		CSL_RETURN(sunIntensity * (rayleighParticipation + mieParticipation) + transmittance * sunRadiance);
		});

	/** Simulate sky color based on an atmospheric scattering approximate model. */
	shader.main([&] {
		// Move to -1,1
		vec4 clipVertex = vec4(-1.0 + 2.0 * uv, 0.0, 1.0) << "clipVertex";
		// Then to world space.
		vec3 viewRay = normalize((clipToWorld * clipVertex)(x, y, z));
		// We then move to the planet model space, where its center is in (0,0,0).
		vec3 planetSpaceViewPos = viewPos + vec3(0.0, atmosphereGroundRadius, 0.0) + vec3(0.0, altitude, 0.0);
		vec3 atmosphereColor = computeAtmosphereRadiance(planetSpaceViewPos, viewRay, lightDirection, defaultSunColor, precomputedScattering);
		atmosphereColor = 1.0 - exp(-1.0 * atmosphereColor);
		fragColor = vec4(pow(atmosphereColor, vec3(1.0 / 2.2)), 1.0);
		});

	return shader;
}