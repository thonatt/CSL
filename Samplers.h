#pragma once


//#include "Algebra.h"

enum class accessType { SAMPLER, IMAGE};

enum class samplerType { BASIC, CUBE, RECTANGLE, MULTI_SAMPLE, BUFFER, ATOMIC };

template<accessType t> struct AccessTypeInfo;
template<> struct AccessTypeInfo<accessType::SAMPLER> {
	static const std::string str() { return "sampler"; }
};
template<> struct AccessTypeInfo<accessType::IMAGE> {
	static const std::string str() { return "image"; }
};

template<samplerType t> struct SamplerTypeInfo;
template<> struct SamplerTypeInfo<samplerType::BASIC> {
	static const std::string str() { return ""; }
};
template<> struct SamplerTypeInfo<samplerType::RECTANGLE> {
	static const std::string str() { return "Rect"; }
};
template<> struct SamplerTypeInfo<samplerType::CUBE> {
	static const std::string str() { return "Cube"; }
};
template<> struct SamplerTypeInfo<samplerType::MULTI_SAMPLE> {
	static const std::string str() { return "MS"; }
};
template<> struct SamplerTypeInfo<samplerType::BUFFER> {
	static const std::string str() { return "Buffer"; }
};


template<numberType type> struct IsSamplerType {
	static const bool value = false;
};

template<> struct IsSamplerType<numberType::FLOAT> {
	static const bool value = true;
	static const std::string str() { return ""; }
};
template<> struct IsSamplerType<numberType::INT> {
	static const bool value = true;
	static const std::string str() { return "i"; }
};
template<> struct IsSamplerType<numberType::UINT> {
	static const bool value = true;
	static const std::string str() { return "u"; }
};

template<accessType aType, numberType nType, unsigned int Dim, samplerType sType = samplerType::BASIC, bool isArray = false, bool isShadow = false>
struct IsValid {
	static const bool value = false;
};

template<accessType aType, numberType nType, unsigned int Dim > struct IsValid<aType, nType, Dim> {
	static const bool value = true;
};

template<accessType aType, numberType nType, bool isArray> struct IsValid<aType, nType, 0, samplerType::CUBE, isArray> {
	static const bool value = true;
};

template<accessType aType, numberType nType> struct IsValid<aType, nType, 2, samplerType::RECTANGLE> {
	static const bool value = true;
};

template<accessType aType, numberType nType> struct IsValid<aType, nType, 0, samplerType::BUFFER> {
	static const bool value = true;
};

template<accessType aType, numberType nType, unsigned int Dim> struct IsValid<aType, nType, Dim, samplerType::BASIC, true> {
	static_assert(Dim <= 2, "Sampler Array, Dim <= 2");
	static const bool value = true;
};

template<accessType aType, numberType nType, bool isArray> struct IsValid<aType, nType, 2, samplerType::MULTI_SAMPLE, isArray> {
	static const bool value = true;
};

template<accessType aType> struct IsValid<aType, numberType::FLOAT, 2, samplerType::RECTANGLE, false, true> {
	static const bool value = true;
};

template<unsigned int Dim, bool isArray> struct IsValid<accessType::SAMPLER, numberType::FLOAT, Dim, samplerType::BASIC, isArray, true> {
	static_assert(Dim <= 2, "Sampler Shadow, Dim <= 2");
	static const bool value = true;
};

template<numberType nType, bool isArray> struct IsValid<accessType::SAMPLER, nType, 0, samplerType::CUBE, isArray, true> {
	static const bool value = true;
};

template<> struct IsValid<accessType::SAMPLER, numberType::UINT, 0, samplerType::ATOMIC> {
	static const bool value = true;
};

template<
	accessType aType,
	numberType nType,
	unsigned int Dim,
	samplerType sType = samplerType::BASIC,
	bool isArray = false,
	bool isShadow = false
> class Sampler {
public:
	static_assert(Dim <= 3, "Dim must be <= 3");
	static_assert(IsSamplerType<nType>::value, "Invalide sampler nType");
	static_assert(IsValid<aType, nType, Dim, sType, isArray, isShadow>::value, "Invalide sampler template arguments");

	static const int D = Dim;
	static const bool v = IsValid <aType, nType, Dim, sType, isArray, isShadow>::value;
	static const std::string typeStr() {
		return IsSamplerType<nType>::str() + AccessTypeInfo<aType>::str() + 
			(Dim != 0 ? std::to_string(Dim) + "D" : "") + SamplerTypeInfo<sType>::str() +
			(isArray ? "Array" : "") + (isShadow ? "Shadow" : "");
	}
};
 
