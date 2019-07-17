#include "verts.h"

#include "../Shaders.h"


std::string transfeedBackVertex()
{
	using namespace all_swizzles;
	using namespace vert_330;

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

	auto randomInitialVelocity = makeFun<vec3>( "randomInitialVelocity", [&] {
		Float theta = mix(0.0, pi / 8.0, texelFetch(RandomTex, 3 * gl_VertexID, 0)[r]) << "theta";
		Float phi = mix(0.0, 2.0 * pi, texelFetch(RandomTex, 3 * gl_VertexID + 1, 0)[r]) << "phi";
		Float velocity = mix(1.25, 1.5, texelFetch(RandomTex, 3 * gl_VertexID + 2, 0)[r]) << "velocity";
		vec3 v = vec3(sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi)) << "v";
		GL_RETURN(normalize(EmitterBasis * v) * velocity);
	});

	auto update = makeFun<void>("update", [&] {
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

	auto render = makeFun<void>("render", [&] {
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
