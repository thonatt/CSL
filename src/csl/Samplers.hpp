#pragma once

#include "NamedObjects.hpp"

namespace csl {

	namespace core {

		template<AccessType aType, ScalarType nType, uint N, SamplerType sType, uint sFlags> // SamplerIsArray isArray, SamplerIsShadow isShadow>
		class Sampler : public NamedObject<Sampler<aType, nType, N, sType, sFlags>> {
		public:
			//static_assert(N <= 3, "Dim must be <= 3");
			//static_assert(IsSamplerType<nType>::value, "Invalide sampler nType");
			//static_assert(IsValid<aType, nType, Dim, sType, isArray, isShadow>::value, "Invalide sampler template arguments");

			Sampler(const std::string & s = "", uint obj_flags = 0)
				: NamedObject<Sampler>(s, obj_flags) {
			}

			Sampler(const Ex & _ex, uint ctor_flags = 0, uint obj_flags = IS_TRACKED, const std::string & s = "")
				: NamedObject<Sampler>(_ex, ctor_flags, obj_flags, s)
			{
			}

		};

	}

	using sampler1D = core::Sampler< core::SAMPLER, core::FLOAT, 1 >;
	using sampler2D = core::Sampler < core::SAMPLER, core::FLOAT, 2 >;
	using sampler3D = core::Sampler < core::SAMPLER, core::FLOAT, 3 >;
	using samplerCube = core::Sampler < core::SAMPLER, core::FLOAT, 0, core::CUBE >;
	using sampler2DRect = core::Sampler < core::SAMPLER, core::FLOAT, 2, core::RECTANGLE >;
	using sampler1DArray = core::Sampler < core::SAMPLER, core::FLOAT, 1, core::BASIC, core::IS_ARRAY >;
	using sampler2DArray = core::Sampler < core::SAMPLER, core::FLOAT, 2, core::BASIC, core::IS_ARRAY >;
	using samplerBuffer = core::Sampler < core::SAMPLER, core::FLOAT, 0, core::BUFFER >;
	using sampler2DMS = core::Sampler < core::SAMPLER, core::FLOAT, 2, core::MULTI_SAMPLE >;
	using sampler2DMSArray = core::Sampler < core::SAMPLER, core::FLOAT, 2, core::MULTI_SAMPLE, core::IS_ARRAY>;
	using samplerCubeArray = core::Sampler < core::SAMPLER, core::FLOAT, 0, core::CUBE, core::IS_ARRAY >;
	using sampler1DShadow = core::Sampler < core::SAMPLER, core::FLOAT, 1, core::BASIC, core::IS_SHADOW >;
	using sampler2DShadow = core::Sampler < core::SAMPLER, core::FLOAT, 2, core::BASIC, core::IS_SHADOW >;
	using sampler2DRectShadow = core::Sampler < core::SAMPLER, core::FLOAT, 2, core::RECTANGLE, core::IS_SHADOW>;
	using sampler1DArrayShadow = core::Sampler < core::SAMPLER, core::FLOAT, 1, core::BASIC, core::IS_ARRAY | core::IS_SHADOW >;
	using sampler2DArrayShadow = core::Sampler < core::SAMPLER, core::FLOAT, 2, core::BASIC, core::IS_ARRAY | core::IS_SHADOW >;
	using samplerCubeShadow = core::Sampler < core::SAMPLER, core::FLOAT, 0, core::CUBE, core::IS_SHADOW >;
	using samplerCubeArrayShadow = core::Sampler < core::SAMPLER, core::FLOAT, 0, core::CUBE, core::IS_ARRAY | core::IS_SHADOW >;

