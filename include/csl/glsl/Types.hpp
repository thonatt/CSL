#pragma once

#include <csl/Types.hpp>

namespace csl
{
	namespace glsl {

		using Double = Scalar<double>;
		using Float = Scalar<float>;
		using Uint = Scalar<unsigned int>;
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

		using uvec2 = Vector<unsigned int, 2>;
		using uvec3 = Vector<unsigned int, 3>;
		using uvec4 = Vector<unsigned int, 4>;

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

		using sampler1D = Sampler<float, 1>;
		using sampler2D = Sampler<float, 2>;
		using sampler3D = Sampler<float, 3>;
		using samplerCube = Sampler<float, 0, SamplerFlags::Sampler | SamplerFlags::Cube>;
		using sampler2DRect = Sampler<float, 2, SamplerFlags::Sampler | SamplerFlags::Rectangle>;
		using sampler1DArray = Sampler<float, 1, SamplerFlags::Default | SamplerFlags::Array>;
		using sampler2DArray = Sampler<float, 2, SamplerFlags::Default | SamplerFlags::Array>;
		using samplerBuffer = Sampler <float, 0, SamplerFlags::Sampler | SamplerFlags::Buffer>;
		using sampler2DMS = Sampler<float, 2, SamplerFlags::Default | SamplerFlags::Multisample>;
		using sampler2DMSArray = Sampler<float, 2, SamplerFlags::Default | SamplerFlags::Multisample | SamplerFlags::Array>;
		using samplerCubeArray = Sampler<float, 0, SamplerFlags::Sampler | SamplerFlags::Cube | SamplerFlags::Array>;
		using sampler1DShadow = Sampler<float, 1, SamplerFlags::Default | SamplerFlags::Shadow>;
		using sampler2DShadow = Sampler <float, 2, SamplerFlags::Default | SamplerFlags::Shadow>;
		using sampler2DRectShadow = Sampler<float, 2, SamplerFlags::Default | SamplerFlags::Rectangle | SamplerFlags::Shadow>;
		using sampler1DArrayShadow = Sampler<float, 1, SamplerFlags::Default | SamplerFlags::Array | SamplerFlags::Shadow>;
		using sampler2DArrayShadow = Sampler<float, 2, SamplerFlags::Default | SamplerFlags::Array | SamplerFlags::Shadow>;
		using samplerCubeShadow = Sampler<float, 0, SamplerFlags::Sampler | SamplerFlags::Cube | SamplerFlags::Shadow>;
		using samplerCubeArrayShadow = Sampler<float, 0, SamplerFlags::Sampler | SamplerFlags::Cube | SamplerFlags::Array | SamplerFlags::Shadow>;

		using isampler1D = Sampler<int, 1>;
		using isampler2D = Sampler<int, 2>;
		using isampler3D = Sampler<int, 3>;
		using isamplerCube = Sampler<int, 0, SamplerFlags::Sampler | SamplerFlags::Cube>;
		using isampler2DRect = Sampler<int, 2, SamplerFlags::Sampler | SamplerFlags::Rectangle>;
		using isampler1DArray = Sampler<int, 1, SamplerFlags::Default | SamplerFlags::Array>;
		using isampler2DArray = Sampler<int, 2, SamplerFlags::Default | SamplerFlags::Array>;
		using isamplerBuffer = Sampler <int, 0, SamplerFlags::Sampler | SamplerFlags::Buffer>;
		using isampler2DMS = Sampler<int, 2, SamplerFlags::Default | SamplerFlags::Multisample>;
		using isampler2DMSArray = Sampler<int, 2, SamplerFlags::Sampler | SamplerFlags::Multisample | SamplerFlags::Array>;
		using isamplerCubeArray = Sampler<int, 0, SamplerFlags::Sampler | SamplerFlags::Cube | SamplerFlags::Array>;

		using usampler1D = Sampler<unsigned int, 1>;
		using usampler2D = Sampler<unsigned int, 2>;
		using usampler3D = Sampler<unsigned int, 3>;
		using usamplerCube = Sampler<unsigned int, 0, SamplerFlags::Sampler | SamplerFlags::Cube>;
		using usampler2DRect = Sampler<unsigned int, 2, SamplerFlags::Sampler | SamplerFlags::Rectangle>;
		using usampler1DArray = Sampler<unsigned int, 1, SamplerFlags::Default | SamplerFlags::Array>;
		using usampler2DArray = Sampler<unsigned int, 2, SamplerFlags::Default | SamplerFlags::Array>;
		using usamplerBuffer = Sampler <unsigned int, 0, SamplerFlags::Sampler | SamplerFlags::Buffer>;
		using usampler2DMS = Sampler<unsigned int, 2, SamplerFlags::Sampler | SamplerFlags::Multisample>;
		using usampler2DMSArray = Sampler<unsigned int, 2, SamplerFlags::Sampler | SamplerFlags::Multisample | SamplerFlags::Array>;
		using usamplerCubeArray = Sampler<unsigned int, 0, SamplerFlags::Sampler | SamplerFlags::Cube | SamplerFlags::Array>;

