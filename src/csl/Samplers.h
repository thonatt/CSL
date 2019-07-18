#pragma once

#include "ExpressionsTest.h"

namespace csl {

	template<AccessType aType, ScalarType nType, uint N, SamplerType sType, uint sFlags> // SamplerIsArray isArray, SamplerIsShadow isShadow>
	class Sampler : public NamedObject<Sampler<aType, nType, N, sType, sFlags>> {
	public:
		//static_assert(N <= 3, "Dim must be <= 3");
		//static_assert(IsSamplerType<nType>::value, "Invalide sampler nType");
		//static_assert(IsValid<aType, nType, Dim, sType, isArray, isShadow>::value, "Invalide sampler template arguments");

		Sampler(const std::string & s = "", uint obj_flags = 0)
			: NamedObject<Sampler>(s, obj_flags) {
		}
	};

	using sampler1D = Sampler < SAMPLER, FLOAT, 1 >;
	using sampler2D = Sampler < SAMPLER, FLOAT, 2 >;
	using sampler3D = Sampler < SAMPLER, FLOAT, 3 >;
	using samplerCube = Sampler < SAMPLER, FLOAT, 0, CUBE >;
	using sampler2DRect = Sampler < SAMPLER, FLOAT, 2, RECTANGLE >;
	using sampler1DArray = Sampler < SAMPLER, FLOAT, 1, BASIC, IS_ARRAY >;
	using sampler2DArray = Sampler < SAMPLER, FLOAT, 2, BASIC, IS_ARRAY >;
	using samplerBuffer = Sampler < SAMPLER, FLOAT, 0, BUFFER >;
	using sampler2DMS = Sampler < SAMPLER, FLOAT, 2, MULTI_SAMPLE >;
	using sampler2DMSArray = Sampler < SAMPLER, FLOAT, 2, MULTI_SAMPLE, IS_ARRAY>;
	using samplerCubeArray = Sampler < SAMPLER, FLOAT, 0, CUBE, IS_ARRAY >;
	using sampler1DShadow = Sampler < SAMPLER, FLOAT, 1, BASIC, IS_SHADOW >;
	using sampler2DShadow = Sampler < SAMPLER, FLOAT, 2, BASIC, IS_SHADOW >;
	using sampler2DRectShadow = Sampler < SAMPLER, FLOAT, 2, RECTANGLE, IS_SHADOW>;
	using sampler1DArrayShadow = Sampler < SAMPLER, FLOAT, 1, BASIC, IS_ARRAY | IS_SHADOW >;
	using sampler2DArrayShadow = Sampler < SAMPLER, FLOAT, 2, BASIC, IS_ARRAY | IS_SHADOW >;
	using samplerCubeShadow = Sampler < SAMPLER, FLOAT, 0, CUBE, IS_SHADOW >;
	using samplerCubeArrayShadow = Sampler < SAMPLER, FLOAT, 0, CUBE, IS_ARRAY | IS_SHADOW >;

	using isampler1D = Sampler < SAMPLER, INT, 1 >;
	using isampler2D = Sampler < SAMPLER, INT, 2 >;
	using isampler3D = Sampler < SAMPLER, INT, 3 >;
	using isamplerCube = Sampler < SAMPLER, INT, 0, CUBE >;
	using isampler2DRect = Sampler < SAMPLER, INT, 2, RECTANGLE >;
	using isampler1DArray = Sampler < SAMPLER, INT, 1, BASIC, IS_ARRAY >;
	using isampler2DArray = Sampler < SAMPLER, INT, 2, BASIC, IS_ARRAY >;
	using isamplerBuffer = Sampler < SAMPLER, INT, 0, BUFFER >;
	using isampler2DMS = Sampler < SAMPLER, INT, 2, MULTI_SAMPLE >;
	using isampler2DMSArray = Sampler < SAMPLER, INT, 2, MULTI_SAMPLE, IS_ARRAY >;
	using isamplerCubeArray = Sampler < SAMPLER, INT, 0, CUBE, IS_ARRAY >;