using sampler1D =				Sampler < accessType::SAMPLER, numberType::FLOAT, 1 >;
using sampler2D =				Sampler < accessType::SAMPLER,  numberType::FLOAT, 2 >;
using sampler3D =				Sampler < accessType::SAMPLER,  numberType::FLOAT, 3 >;
using samplerCube =				Sampler < accessType::SAMPLER,  numberType::FLOAT, 0, samplerType::CUBE >;
using sampler2DRect =			Sampler < accessType::SAMPLER,  numberType::FLOAT, 2, samplerType::RECTANGLE >;
using sampler1DArray =			Sampler < accessType::SAMPLER,  numberType::FLOAT, 1, samplerType::BASIC, true >;
using sampler2DArray =			Sampler < accessType::SAMPLER,  numberType::FLOAT, 2, samplerType::BASIC, true >;
using samplerBuffer =			Sampler < accessType::SAMPLER,  numberType::FLOAT, 0, samplerType::BUFFER >;
using sampler2DMS =				Sampler < accessType::SAMPLER,  numberType::FLOAT, 2, samplerType::MULTI_SAMPLE >;
using sampler2DMSArray =		Sampler < accessType::SAMPLER,  numberType::FLOAT, 2, samplerType::MULTI_SAMPLE, true>;
using samplerCubeArray =		Sampler < accessType::SAMPLER,  numberType::FLOAT, 0, samplerType::CUBE, true >;
using sampler1DShadow =			Sampler < accessType::SAMPLER,  numberType::FLOAT, 1, samplerType::BASIC, false, true >;
using sampler2DShadow =			Sampler < accessType::SAMPLER,  numberType::FLOAT, 2, samplerType::BASIC, false, true >;
using sampler2DRectShadow =		Sampler < accessType::SAMPLER,  numberType::FLOAT, 2, samplerType::RECTANGLE, false, true>;
using sampler1DArrayShadow =	Sampler < accessType::SAMPLER,  numberType::FLOAT, 1, samplerType::BASIC, true, true >;
using sampler2DArrayShadow =	Sampler < accessType::SAMPLER,  numberType::FLOAT, 2, samplerType::BASIC, true, true >;
using samplerCubeShadow =		Sampler < accessType::SAMPLER,  numberType::FLOAT, 0, samplerType::CUBE, false, true >;
using samplerCubeArrayShadow =	Sampler < accessType::SAMPLER,  numberType::FLOAT, 0, samplerType::CUBE, true, true >;

using isampler1D = Sampler < accessType::SAMPLER,  numberType::INT, 1 >;
using isampler2D = Sampler < accessType::SAMPLER,  numberType::INT, 2 >;
using isampler3D = Sampler < accessType::SAMPLER,  numberType::INT, 3 >;
using isamplerCube = Sampler < accessType::SAMPLER,  numberType::INT, 0, samplerType::CUBE >;
using isampler2DRect = Sampler < accessType::SAMPLER,  numberType::INT, 2, samplerType::RECTANGLE >;
using isampler1DArray = Sampler < accessType::SAMPLER,  numberType::INT, 1, samplerType::BASIC, true >;
using isampler2DArray = Sampler < accessType::SAMPLER,  numberType::INT, 2, samplerType::BASIC, true >;
using isamplerBuffer = Sampler < accessType::SAMPLER,  numberType::INT, 0, samplerType::BUFFER >;
using isampler2DMS = Sampler < accessType::SAMPLER,  numberType::INT, 2, samplerType::MULTI_SAMPLE >;
using isampler2DMSArray = Sampler < accessType::SAMPLER,  numberType::INT, 2, samplerType::MULTI_SAMPLE, true>;
using isamplerCubeArray = Sampler < accessType::SAMPLER,  numberType::INT, 0, samplerType::CUBE, true >;

using atomic_uint = Sampler<accessType::SAMPLER, numberType::UINT, 0, samplerType::ATOMIC>;
using usampler1D = Sampler < accessType::SAMPLER,  numberType::UINT, 1 >;
using usampler2D = Sampler < accessType::SAMPLER,  numberType::UINT, 2 >;
using usampler3D = Sampler < accessType::SAMPLER,  numberType::UINT, 3 >;
using usamplerCube = Sampler < accessType::SAMPLER,  numberType::UINT, 0, samplerType::CUBE >;
using usampler2DRect = Sampler < accessType::SAMPLER,  numberType::UINT, 2, samplerType::RECTANGLE >;
using usampler1DArray = Sampler < accessType::SAMPLER,  numberType::UINT, 1, samplerType::BASIC, true >;
using usampler2DArray = Sampler < accessType::SAMPLER,  numberType::UINT, 2, samplerType::BASIC, true >;
using usamplerBuffer = Sampler < accessType::SAMPLER,  numberType::UINT, 0, samplerType::BUFFER >;
using usampler2DMS = Sampler < accessType::SAMPLER,  numberType::UINT, 2, samplerType::MULTI_SAMPLE >;
using usampler2DMSArray = Sampler < accessType::SAMPLER,  numberType::UINT, 2, samplerType::MULTI_SAMPLE, true>;
using usamplerCubeArray = Sampler < accessType::SAMPLER,  numberType::UINT, 0, samplerType::CUBE, true >;

