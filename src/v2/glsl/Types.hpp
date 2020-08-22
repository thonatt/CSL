#pragma once

#include "../v2/Types.hpp"

namespace v2
{
	namespace glsl {

		using Double = Scalar<double>;
		using Float = Scalar<float>;
		using Uint = Scalar<unsigned char>;
		using Int = Scalar<int>;
		using Bool = Scalar<bool>;

		using vec2 = Vector<float, 2>;
		using vec3 = Vector<float, 3>;
		using vec4 = Vector<float, 4>;

		using dvec2 = Vector<double, 2>;
		using dvec3 = Vector<double, 2>;
		using dvec4 = Vector<double, 2>;

		using ivec2 = Vector<int, 2>;
		using ivec3 = Vector<int, 3>;
		using ivec4 = Vector<int, 4>;

		using uvec2 = Vector<unsigned char, 2>;
		using uvec3 = Vector<unsigned char, 3>;
		using uvec4 = Vector<unsigned char, 4>;

		using bvec2 = Vector<bool, 2>;
		using bvec3 = Vector<bool, 3>;
		using bvec4 = Vector<bool, 4>;

		using mat2x2 = Matrix<float, 2, 2>;
		using mat2x3 = Matrix<float, 3, 2>;
		using mat2x4 = Matrix<float, 4, 2>;
		using mat3x2 = Matrix<float, 2, 3>;
		using mat3x3 = Matrix<float, 3, 3>;
		using mat3x4 = Matrix<float, 4, 3>;
		using mat4x2 = Matrix<float, 2, 4>;
		using mat4x3 = Matrix<float, 3, 3>;
		using mat4x4 = Matrix<float, 4, 4>;

		using mat2 = mat2x2;
		using mat3 = mat3x3;
		using mat4 = mat4x4;

		using sampler1D = Sampler< SamplerAccessType::Sampler, float, 1 >;
		using sampler2D = Sampler < SamplerAccessType::Sampler, float, 2 >;
		using sampler3D = Sampler < SamplerAccessType::Sampler, float, 3 >;
		using samplerCube = Sampler < SamplerAccessType::Sampler, float, 0, SamplerType::Cube >;
		using sampler2DRect = Sampler < SamplerAccessType::Sampler, float, 2, SamplerType::Rectangle >;
		using sampler1DArray = Sampler < SamplerAccessType::Sampler, float, 1, SamplerType::Basic, SamplerFlags::Array >;
		using sampler2DArray = Sampler < SamplerAccessType::Sampler, float, 2, SamplerType::Basic, SamplerFlags::Array >;
		using samplerBuffer = Sampler < SamplerAccessType::Sampler, float, 0, SamplerType::Buffer >;
		using sampler2DMS = Sampler < SamplerAccessType::Sampler, float, 2, SamplerType::MultiSample >;
		using sampler2DMSArray = Sampler < SamplerAccessType::Sampler, float, 2, SamplerType::MultiSample, SamplerFlags::Array>;
		using samplerCubeArray = Sampler < SamplerAccessType::Sampler, float, 0, SamplerType::Cube, SamplerFlags::Array >;
		using sampler1DShadow = Sampler < SamplerAccessType::Sampler, float, 1, SamplerType::Basic, SamplerFlags::Shadow >;
		using sampler2DShadow = Sampler < SamplerAccessType::Sampler, float, 2, SamplerType::Basic, SamplerFlags::Shadow >;
		using sampler2DRectShadow = Sampler < SamplerAccessType::Sampler, float, 2, SamplerType::Rectangle, SamplerFlags::Shadow>;
		using sampler1DArrayShadow = Sampler < SamplerAccessType::Sampler, float, 1, SamplerType::Basic, SamplerFlags::Array | SamplerFlags::Shadow >;
		using sampler2DArrayShadow = Sampler < SamplerAccessType::Sampler, float, 2, SamplerType::Basic, SamplerFlags::Array | SamplerFlags::Shadow >;
		using samplerCubeShadow = Sampler < SamplerAccessType::Sampler, float, 0, SamplerType::Cube, SamplerFlags::Shadow >;
		using samplerCubeArrayShadow = Sampler < SamplerAccessType::Sampler, float, 0, SamplerType::Cube, SamplerFlags::Array | SamplerFlags::Shadow >;

		using isampler1D = Sampler < SamplerAccessType::Sampler, int, 1 >;
		using isampler2D = Sampler < SamplerAccessType::Sampler, int, 2 >;
		using isampler3D = Sampler < SamplerAccessType::Sampler, int, 3 >;
		using isamplerCube = Sampler < SamplerAccessType::Sampler, int, 0, SamplerType::Cube >;
		using isampler2DRect = Sampler < SamplerAccessType::Sampler, int, 2, SamplerType::Rectangle >;
		using isampler1DArray = Sampler < SamplerAccessType::Sampler, int, 1, SamplerType::Basic, SamplerFlags::Array >;
		using isampler2DArray = Sampler < SamplerAccessType::Sampler, int, 2, SamplerType::Basic, SamplerFlags::Array >;
		using isamplerBuffer = Sampler < SamplerAccessType::Sampler, int, 0, SamplerType::Buffer >;
		using isampler2DMS = Sampler < SamplerAccessType::Sampler, int, 2, SamplerType::MultiSample >;
		using isampler2DMSArray = Sampler < SamplerAccessType::Sampler, int, 2, SamplerType::MultiSample, SamplerFlags::Array >;
		using isamplerCubeArray = Sampler < SamplerAccessType::Sampler, int, 0, SamplerType::Cube, SamplerFlags::Array >;

