#pragma once

#include <string>
#include "TypesHelpers.h"

template<typename T> 
struct TypeStr {
	static const std::string str() { return T::typeStr(); }
};

template<typename T>
const std::string getTypeStr() {
	return TypeStr<T>::str();
}

// specialization for cpp types

template<> const std::string TypeStr<void>::str() { return "void"; }

// helper for scalar types prefix

template<ScalarType nType> struct TypePrefixStr {
	static const std::string str();
};

template<> const std::string TypePrefixStr<BOOL>::str() { return "b"; }
template<> const std::string TypePrefixStr<INT>::str() { return "i"; }
template<> const std::string TypePrefixStr<UINT>::str() { return "u"; }
template<> const std::string TypePrefixStr<FLOAT>::str() { return ""; }
template<> const std::string TypePrefixStr<DOUBLE>::str() { return "d"; }

//helper for sampler types

template<AccessType t>
struct AccessTypeInfo {
	static const std::string str();
};
template<> const std::string AccessTypeInfo<SAMPLER>::str() { return "sampler"; }
template<> const std::string AccessTypeInfo<IMAGE>::str() { return "image"; }

template<SamplerType t> 
struct SamplerTypeInfo {
	static const std::string str();
};
template<> const std::string SamplerTypeInfo<BASIC>::str() { return ""; }
template<> const std::string SamplerTypeInfo<CUBE>::str() { return "Cube"; }
template<> const std::string SamplerTypeInfo<RECTANGLE>::str() { return "Rect"; }
template<> const std::string SamplerTypeInfo<MULTI_SAMPLE>::str() { return "MS"; }
template<> const std::string SamplerTypeInfo<BUFFER>::str() { return "Buffer"; }

// specialization for glsl types

//algebra types

template<ScalarType type, uint N, AssignType assignable>
struct TypeStr<Vec<type, N, assignable>> {
	static const std::string str() {
		return TypePrefixStr<type>::str() + "vec" + std::to_string(N);
	}
};

template<ScalarType type, uint N, uint M, AssignType assignable>
struct TypeStr< Matrix<type, N, M, assignable> > {
	static const std::string str() {
		return TypePrefixStr<type>::str() + "mat" + std::to_string(N) + (N == M ? std::string("") : "x" + std::to_string(M));
	}
};

template<> struct TypeStr<Bool> {
	static const std::string str() { return "bool"; }
};

template<> struct TypeStr<Uint> {
	static const std::string str() { return "uint"; }
};

template<> struct TypeStr<Int> {
	static const std::string str() { return "int"; }
};

template<> struct TypeStr<Float> {
	static const std::string str() { return "float"; }
};

template<> struct TypeStr<Double> {
	static const std::string str() { return "double"; }
};

// sampler types

template<AccessType aType, ScalarType nType, uint N, SamplerType sType, SamplerIsArray isArray, SamplerIsShadow isShadow >
struct TypeStr<Sampler<aType, nType, N, sType, isArray, isShadow>> {
	static const std::string str() {
		return TypePrefixStr<nType>::str() + AccessTypeInfo<aType>::str() +
			(N != 0 ? std::to_string(N) + "D" : "") + SamplerTypeInfo<sType>::str() +
			(isArray ? "Array" : "") + (isShadow ? "Shadow" : "");
	}
};

template<> const std::string TypeStr<Sampler<SAMPLER, UINT, 0, ATOMIC>>::str() { return "atomic_uint"; }