	using isampler1D = core::Sampler < core::SAMPLER, core::INT, 1 >;
	using isampler2D = core::Sampler < core::SAMPLER, core::INT, 2 >;
	using isampler3D = core::Sampler < core::SAMPLER, core::INT, 3 >;
	using isamplerCube = core::Sampler < core::SAMPLER, core::INT, 0, core::CUBE >;
	using isampler2DRect = core::Sampler < core::SAMPLER, core::INT, 2, core::RECTANGLE >;
	using isampler1DArray = core::Sampler < core::SAMPLER, core::INT, 1, core::BASIC, core::IS_ARRAY >;
	using isampler2DArray = core::Sampler < core::SAMPLER, core::INT, 2, core::BASIC, core::IS_ARRAY >;
	using isamplerBuffer = core::Sampler < core::SAMPLER, core::INT, 0, core::BUFFER >;
	using isampler2DMS = core::Sampler < core::SAMPLER, core::INT, 2, core::MULTI_SAMPLE >;
	using isampler2DMSArray = core::Sampler < core::SAMPLER, core::INT, 2, core::MULTI_SAMPLE, core::IS_ARRAY >;
	using isamplerCubeArray = core::Sampler < core::SAMPLER, core::INT, 0, core::CUBE, core::IS_ARRAY >;

	using usampler1D = core::Sampler < core::SAMPLER, core::UINT, 1 >;
	using usampler2D = core::Sampler < core::SAMPLER, core::UINT, 2 >;
	using usampler3D = core::Sampler < core::SAMPLER, core::UINT, 3 >;
	using usamplerCube = core::Sampler < core::SAMPLER, core::UINT, 0, core::CUBE >;
	using usampler2DRect = core::Sampler < core::SAMPLER, core::UINT, 2, core::RECTANGLE >;
	using usampler1DArray = core::Sampler < core::SAMPLER, core::UINT, 1, core::BASIC, core::IS_ARRAY >;
	using usampler2DArray = core::Sampler < core::SAMPLER, core::UINT, 2, core::BASIC, core::IS_ARRAY >;
	using usamplerBuffer = core::Sampler < core::SAMPLER, core::UINT, 0, core::BUFFER >;
	using usampler2DMS = core::Sampler < core::SAMPLER, core::UINT, 2, core::MULTI_SAMPLE >;
	using usampler2DMSArray = core::Sampler < core::SAMPLER, core::UINT, 2, core::MULTI_SAMPLE, core::IS_ARRAY >;
	using usamplerCubeArray = core::Sampler < core::SAMPLER, core::UINT, 0, core::CUBE, core::IS_ARRAY >;

	using image1D = core::Sampler < core::IMAGE, core::FLOAT, 1 >;
	using image2D = core::Sampler < core::IMAGE, core::FLOAT, 2 >;
	using image3D = core::Sampler < core::IMAGE, core::FLOAT, 3 >;
	using imageCube = core::Sampler < core::IMAGE, core::FLOAT, 0, core::CUBE >;
	using image2DRect = core::Sampler < core::IMAGE, core::FLOAT, 2, core::RECTANGLE >;
	using image1DArray = core::Sampler < core::IMAGE, core::FLOAT, 1, core::BASIC, core::IS_ARRAY >;
	using image2DArray = core::Sampler < core::IMAGE, core::FLOAT, 2, core::BASIC, core::IS_ARRAY >;
	using imageBuffer = core::Sampler < core::IMAGE, core::FLOAT, 0, core::BUFFER >;
	using image2DMS = core::Sampler < core::IMAGE, core::FLOAT, 2, core::MULTI_SAMPLE >;
	using image2DMSArray = core::Sampler < core::IMAGE, core::FLOAT, 2, core::MULTI_SAMPLE, core::IS_ARRAY >;
	using imageCubeArray = core::Sampler < core::IMAGE, core::FLOAT, 0, core::CUBE, core::IS_ARRAY >;

	using iimage1D = core::Sampler < core::IMAGE, core::INT, 1 >;
	using iimage2D = core::Sampler < core::IMAGE, core::INT, 2 >;
	using iimage3D = core::Sampler < core::IMAGE, core::INT, 3 >;
	using iimageCube = core::Sampler < core::IMAGE, core::INT, 0, core::CUBE >;
	using iimage2DRect = core::Sampler < core::IMAGE, core::INT, 2, core::RECTANGLE >;
	using iimage1DArray = core::Sampler < core::IMAGE, core::INT, 1, core::BASIC, core::IS_ARRAY >;
	using iimage2DArray = core::Sampler < core::IMAGE, core::INT, 2, core::BASIC, core::IS_ARRAY >;
	using iimageBuffer = core::Sampler < core::IMAGE, core::INT, 0, core::BUFFER >;
	using iimage2DMS = core::Sampler < core::IMAGE, core::INT, 2, core::MULTI_SAMPLE >;
	using iimage2DMSArray = core::Sampler < core::IMAGE, core::INT, 2, core::MULTI_SAMPLE, core::IS_ARRAY>;
	using iimageCubeArray = core::Sampler < core::IMAGE, core::INT, 0, core::CUBE, core::IS_ARRAY >;