	using usampler1D = Sampler < SAMPLER, UINT, 1 >;
	using usampler2D = Sampler < SAMPLER, UINT, 2 >;
	using usampler3D = Sampler < SAMPLER, UINT, 3 >;
	using usamplerCube = Sampler < SAMPLER, UINT, 0, CUBE >;
	using usampler2DRect = Sampler < SAMPLER, UINT, 2, RECTANGLE >;
	using usampler1DArray = Sampler < SAMPLER, UINT, 1, BASIC, IS_ARRAY >;
	using usampler2DArray = Sampler < SAMPLER, UINT, 2, BASIC, IS_ARRAY >;
	using usamplerBuffer = Sampler < SAMPLER, UINT, 0, BUFFER >;
	using usampler2DMS = Sampler < SAMPLER, UINT, 2, MULTI_SAMPLE >;
	using usampler2DMSArray = Sampler < SAMPLER, UINT, 2, MULTI_SAMPLE, IS_ARRAY >;
	using usamplerCubeArray = Sampler < SAMPLER, UINT, 0, CUBE, IS_ARRAY >;

	using image1D = Sampler < IMAGE, FLOAT, 1 >;
	using image2D = Sampler < IMAGE, FLOAT, 2 >;
	using image3D = Sampler < IMAGE, FLOAT, 3 >;
	using imageCube = Sampler < IMAGE, FLOAT, 0, CUBE >;
	using image2DRect = Sampler < IMAGE, FLOAT, 2, RECTANGLE >;
	using image1DArray = Sampler < IMAGE, FLOAT, 1, BASIC, IS_ARRAY >;
	using image2DArray = Sampler < IMAGE, FLOAT, 2, BASIC, IS_ARRAY >;
	using imageBuffer = Sampler < IMAGE, FLOAT, 0, BUFFER >;
	using image2DMS = Sampler < IMAGE, FLOAT, 2, MULTI_SAMPLE >;
	using image2DMSArray = Sampler < IMAGE, FLOAT, 2, MULTI_SAMPLE, IS_ARRAY >;
	using imageCubeArray = Sampler < IMAGE, FLOAT, 0, CUBE, IS_ARRAY >;

	using iimage1D = Sampler < IMAGE, INT, 1 >;
	using iimage2D = Sampler < IMAGE, INT, 2 >;
	using iimage3D = Sampler < IMAGE, INT, 3 >;
	using iimageCube = Sampler < IMAGE, INT, 0, CUBE >;
	using iimage2DRect = Sampler < IMAGE, INT, 2, RECTANGLE >;
	using iimage1DArray = Sampler < IMAGE, INT, 1, BASIC, IS_ARRAY >;
	using iimage2DArray = Sampler < IMAGE, INT, 2, BASIC, IS_ARRAY >;
	using iimageBuffer = Sampler < IMAGE, INT, 0, BUFFER >;
	using iimage2DMS = Sampler < IMAGE, INT, 2, MULTI_SAMPLE >;
	using iimage2DMSArray = Sampler < IMAGE, INT, 2, MULTI_SAMPLE, IS_ARRAY>;
	using iimageCubeArray = Sampler < IMAGE, INT, 0, CUBE, IS_ARRAY >;

	using uimage1D = Sampler < IMAGE, UINT, 1 >;
	using uimage2D = Sampler < IMAGE, UINT, 2 >;
	using uimage3D = Sampler < IMAGE, UINT, 3 >;
	using uimageCube = Sampler < IMAGE, UINT, 0, CUBE >;
	using uimage2DRect = Sampler < IMAGE, UINT, 2, RECTANGLE >;
	using uimage1DArray = Sampler < IMAGE, UINT, 1, BASIC, IS_ARRAY >;
	using uimage2DArray = Sampler < IMAGE, UINT, 2, BASIC, IS_ARRAY >;
	using uimageBuffer = Sampler < IMAGE, UINT, 0, BUFFER >;
	using uimage2DMS = Sampler < IMAGE, UINT, 2, MULTI_SAMPLE >;
	using uimage2DMSArray = Sampler < IMAGE, UINT, 2, MULTI_SAMPLE, IS_ARRAY>;
	using uimageCubeArray = Sampler < IMAGE, UINT, 0, CUBE, IS_ARRAY >;

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

