#include "frags.h"

#include <csl/Shaders.h>

std::string blurShader()
{
	using namespace csl::frag_450;
	using namespace csl::swizzles::all;
	
	Shader shader;

	GL_INTERFACE_BLOCK(In<>, Interface, In, ,
		(vec2) uv
	);

	Uniform<sampler2D, Layout<Binding<0>>> sceenTexture("screenTexture");
	Uniform<vec2> fetchOffset("fetchOffset");
	Out<vec3> fragColor("fragColor");

	shader.main([&] {
		vec3 col = texture(sceenTexture, In.uv)[r, g, b] * Float(5.0) / 16.0 << "col";
		col += texture(sceenTexture, In.uv - fetchOffset)[r, g, b] * Float(5.0) / 16.0;
		col += texture(sceenTexture, In.uv + fetchOffset)[r, g, b] * Float(5.0) / 16.0;
		fragColor = col;;
		
	});

	return shader.str();
}

std::string ambiantShader() 
{
	using namespace csl::frag_450;
	using namespace csl::swizzles::all;

	Shader shader;

	double INV_M_PI = 0.3183098862;
	double M_PI = 3.1415926536;
	double M_INV_LOG2 = 1.4426950408889;

	uint SAMPLES_COUNT = 16u;
	int MAX_LOD = 5;

	//GL_INTERFACE(In<>, Interface, (vec2) uv) in("In");

	GL_INTERFACE_BLOCK(In<>, Interface, In, , (vec2) uv);

	Uniform<sampler2D, Layout<Binding<0> > > albedoTexture("albedoTexture"); 
	Uniform<sampler2D, Layout<Binding<1> > > normalTexture("normalTexture");
	Uniform<sampler2D, Layout<Binding<2> > > effectsTexture("effectsTexture");
	Uniform<sampler2D, Layout<Binding<3> > > depthTexture("depthTexture");
	Uniform<sampler2D, Layout<Binding<4> > > ssaoTexture("ssaoTexture");
	Uniform<samplerCube, Layout<Binding<5> > > textureCubeMap("textureCubeMap");
	Uniform<sampler2D, Layout<Binding<6> > > brdfPrecalc("brdfPrecalc");

	Array<Uniform<vec3>, 9> shCoeffs("shCoeffs");
	Uniform<mat4> inverseV("inverseV");
	Uniform<vec4> projectionMatrix("projectionMatrix");

	Out<vec3, Layout<Location<0>> > fragColor("fragColor");

	auto positionFromDepth = makeFunc<vec3>("positionFromDepth", [&](Float depth) {
		Float depth2 = 2.0 * depth - 1.0 << "depth2";
		vec2 ndcPos = 2.0 * In.uv - 1.0 << "ndcPos";
		Float viewDepth = -projectionMatrix[w] / (depth2 + projectionMatrix[z]) << "viewDepth";
		GL_RETURN(vec3(-ndcPos * viewDepth / projectionMatrix[x, y], viewDepth));
	}, "depth");

	auto ggx = makeFunc<vec3>("ggx", [&](vec3 n, vec3 v, vec3 F0, Float roughness) {
		Float NdotV = max(0.0, dot(v, n)) << "NdotV";
		vec3 ref = -reflect(v, n) << "r";
		ref = normalize((inverseV * vec4(ref, 0.0))[x,y,z]);
		vec2 brdfParams = texture(brdfPrecalc, vec2(NdotV, roughness))[r, g] << "brdfParams";
		vec3 specularColor = textureLod(textureCubeMap, ref, MAX_LOD * roughness)[r, g, b] << "specularColor";
		GL_RETURN(specularColor * (brdfParams[x] * F0 + brdfParams[y]));
	}, "n", "v", "F0", "roughness");


	auto applySH = makeFunc<vec3>("applySH", [&](vec3 wn) {
		GL_RETURN((shCoeffs[7] * wn[z] + shCoeffs[4] * wn[y] + shCoeffs[8] * wn[x] + shCoeffs[3]) * wn[x] +
			(shCoeffs[5] * wn[z] - shCoeffs[8] * wn[y] + shCoeffs[1]) * wn[y] +
			(shCoeffs[6] * wn[z] + shCoeffs[2]) * wn[z] +
			shCoeffs[0]);
	}, "wn");

	shader.main([&] {
		vec4 albedoInfo = texture(albedoTexture, In.uv) << "albedoInfo";

		GL_IF (albedoInfo[a] == 0) {
			fragColor = albedoInfo[r,g,b];
			GL_RETURN;
		}
		lineBreak();

		vec3 baseColor = albedoInfo[r, g, b] << "baseColor";
		vec3 infos = texture(effectsTexture, In.uv)[r, g, b] << "infos";
		Float roughness = max(0.045, infos[r]) << "roughness";
		Float metallic = infos[g] << "metallic";
		Float depth = texture(depthTexture, In.uv)[r] << "depth";
		vec3 position = positionFromDepth(depth) << "position";
		vec3 n = normalize(2.0 * texture(normalTexture, In.uv)[r, g, b] - 1.0) << "n";
		vec3 v = normalize(-position) << "v";
		
		lineBreak();

		Float precomputedAO = infos[b] << "precomputedAP";
		Float realtimeAO = texture(ssaoTexture, In.uv)[r] << "realtimeAO";
		Float ao = realtimeAO * precomputedAO << "ao";

		lineBreak();

		vec3 worldNormal = normalize(vec3(inverseV * vec4(n, 0.0))) << "worldNormal";
		vec3 envLighting = applySH(worldNormal) << "envLighting";

		vec3 F0 = mix(vec3(0.08), baseColor, metallic) << "F0";

		vec3 diffuse = (1.0 - metallic) * baseColor * (1.0 - F0) * envLighting << "diffuse";

		vec3 specular = ggx(n, v, F0, roughness) << "specular";

		lineBreak();

		fragColor = ao * (diffuse + specular);
	});

	return shader.str();
}