using image1D = Sampler < accessType::IMAGE, numberType::FLOAT, 1 >;
using image2D = Sampler < accessType::IMAGE, numberType::FLOAT, 2 >;
using image3D = Sampler < accessType::IMAGE, numberType::FLOAT, 3 >;
using imageCube = Sampler < accessType::IMAGE, numberType::FLOAT, 0, samplerType::CUBE >;
using image2DRect = Sampler < accessType::IMAGE, numberType::FLOAT, 2, samplerType::RECTANGLE >;
using image1DArray = Sampler < accessType::IMAGE, numberType::FLOAT, 1, samplerType::BASIC, true >;
using image2DArray = Sampler < accessType::IMAGE, numberType::FLOAT, 2, samplerType::BASIC, true >;
using imageBuffer = Sampler < accessType::IMAGE, numberType::FLOAT, 0, samplerType::BUFFER >;
using image2DMS = Sampler < accessType::IMAGE, numberType::FLOAT, 2, samplerType::MULTI_SAMPLE >;
using image2DMSArray = Sampler < accessType::IMAGE, numberType::FLOAT, 2, samplerType::MULTI_SAMPLE, true>;
using imageCubeArray = Sampler < accessType::IMAGE, numberType::FLOAT, 0, samplerType::CUBE, true >;

using iimage1D = Sampler < accessType::IMAGE, numberType::INT, 1 >;
using iimage2D = Sampler < accessType::IMAGE, numberType::INT, 2 >;
using iimage3D = Sampler < accessType::IMAGE, numberType::INT, 3 >;
using iimageCube = Sampler < accessType::IMAGE, numberType::INT, 0, samplerType::CUBE >;
using iimage2DRect = Sampler < accessType::IMAGE, numberType::INT, 2, samplerType::RECTANGLE >;
using iimage1DArray = Sampler < accessType::IMAGE, numberType::INT, 1, samplerType::BASIC, true >;
using iimage2DArray = Sampler < accessType::IMAGE, numberType::INT, 2, samplerType::BASIC, true >;
using iimageBuffer = Sampler < accessType::IMAGE, numberType::INT, 0, samplerType::BUFFER >;
using iimage2DMS = Sampler < accessType::IMAGE, numberType::INT, 2, samplerType::MULTI_SAMPLE >;
using iimage2DMSArray = Sampler < accessType::IMAGE, numberType::INT, 2, samplerType::MULTI_SAMPLE, true>;
using iimageCubeArray = Sampler < accessType::IMAGE, numberType::INT, 0, samplerType::CUBE, true >;

using uimage1D = Sampler < accessType::IMAGE, numberType::UINT, 1 >;
using uimage2D = Sampler < accessType::IMAGE, numberType::UINT, 2 >;
using uimage3D = Sampler < accessType::IMAGE, numberType::UINT, 3 >;
using uimageCube = Sampler < accessType::IMAGE, numberType::UINT, 0, samplerType::CUBE >;
using uimage2DRect = Sampler < accessType::IMAGE, numberType::UINT, 2, samplerType::RECTANGLE >;
using uimage1DArray = Sampler < accessType::IMAGE, numberType::UINT, 1, samplerType::BASIC, true >;
using uimage2DArray = Sampler < accessType::IMAGE, numberType::UINT, 2, samplerType::BASIC, true >;
using uimageBuffer = Sampler < accessType::IMAGE, numberType::UINT, 0, samplerType::BUFFER >;
using uimage2DMS = Sampler < accessType::IMAGE, numberType::UINT, 2, samplerType::MULTI_SAMPLE >;
using uimage2DMSArray = Sampler < accessType::IMAGE, numberType::UINT, 2, samplerType::MULTI_SAMPLE, true>;
using uimageCubeArray = Sampler < accessType::IMAGE, numberType::UINT, 0, samplerType::CUBE, true >;

template<> const std::string atomic_uint::typeStr() { return "atomic_uint"; }