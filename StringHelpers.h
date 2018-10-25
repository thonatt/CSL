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

template<>
struct TypeStr<void> {
	static const std::string str() { return "void"; }
};

// helper for scalar types prefix

template<ScalarType nType> struct TypePrefixStr;

template<> struct TypePrefixStr<BOOL> {
	static const std::string str() { return "b"; }
};
template<> struct TypePrefixStr<INT> {
	static const std::string str() { return "i"; }
};
template<> struct TypePrefixStr<UINT> {
	static const std::string str() { return "u"; }
};
template<> struct TypePrefixStr<FLOAT> {
	static const std::string str() { return ""; }
};
template<> struct TypePrefixStr<DOUBLE> {
	static const std::string str() { return "d"; }
};

// specialization for glsl types

template<ScalarType type, uint N, AssignType assignable>
struct TypeStr< Vec<type, N, assignable> > {
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

