#include "verts.h"

#include <csl/Shaders.h>


std::string transfeedBackVertex()
{
	using namespace csl::vert_330;
	using namespace csl::swizzles::all;

	double pi = 3.14159265359;

	Shader shader;

	In<vec3, Layout<Location<0>>> VertexPosition("VertexPosition");
	In<vec3, Layout<Location<1>>> VertexVelocity("VertexVelocity");
	In<Float, Layout<Location<2>>> VertexAge("VertexAge");
	lineBreak();

	Uniform<Int> Pass("Pass");
	lineBreak();

	Out<vec3> Position("Position");
	Out<vec3> Velocity("Velocity");
	Out<Float> Age("Age");
	Out<Float> Transp("Transp");
	Out<vec2> TexCoord("TexCoord");
	lineBreak();

	Uniform<Float> Time("Time");
	Uniform<Float> DeltaT("DeltaT");
	Uniform<vec3> Accel("Accel");
	Uniform<Float> ParticleLifetime("ParticleLifetime");
	Uniform<vec3> Emitter = vec3(0) << "Emitter";
	Uniform<mat3> EmitterBasis("EmitterBasis");
	Uniform<Float> ParticleSize("ParticleSize");
	lineBreak();

	Uniform<mat4> MV("MV");
	Uniform<mat4> Proj("Proj");
	lineBreak();

	Uniform<sampler1D> RandomTex("RandomTex");
	lineBreak();

	const Array<vec3> offsets = { "offsets", vec3(-0.5,-0.5,0), vec3(0.5,-0.5,0), vec3(0.5,0.5,0),
							  vec3(-0.5,-0.5,0), vec3(0.5,0.5,0), vec3(-0.5,0.5,0) };
	lineBreak();
	
	const Array<vec2> texCoords = { "texCoords", vec2(0,0), vec2(1,0), vec2(1,1), vec2(0,0), vec2(1,1), vec2(0,1) };

	auto randomInitialVelocity = makeFunc<vec3>( "randomInitialVelocity", [&] {
		Float theta = mix(0.0, pi / 8.0, texelFetch(RandomTex, 3 * gl_VertexID, 0)[r]) << "theta";
		Float phi = mix(0.0, 2.0 * pi, texelFetch(RandomTex, 3 * gl_VertexID + 1, 0)[r]) << "phi";
		Float velocity = mix(1.25, 1.5, texelFetch(RandomTex, 3 * gl_VertexID + 2, 0)[r]) << "velocity";
		vec3 v = vec3(sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi)) << "v";
		GL_RETURN(normalize(EmitterBasis * v) * velocity);
	});

	auto update = makeFunc<void>("update", [&] {
		GL_IF (VertexAge < 0 || VertexAge > ParticleLifetime) {
			Position = Emitter;
			Velocity = randomInitialVelocity();
			GL_IF (VertexAge < 0) Age = VertexAge + DeltaT;
			GL_ELSE Age = (VertexAge - ParticleLifetime) + DeltaT;
		} GL_ELSE {
			Position = VertexPosition + VertexVelocity * DeltaT;
			Velocity = VertexVelocity + Accel * DeltaT;
			Age = VertexAge + DeltaT;
		}
	});

	auto render = makeFunc<void>("render", [&] {
		Transp = 0.0;
		vec3 posCam = vec3(0.0) << "posCam";
		GL_IF (VertexAge >= 0.0) {
			posCam = (MV * vec4(VertexPosition, 1))[x,y,z] + offsets[gl_VertexID] * ParticleSize;
			Transp = clamp(1.0 - VertexAge / ParticleLifetime, 0, 1);
		}
		TexCoord = texCoords[gl_VertexID];
		gl_Position = Proj * vec4(posCam, 1);
	});

	shader.main([&] {
		GL_IF (Pass == 1)
			update();
		GL_ELSE
			render();
	});

	return shader.str();
}

std::string dolphinVertex() {

	using namespace csl::vert_330;
	using namespace csl::swizzles::all;

	Shader shader;

	GL_STRUCT(Light,
		(ivec4) color,
		(vec4) cosatt,
		(vec4) distatt,
		(vec4) pos,
		(vec4) dir
	);

	using VSBlockQualifier = Uniform<Layout<Binding<2>, std140>>;
	GL_INTERFACE_BLOCK(VSBlockQualifier, VSBLock, , ,
		(Uint) components,
		(Uint) xfmem_dualTexInfo,
		(Uint) xfmem_numColorChans,
		(GetArray<vec4>::Size<6>) cpnmtx,
		(GetArray<vec4>::Size<4>) cproj,
		(GetArray<ivec4>::Size<6>) cmtrl,
		(GetArray<Light>::Size<8>) clights,
		(GetArray<vec4>::Size<24>) ctexmtx,
		(GetArray<vec4>::Size<64>) ctrmtx,
		(GetArray<vec4>::Size<32>) cnmtx,
		(GetArray<vec4>::Size<64>) cpostmtx,
		(vec4) cpixelcenter,
		(vec2) cviewport,
		(GetArray<uvec4>::Size<8>) xfmem_pack1
	);

	GL_STRUCT(VS_OUTPUT,
		(vec4) pos,
		(vec4) colors_0,
		(vec4) colors_1,
		(vec3) tex0,
		(vec3) tex1,
		(vec4) clipPos,
		(Float) clipDist0,
		(Float) clipDist1
	);

	auto CalculateLighting = makeFunc<ivec4>("CalculateLighting", [](Uint index, Uint attnfunc, Uint diffusefunc, vec3 pos, vec3 normal) {
		vec3 ldir("ldir"), h("h"), cosAttn("cosAttn"), distAttn("distAttn");
		Float dist("dist"), dist2("dist2"), attn("attn");
		GL_RETURN(dist*dist2*attn *(ldir + h + cosAttn + distAttn));
	}, "index", "attnfunc", "diffusefunc", "pos", "normal" );

	return shader.str();
}