std::string ssaoShader()
{
	using namespace csl::frag_330;
	using namespace csl::swizzles::all;

	Shader shader;

	GL_INTERFACE_BLOCK(In<>, Interface, in, , (vec2) uv);

	Uniform<sampler2D, Layout<Binding<0>>> depthTexture("depthTexture");
	Uniform<sampler2D, Layout<Binding<1>>> normalTexture("normalTexture");
	Uniform<sampler2D, Layout<Binding<2>>> noiseTexture("noiseTexture");

	Uniform<mat4> projectionMatrix("projectionMatrix");
	Array<Uniform<vec3>, 24> samples("samples");

	Uniform<Float> radius = Float(0.5) << "radius";
	
	Out<Float, Layout<Location<0>>> fragColor("fragColor");

	auto linearizeDepth = makeFunc<Float>("linearizeDepth", [&](Float depth) {
		Float depth2 = 2.0*depth - 1.0 << "depth2";
		Float viewDepth = -projectionMatrix[3][2] / (depth2 + projectionMatrix[2][2]) << "viewDepth";
		GL_RETURN(viewDepth);
	}, "depth");

	auto positionFromUV = makeFunc<vec3>("positionFromUV", [&](vec2 uv) {
		Float depth = texture(depthTexture, uv)[r] << "depth";
		Float viewDepth = linearizeDepth(depth) << "viewDepth";
		vec2 ndcPos = 2.0 * uv - 1.0 << "ndcPos";
		GL_RETURN(vec3(-ndcPos * viewDepth / vec2(projectionMatrix[0][0], projectionMatrix[1][1]), viewDepth));
	}, "uv");

	shader.main([&]{
		vec3 n = normalize(2.0 * texture(normalTexture, in.uv)[r, g, b] - 1.0) << "n";

		GL_IF (length(n) < 0.1) {
			fragColor = 1.0;
			GL_RETURN;
		}

		vec3 randomOrientation = texture(noiseTexture, gl_FragCoord[x, y] / 5.0)[r, g, b] << "randomOrientation";

		vec3 t = normalize(randomOrientation - n * dot(randomOrientation, n)) << "t";
		vec3 b = normalize(cross(n, t)) << "b";
		mat3 tbn = mat3(t, b, n) << "tbn";

		vec3 position = positionFromUV(in.uv) << "position";

		Float occlusion = Float(0.0) << "occlusion";
		GL_FOR(Int i = Int(0) << "i"; i < 24; ++i) {

			vec3 randomSample = position + radius * tbn * samples[i] << "randomSample";

			vec4 sampleClipSpace = projectionMatrix * vec4(randomSample, 1.0) << "sampleClipSpace";
			vec2 sampleUV = (sampleClipSpace[x, y] / sampleClipSpace[w]) * 0.5 + 0.5 << "sampleUV";

			Float sampleDepth = linearizeDepth(texture(depthTexture, sampleUV)[r]) << "sampleDepth";
			Float isValid = GL_TERNARY(abs(position[z] - sampleDepth) < radius, 1.0, 0.0) << "isValid";

			occlusion += GL_TERNARY(sampleDepth >= randomSample[z], isValid, 0.0);
		}

		occlusion = 1.0 - (occlusion / 24.0);
		fragColor = pow(occlusion, 2.0);
	});

	return shader.str();
}