		using usampler1D = Sampler < SamplerAccessType::Sampler, unsigned char, 1 >;
		using usampler2D = Sampler < SamplerAccessType::Sampler, unsigned char, 2 >;
		using usampler3D = Sampler < SamplerAccessType::Sampler, unsigned char, 3 >;
		using usamplerCube = Sampler < SamplerAccessType::Sampler, unsigned char, 0, SamplerType::Cube >;
		using usampler2DRect = Sampler < SamplerAccessType::Sampler, unsigned char, 2, SamplerType::Rectangle >;
		using usampler1DArray = Sampler < SamplerAccessType::Sampler, unsigned char, 1, SamplerType::Basic, SamplerFlags::Array >;
		using usampler2DArray = Sampler < SamplerAccessType::Sampler, unsigned char, 2, SamplerType::Basic, SamplerFlags::Array >;
		using usamplerBuffer = Sampler < SamplerAccessType::Sampler, unsigned char, 0, SamplerType::Buffer >;
		using usampler2DMS = Sampler < SamplerAccessType::Sampler, unsigned char, 2, SamplerType::MultiSample >;
		using usampler2DMSArray = Sampler < SamplerAccessType::Sampler, unsigned char, 2, SamplerType::MultiSample, SamplerFlags::Array >;
		using usamplerCubeArray = Sampler < SamplerAccessType::Sampler, unsigned char, 0, SamplerType::Cube, SamplerFlags::Array >;

		using image1D = Sampler < SamplerAccessType::Image, float, 1 >;
		using image2D = Sampler < SamplerAccessType::Image, float, 2 >;
		using image3D = Sampler < SamplerAccessType::Image, float, 3 >;
		using imageCube = Sampler < SamplerAccessType::Image, float, 0, SamplerType::Cube >;
		using image2DRect = Sampler < SamplerAccessType::Image, float, 2, SamplerType::Rectangle >;
		using image1DArray = Sampler < SamplerAccessType::Image, float, 1, SamplerType::Basic, SamplerFlags::Array >;
		using image2DArray = Sampler < SamplerAccessType::Image, float, 2, SamplerType::Basic, SamplerFlags::Array >;
		using imageBuffer = Sampler < SamplerAccessType::Image, float, 0, SamplerType::Buffer >;
		using image2DMS = Sampler < SamplerAccessType::Image, float, 2, SamplerType::MultiSample >;
		using image2DMSArray = Sampler < SamplerAccessType::Image, float, 2, SamplerType::MultiSample, SamplerFlags::Array >;
		using imageCubeArray = Sampler < SamplerAccessType::Image, float, 0, SamplerType::Cube, SamplerFlags::Array >;

		using iimage1D = Sampler < SamplerAccessType::Image, int, 1 >;
		using iimage2D = Sampler < SamplerAccessType::Image, int, 2 >;
		using iimage3D = Sampler < SamplerAccessType::Image, int, 3 >;
		using iimageCube = Sampler < SamplerAccessType::Image, int, 0, SamplerType::Cube >;
		using iimage2DRect = Sampler < SamplerAccessType::Image, int, 2, SamplerType::Rectangle >;
		using iimage1DArray = Sampler < SamplerAccessType::Image, int, 1, SamplerType::Basic, SamplerFlags::Array >;
		using iimage2DArray = Sampler < SamplerAccessType::Image, int, 2, SamplerType::Basic, SamplerFlags::Array >;
		using iimageBuffer = Sampler < SamplerAccessType::Image, int, 0, SamplerType::Buffer >;
		using iimage2DMS = Sampler < SamplerAccessType::Image, int, 2, SamplerType::MultiSample >;
		using iimage2DMSArray = Sampler < SamplerAccessType::Image, int, 2, SamplerType::MultiSample, SamplerFlags::Array>;
		using iimageCubeArray = Sampler < SamplerAccessType::Image, int, 0, SamplerType::Cube, SamplerFlags::Array >;

		using uimage1D = Sampler < SamplerAccessType::Image, unsigned char, 1 >;
		using uimage2D = Sampler < SamplerAccessType::Image, unsigned char, 2 >;
		using uimage3D = Sampler < SamplerAccessType::Image, unsigned char, 3 >;
		using uimageCube = Sampler < SamplerAccessType::Image, unsigned char, 0, SamplerType::Cube >;
		using uimage2DRect = Sampler < SamplerAccessType::Image, unsigned char, 2, SamplerType::Rectangle >;
		using uimage1DArray = Sampler < SamplerAccessType::Image, unsigned char, 1, SamplerType::Basic, SamplerFlags::Array >;
		using uimage2DArray = Sampler < SamplerAccessType::Image, unsigned char, 2, SamplerType::Basic, SamplerFlags::Array >;
		using uimageBuffer = Sampler < SamplerAccessType::Image, unsigned char, 0, SamplerType::Buffer >;
		using uimage2DMS = Sampler < SamplerAccessType::Image, unsigned char, 2, SamplerType::MultiSample >;
		using uimage2DMSArray = Sampler < SamplerAccessType::Image, unsigned char, 2, SamplerType::MultiSample, SamplerFlags::Array>;
		using uimageCubeArray = Sampler < SamplerAccessType::Image, unsigned char, 0, SamplerType::Cube, SamplerFlags::Array >;

		using atomic_uint = Sampler<SamplerAccessType::Sampler, unsigned char, 0, SamplerType::Atomic >;
	}

}