		using image1D = Sampler<float, 1, SamplerFlags::Image | SamplerFlags::Basic>;
		using image2D = Sampler<float, 2, SamplerFlags::Image | SamplerFlags::Basic>;
		using image3D = Sampler<float, 3, SamplerFlags::Image | SamplerFlags::Basic>;
		using imageCube = Sampler<float, 0, SamplerFlags::Image | SamplerFlags::Cube>;
		using image2DRect = Sampler<float, 2, SamplerFlags::Image | SamplerFlags::Rectangle>;
		using image1DArray = Sampler<float, 1, SamplerFlags::Image | SamplerFlags::Basic | SamplerFlags::Array>;
		using image2DArray = Sampler<float, 2, SamplerFlags::Image | SamplerFlags::Basic | SamplerFlags::Array>;
		using imageBuffer = Sampler <float, 0, SamplerFlags::Image | SamplerFlags::Buffer>;
		using image2DMS = Sampler<float, 2, SamplerFlags::Image | SamplerFlags::Basic | SamplerFlags::Multisample>;
		using image2DMSArray = Sampler<float, 2, SamplerFlags::Image | SamplerFlags::Multisample | SamplerFlags::Array>;
		using imageCubeArray = Sampler<float, 0, SamplerFlags::Image | SamplerFlags::Cube | SamplerFlags::Array>;

		using iimage1D = Sampler<int, 1, SamplerFlags::Image | SamplerFlags::Basic>;
		using iimage2D = Sampler<int, 2, SamplerFlags::Image | SamplerFlags::Basic>;
		using iimage3D = Sampler<int, 3, SamplerFlags::Image | SamplerFlags::Basic>;
		using iimageCube = Sampler<int, 0, SamplerFlags::Image | SamplerFlags::Cube>;
		using iimage2DRect = Sampler<int, 2, SamplerFlags::Image | SamplerFlags::Rectangle>;
		using iimage1DArray = Sampler<int, 1, SamplerFlags::Image | SamplerFlags::Basic | SamplerFlags::Array>;
		using iimage2DArray = Sampler<int, 2, SamplerFlags::Image | SamplerFlags::Basic | SamplerFlags::Array>;
		using iimageBuffer = Sampler <int, 0, SamplerFlags::Image | SamplerFlags::Buffer>;
		using iimage2DMS = Sampler<int, 2, SamplerFlags::Image | SamplerFlags::Multisample>;
		using iimage2DMSArray = Sampler<int, 2, SamplerFlags::Image | SamplerFlags::Multisample | SamplerFlags::Array>;
		using iimageCubeArray = Sampler<int, 0, SamplerFlags::Image | SamplerFlags::Cube | SamplerFlags::Array>;

		using uimage1D = Sampler<unsigned int, 1, SamplerFlags::Image | SamplerFlags::Basic>;
		using uimage2D = Sampler<unsigned int, 2, SamplerFlags::Image | SamplerFlags::Basic>;
		using uimage3D = Sampler<unsigned int, 3, SamplerFlags::Image | SamplerFlags::Basic>;
		using uimageCube = Sampler<unsigned int, 0, SamplerFlags::Image | SamplerFlags::Cube>;
		using uimage2DRect = Sampler<unsigned int, 2, SamplerFlags::Image | SamplerFlags::Rectangle>;
		using uimage1DArray = Sampler<unsigned int, 1, SamplerFlags::Image | SamplerFlags::Array>;
		using uimage2DArray = Sampler<unsigned int, 2, SamplerFlags::Image | SamplerFlags::Array>;
		using uimageBuffer = Sampler <unsigned int, 0, SamplerFlags::Image | SamplerFlags::Buffer>;
		using uimage2DMS = Sampler<unsigned int, 2, SamplerFlags::Image | SamplerFlags::Multisample>;
		using uimage2DMSArray = Sampler<unsigned int, 2, SamplerFlags::Image | SamplerFlags::Multisample | SamplerFlags::Array>;
		using uimageCubeArray = Sampler<unsigned int, 0, SamplerFlags::Image | SamplerFlags::Cube | SamplerFlags::Array>;

		using atomic_uint = Sampler<unsigned int, 0, SamplerFlags::Atomic>;
	}
}