std::string discardFragShader()
{
	using namespace csl::frag_410;
	using namespace csl::swizzles::all;

	Shader shader;

	In<vec3> FrontColor("FrontColor");
	In<vec3> BackColor("BackColor");
	In<vec2> TexCoord("TexCoord");
	
	Out<vec4, Layout<Location<0>>> FragColor("FragColor");

	shader.main([&]() {
		Float scale = Float(15.0) << "scale";
		bvec2 toDiscard = greaterThan(fract(TexCoord*scale), vec2(0.2, 0.2)) << "toDiscard";
		GL_IF(all(toDiscard)) {
			GL_DISCARD;
		} GL_ELSE{
			GL_IF(gl_FrontFacing) {
				FragColor = vec4(FrontColor, 1.0);
			}
		GL_ELSE {
				FragColor = vec4(BackColor, 1.0);
			}
		}
	});

	return shader.str();
}

std::string eightiesShader(){
	using namespace csl::frag_410;
	using namespace csl::swizzles::all;
	
	Shader shader;
	
	/// Uniforms and output.
	Uniform<vec2> iResolution("iResolution");
	Uniform<Float> iTime("iTime");
	Uniform<sampler2D> iChannel0("iChannel0");
	Out<vec4, Layout<Location<0>>> fragColor("fragColor");
	
	/// Noise helpers.
	auto noise1D = makeFunc<Float>([&](Float p) {
		Float fl = floor(p);
		Float fc = fract(p);
		Float rand0 = fract(sin(fl) * 43758.5453123);
		Float rand1 = fract(sin(fl+1.0) * 43758.5453123);
		GL_RETURN(mix(rand0, rand1, fc));
	});
	
	auto noise4D = makeFunc<vec4>([&](vec4 p) {
		vec4 fl = floor(p);
		vec4 fc = fract(p);
		vec4 rand0 = fract(sin(fl) * 43758.5453123);
		vec4 rand1 = fract(sin(fl+1.0) * 43758.5453123);
		GL_RETURN(mix(rand0, rand1, fc));
	});
	
	auto hash = makeFunc<Float>([&](vec2 p){
		vec3 p2 = vec3(p[x,y], p[x]);
		vec3 p3  = fract(p2 * 0.2831);
		p3 += dot(p3, p3[y,z,x] + 19.19);
		GL_RETURN(fract((p3[x] + p3[y]) * p3[z]));
	});
	
	
	/// Background utilities.
	auto stars = makeFunc<Float>([&](vec2 localUV, Float starsDens, Float starsDist){
		// Center and scale UVs.
		vec2 p = (localUV - 0.5) * starsDist;
		// Use thresholded high-frequency noise.
		Float brightness = smoothstep(1.0 - starsDens, 1.0, hash(floor(p)));
		// Apply soft transition between the stars and the background.
		const Float startsTh = 0.5;
		GL_RETURN(smoothstep(startsTh, 0.0, length(fract(p) - 0.5)) * brightness);
	});
	
	auto segmentDistance = makeFunc<Float>([&](vec2 p, vec2 a, vec2 b){
		// Project the point on the segment.
		vec2 dir = b - a;
		Float len2 = dot(dir, dir);
		Float t = clamp(dot(p - a, dir) / len2,0.0,1.0);
		vec2 proj = a + t * dir;
		// Distance between the point and its projection.
		GL_RETURN(distance(p, proj));
	});
	
	auto triangleDistance = makeFunc<Float>([&](vec2 p, vec4 tri, Float width){
		// Point at the bottom center, shared by all triangles.
		vec2 point0 = vec2(0.5, 0.37);
		// Distance to each segment.
		Float minDist = 	   segmentDistance(p, point0, 	 tri[x, y]) ;
		minDist = min(minDist, segmentDistance(p, tri[x, y], tri[z, w]));
		minDist = min(minDist, segmentDistance(p, tri[z, w], point0));
		// Smooth result for transition.
		GL_RETURN(1.0 - smoothstep(0.0, width, minDist));
	});
	
	/// Text utilities.
	auto getLetter = makeFunc<Float>([&](Int lid, vec2 uv){
		// If outside, return arbitrarily high distance.
		GL_IF(uv[x] < 0.0 || uv[y] < 0.0 || uv[x] > 1.0 || uv[y] > 1.0){
			GL_RETURN(1000.0);
		}
		// The font texture is 16x16 glyphs.
		Int vlid = lid/16;
		Int hlid = lid - 16*vlid;
		vec2 fontUV = (vec2(hlid, vlid) + uv)/16.0;
		// Fetch in a 3x3 neighborhood to box blur
		Float accum = 0.0;
		GL_FOR(Int i = -1; i < 2; ++i){
			GL_FOR(Int j = -1; j < 2; ++j){
				vec2 offset = vec2(i,j)/1024.0;
				accum += texture(iChannel0, fontUV + offset, 0.0)[a];
			}
		}
		GL_RETURN(accum/9.0);
	});
	
	auto textGradient = makeFunc<vec3>([&](Float interior, Float top, vec2 alphas){
		// Use squared blend for the interior gradients.
		vec2 alphas2 = alphas * alphas;
		// Generate the four possible gradients (interior/edge x upper/lower)
		vec3 bottomInterior = mix(vec3(0.987,0.746,0.993), vec3(0.033,0.011,0.057), alphas2[x]);
		vec3 bottomExterior = mix(vec3(0.633,0.145,0.693), vec3(0.977,1.000,1.000),  alphas[x]);
		vec3 topInterior 	= mix(vec3(0.024,0.811,0.924), vec3(0.600,0.960,1.080), alphas2[y]);
		vec3 topExterior 	= mix(vec3(0.494,0.828,0.977), vec3(0.968,0.987,0.999),  alphas[y]);
		// Blend based on current location.
		vec3 gradInterior 	= mix(bottomInterior, topInterior, top);
		vec3 gradExterior 	= mix(bottomExterior, topExterior, top);
		GL_RETURN(mix(gradExterior, gradInterior, interior));
	});
	
	/// Main render.
	shader.main([&]{
		// Normalized pixel coordinates.
		vec2 uv = gl_FragCoord[x,y]/iResolution[x,y];
		vec2 uvCenter = 2.0 * uv - 1.0;
		
		/// Background.
		// Color gradient.
		vec3 finalColor = 1.5 * mix(vec3(0.308,0.066,0.327), vec3(0.131,0.204,0.458), uv[x]);
		
		const Float gridHeight = 0.3;
		GL_IF(uv[y] < gridHeight){
			
			/// Bottom grid.
			// Compute local cflipped oordinates for the grid.
			vec2 localUV = uv * vec2(2.0, -1.0/gridHeight) + vec2(-1.0, 1.0);
			// Perspective division, scaling, foreshortening and alignment.
			localUV[x] = localUV[x] / (localUV[y] + 0.8);
			localUV *= vec2(10.0, 20.0);
			localUV[y] = sqrt(localUV[y]);
			localUV[x] += 0.5;
			// Generate grid smooth lines (translate along time).
			vec2 unitUV = fract(localUV - vec2(0.0, 0.3 * iTime));
			vec2 gridAxes = smoothstep(0.02, 0.07, unitUV) * (1.0 - smoothstep(0.93, 0.98, unitUV));
			Float gridAlpha = 1.0 - clamp(gridAxes[x] * gridAxes[y], 0.0, 1.0);
			
			/// Fixed star halos.
			// Loop UVs.
			vec2 cyclicUV = mod(localUV - vec2(0.0, 0.3 * iTime), vec2(9.0, 5.0));
			// Distance to some fixed grid vertices.
			const Float haloTh = 0.6;
			Float isBright1 = 1.0 - min(distance(cyclicUV, vec2(6.0,3.0)), haloTh) / haloTh;
			Float isBright2 = 1.0 - min(distance(cyclicUV, vec2(1.0,2.0)), haloTh) / haloTh;
			Float isBright3 = 1.0 - min(distance(cyclicUV, vec2(3.0,4.0)), haloTh) / haloTh;
			Float isBright4 = 1.0 - min(distance(cyclicUV, vec2(2.0,1.0)), haloTh) / haloTh;
			// Halos brightness.
			Float spotLight = isBright1+isBright2+isBright3+isBright4;
			spotLight *= spotLight;
			// Composite grid lines and halos.
			finalColor += 0.15 * gridAlpha * (1.0 + 5.0 * spotLight);
			
		} GL_ELSE {
			/// Starfield.
			// Compensate aspect ratio for circular stars.
			vec2 ratioUVs = uv * vec2(1.0, iResolution[y] / iResolution[x]);
			// Decrease density towards the bottom of the screen.
			Float baseDens = clamp(uv[y]-0.3, 0.0, 1.0);
			// Three layers of stars with varying density, cyclic animation.
			Float deltaDens = 20.0 * (sin(0.05 * iTime - 1.5) + 1.0);
			finalColor += 0.50 * stars(ratioUVs, 0.10 * baseDens, 150.0 - deltaDens);
			finalColor += 0.75 * stars(ratioUVs, 0.05 * baseDens,  80.0 - deltaDens);
			finalColor += 1.00 * stars(ratioUVs, 0.01 * baseDens,  30.0 - deltaDens);
		}
		
		/// Triangles.
		// Triangles upper points.
		vec4 points1 = vec4(0.30,0.85,0.70,0.85);
		vec4 points2 = vec4(0.33,0.83,0.73,0.88);
		vec4 points3 = vec4(0.35,0.80,0.66,0.82);
		vec4 points4 = vec4(0.38,0.91,0.66,0.87);
		vec4 points5 = vec4(0.31,0.89,0.72,0.83);
		// Randomly perturb based on time.
		points2 += 0.04 * noise4D(10.0 * points2 + 0.4 * iTime);
		points3 += 0.04 * noise4D(10.0 * points3 + 0.4 * iTime);
		points4 += 0.04 * noise4D(10.0 * points4 + 0.4 * iTime);
		points5 += 0.04 * noise4D(10.0 * points5 + 0.4 * iTime);
		// Intensity of the triangle edges.
		Float tri1 = triangleDistance(uv, points1, 0.010);
		Float tri2 = triangleDistance(uv, points2, 0.005);
		Float tri3 = triangleDistance(uv, points3, 0.030);
		Float tri4 = triangleDistance(uv, points4, 0.005);
		Float tri5 = triangleDistance(uv, points5, 0.003);
		Float intensityTri = 0.9 * tri1 + 0.5 * tri2 + 0.2 * tri3 + 0.6 * tri4 + 0.5 * tri5;
		// Triangles color gradient, from left to right.
		Float alphaTriangles = clamp((uv[x] - 0.3) / 0.4, 0.0, 1.0);
		vec3 baseTriColor = mix(vec3(0.957,0.440,0.883), vec3(0.473,0.548,0.919), alphaTriangles);
		// Additive blending.
		finalColor += intensityTri*baseTriColor;
		
		/// Horizon gradient.
		const Float horizonHeight = 0.025;
		Float horizonIntensity = 1.0 - min(abs(uv[y] - gridHeight), horizonHeight) / horizonHeight;
		// Modulate base on distance to screen edges.
		horizonIntensity *= (1.0 - 0.7 * abs(uvCenter[x]) + 0.5);
		finalColor += 2.0 * horizonIntensity * baseTriColor;
		
		/// Letters.
		// Centered UVs for text box.
		vec2 textUV = uvCenter * 2.2 - vec2(0.0, 0.5);
		GL_IF(abs(textUV[x]) < 1.0 && abs(textUV[y]) < 1.0){
			// Rescale UVs.
			textUV = textUV * 0.5 + 0.5;
			textUV[x] *= 3.5;
			// Per-sign UV, manual shifts for kerning.
			const vec2 letterScaling = vec2(0.47,0.93);
			vec2 uvLetter1 = (textUV - vec2(0.60,0.50)) * letterScaling + 0.5;
			vec2 uvLetter2 = (textUV - vec2(1.68,0.50)) * letterScaling + 0.5;
			vec2 uvLetter3 = (textUV - vec2(2.70,0.50)) * letterScaling + 0.5;
			// Get letters distance to edge, merge.
			Float let1 = getLetter(179, uvLetter1);
			Float let2 = getLetter(163, uvLetter2);
			Float let3 = getLetter(188, uvLetter3);
			// Merge and threshold.
			Float finalDist = 0.52 - min(let1, min(let2, let3));
			// Split between top and bottom gradients (landscape in the reflection).
			Float localTh = 0.49 + 0.03 * noise1D(70.0 * uv[x] + iTime);
			Float isTop = smoothstep(localTh - 0.01, localTh + 0.01, textUV[y]);
			// Split between interior and edge gradients.
			Float isInt = smoothstep(0.018, 0.022, finalDist);
			// Compute relative heights along the color gradients (both bottom and up (shifted)), rescale.
			vec2 localUBYs = vec2(1.8 * (textUV[y] - 0.5) + 0.5);
			localUBYs[y] -= isTop * 0.5;
			vec2 gradientBlend = localUBYs / localTh;
			// Evaluate final mixed color gradient.
			vec3 textColor = textGradient(isInt, isTop, gradientBlend);
			// Add sharp reflection along a flat diagonal.
			GL_IF(textUV[x] - 20.0 * textUV[y] < -14.0 || textUV[x] - 20.0 * textUV[y] > -2.5){
				textColor += 0.1;
			}
			// Soft letter edges.
			Float finalDistSmooth = smoothstep(-0.0025, 0.0025,finalDist);
			finalColor = mix(finalColor, textColor, finalDistSmooth);
		}
		
		/// Vignetting.
		const Float radiusMin = 0.8;
		const Float radiusMax = 1.8;
		Float vignetteIntensity = (length(uvCenter) - radiusMin) / (radiusMax - radiusMin);
		finalColor *= clamp(1.0 - vignetteIntensity, 0.0, 1.0);
		
		/// Exposure tweak, output.
		fragColor = vec4(pow(finalColor, vec3(1.2)),1.0);
	});
	
	
	return shader.str();
}