	using uimage1D = core::Sampler < core::IMAGE, core::UINT, 1 >;
	using uimage2D = core::Sampler < core::IMAGE, core::UINT, 2 >;
	using uimage3D = core::Sampler < core::IMAGE, core::UINT, 3 >;
	using uimageCube = core::Sampler < core::IMAGE, core::UINT, 0, core::CUBE >;
	using uimage2DRect = core::Sampler < core::IMAGE, core::UINT, 2, core::RECTANGLE >;
	using uimage1DArray = core::Sampler < core::IMAGE, core::UINT, 1, core::BASIC, core::IS_ARRAY >;
	using uimage2DArray = core::Sampler < core::IMAGE, core::UINT, 2, core::BASIC, core::IS_ARRAY >;
	using uimageBuffer = core::Sampler < core::IMAGE, core::UINT, 0, core::BUFFER >;
	using uimage2DMS = core::Sampler < core::IMAGE, core::UINT, 2, core::MULTI_SAMPLE >;
	using uimage2DMSArray = core::Sampler < core::IMAGE, core::UINT, 2, core::MULTI_SAMPLE, core::IS_ARRAY>;
	using uimageCubeArray = core::Sampler < core::IMAGE, core::UINT, 0, core::CUBE, core::IS_ARRAY >;

} //namespace csl

//
//template<ScalarType type> struct IsSamplerType {
//	static const bool value = false;
//};
//
//template<> struct IsSamplerType<FLOAT> {
//	static const bool value = true;
//	static const std::string str() { return ""; }
//};
//template<> struct IsSamplerType<INT> {
//	static const bool value = true;
//	static const std::string str() { return "i"; }
//};
//template<> struct IsSamplerType<UINT> {
//	static const bool value = true;
//	static const std::string str() { return "u"; }
//};

//template<accessType aType, ScalarType nType, unsigned int Dim, samplerType sType = BASIC, samplerIsArray isArray = NOT_ARRAY, samplerIsShadow isShadow = NOT_SHADOW >
//struct IsValid {
//	static const bool value = false;
//};
//
//template<accessType aType, ScalarType nType, unsigned int Dim > struct IsValid<aType, nType, Dim> {
//	static const bool value = true;
//};
//
//template<accessType aType, ScalarType nType, samplerIsArray isArray> struct IsValid<aType, nType, 0, CUBE, isArray> {
//	static const bool value = true;
//};
//
//template<accessType aType, ScalarType nType> struct IsValid<aType, nType, 2, RECTANGLE> {
//	static const bool value = true;
//};
//
//template<accessType aType, ScalarType nType> struct IsValid<aType, nType, 0, BUFFER> {
//	static const bool value = true;
//};
//
//template<accessType aType, ScalarType nType, unsigned int Dim> struct IsValid<aType, nType, Dim, BASIC, ARRAY> {
//	static_assert(Dim <= 2, "Sampler Array, Dim <= 2");
//	static const bool value = true;
//};
//
//template<accessType aType, ScalarType nType, samplerIsArray isArray> struct IsValid<aType, nType, 2, MULTI_SAMPLE, isArray> {
//	static const bool value = true;
//};
//
//template<accessType aType> struct IsValid<aType, FLOAT, 2, RECTANGLE, NOT_ARRAY, NOT_SHADOW> {
//	static const bool value = true;
//};
//
//template<unsigned int Dim, samplerIsArray isArray> struct IsValid<SAMPLER, FLOAT, Dim, BASIC, isArray, SHADOW> {
//	static_assert(Dim <= 2, "Sampler Shadow, Dim <= 2");
//	static const bool value = true;
//};
//
//template<ScalarType nType, samplerIsArray isArray> struct IsValid<SAMPLER, nType, 0, CUBE, isArray, SHADOW> {
//	static const bool value = true;
//};
//
//template<> struct IsValid<SAMPLER, UINT, 0, ATOMIC> {
//	static const bool value = true;
//};

