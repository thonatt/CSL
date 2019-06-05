#include "frags.h"

#include "../Shaders.h"

std::string blurShader()
{

	using namespace all_swizzles;
	using namespace frag_450;
	
	Shader shader;

	//GL_STRUCT(Interface, (vec2) uv);
	//In<Interface> in("In");
	GL_INTERFACE(In, Interface, (vec2) uv) in("In");

	Uniform<sampler2D, Layout<Binding<0> >> sceenTexture("screenTexture");
	Uniform<vec2> fetchOffset("fetchOffset");

	GL_DECLARE(fragColor, Out<vec3> );
	//Out<vec3> fragColor("fragColor");

	shader.main([&]() {

		vec3 col = texture(sceenTexture, in.uv)[r, g, b] * Float(5.0) / 16.0 << "col";
		col += texture(sceenTexture, in.uv - fetchOffset)[r, g, b] * Float(5.0) / 16.0;
		col += texture(sceenTexture, in.uv + fetchOffset)[r, g, b] * Float(5.0) / 16.0;
		fragColor = col;
		
	});

	return shader.str();
}

std::string ambiantShader() {

	using namespace all_swizzles;
	using namespace frag_330;

	Shader shader;

	double INV_M_PI = 0.3183098862;
	double M_PI = 3.1415926536;
	double M_INV_LOG2 = 1.4426950408889;

	uint SAMPLES_COUNT = 16u;
	int MAX_LOD = 5;

	GL_INTERFACE(In, Interface, (vec2) uv) in("In");

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

	auto positionFromDepth = makeFun<vec3>("positionFromDepth", [&](Float depth) {
		Float depth2 = 2.0 * depth - 1.0 << "depth2";
		vec2 ndcPos = 2.0 * in.uv - 1.0 << "ndcPos";
		Float viewDepth = -projectionMatrix[w] / (depth2 + projectionMatrix[z]) << "viewDepth";
		GL_RETURN(vec3(-ndcPos * viewDepth / projectionMatrix[x, y], viewDepth));
	}, "depth");

	auto ggx = makeFun<vec3>("ggx", [&](vec3 n, vec3 v, vec3 F0, Float roughness) {
		Float NdotV = max(0.0, dot(v, n)) << "NdotV";
		vec3 ref = -reflect(v, n) << "r";
		ref = normalize((inverseV * vec4(ref, 0.0))[x,y,z]);
		vec2 brdfParams = texture(brdfPrecalc, vec2(NdotV, roughness))[r, g] << "brdfParams";
		vec3 specularColor = textureLod(textureCubeMap, ref, MAX_LOD * roughness)[r, g, b] << "specularColor";
		GL_RETURN(specularColor * (brdfParams[x] * F0 + brdfParams[y]));
	}, "n", "v", "F0", "roughness");


	auto applySH = makeFun<vec3>("applySH", [&](vec3 wn) {
		GL_RETURN((shCoeffs[7] * wn[z] + shCoeffs[4] * wn[y] + shCoeffs[8] * wn[x] + shCoeffs[3]) * wn[x] +
			(shCoeffs[5] * wn[z] - shCoeffs[8] * wn[y] + shCoeffs[1]) * wn[y] +
			(shCoeffs[6] * wn[z] + shCoeffs[2]) * wn[z] +
			shCoeffs[0]);
	}, "wn");

	shader.main([&] {
		vec4 albedoInfo = texture(albedoTexture, in.uv) << "albedoInfo";

		GL_IF (albedoInfo[a] == 0) {
			fragColor = albedoInfo[r,g,b];
			GL_RETURN();
		}
		lineBreak();

		vec3 baseColor = albedoInfo[r, g, b] << "baseColor";
		vec3 infos = texture(effectsTexture, in.uv)[r, g, b] << "infos";
		Float roughness = max(0.045, infos[r]) << "roughness";
		Float metallic = infos[g] << "metallic";
		Float depth = texture(depthTexture, in.uv)[r] << "depth";
		vec3 position = positionFromDepth(depth) << "position";
		vec3 n = normalize(2.0 * texture(normalTexture, in.uv)[r, g, b] - 1.0) << "n";
		vec3 v = normalize(-position) << "v";
		
		lineBreak();

		Float precomputedAO = infos[b] << "precomputedAP";
		Float realtimeAO = texture(ssaoTexture, in.uv)[r] << "realtimeAO";
		Float ao = realtimeAO * precomputedAO << "ao";

		lineBreak();

		vec3 worldNormal = normalize(vec3(inverseV * vec4(n, 0.0))) << "worldNormal";
		vec3 envLighting = applySH(worldNormal) << "envLighting";

		vec3 F0 = mix(vec3(0.08), baseColor, metallic) << "F0";

		vec3 diffuse = (1.0 - metallic) * baseColor * (1.0 - F0) * envLighting << "diffuse";

		vec3 specular = ggx(n, v, F0, roughness) << "speclar";

		lineBreak();

		fragColor = ao * (diffuse + specular);
	});

	return shader.str();
}

std::string ssaoShader()
{
	using namespace all_swizzles;
	using namespace frag_330;

	Shader shader;

	GL_INTERFACE(In, Interface, (vec2) uv) in("In");

	Uniform<sampler2D, Layout<Binding<0>>> depthTexture("depthTexture");
	Uniform<sampler2D, Layout<Binding<1>>> normalTexture("normalTexture");
	Uniform<sampler2D, Layout<Binding<2>>> noiseTexture("noiseTexture");

	Uniform<mat4> projectionMatrix("projectionMatrix");
	Array<Uniform<vec3>, 24> samples("samples");

	Uniform<Float> radius = Float(0.5) << "radius";
	
	Out<Float, Layout<Location<0>>> fragColor("fragColor");

	auto linearizeDepth = makeFun<Float>("linearizeDepth", [&](Float depth) {
		Float depth2 = 2.0*depth - 1.0 << "depth2";
		Float viewDepth = -projectionMatrix[3][2] / (depth2 + projectionMatrix[2][2]) << "viewDepth";
		GL_RETURN(viewDepth);
	}, "depth");

	auto positionFromUV = makeFun<vec3>("positionFromUV", [&](vec2 uv) {
		Float depth = texture(depthTexture, uv)[r] << "depth";
		Float viewDepth = linearizeDepth(depth) << "viewDepth";
		vec2 ndcPos = 2.0 * uv - 1.0 << "ndcPos";
		GL_RETURN(vec3(-ndcPos * viewDepth / vec2(projectionMatrix[0][0], projectionMatrix[1][1]), viewDepth));
	}, "uv");

	shader.main([&]() {
		vec3 n = normalize(2.0 * texture(normalTexture, in.uv)[r, g, b] - 1.0) << "n";

		GL_IF (length(n) < 0.1) {
			fragColor = 1.0;
			GL_RETURN();
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
