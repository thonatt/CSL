#pragma once

#include <string>
#include "TypesHelpers.h"
#include <boost/preprocessor/seq/for_each_i.hpp>

template<typename T>
struct TypeStr {
	static std::string str(int trailing = 0) { return T::typeStr(trailing); }
};

template<typename T>
std::string getTypeStr(int trailing = 0) {
	return TypeStr<T>::str(trailing);
}

template<typename T>
struct TypeNamingStr {
	static std::string str() { return TypeStr<T>::str(); }
};

template<typename T>
std::string getTypeNamingStr() {
	return TypeNamingStr<T>::str();
}

template<typename ... Ts> struct MultipleTypeStr {
	static std::string str() {
		return "";
	}
};

template<typename T> struct MultipleTypeStr<T> {
	static std::string str() {
		return TypeStr<T>::str();
	}
};

template<typename T, typename U, typename ... Ts> struct MultipleTypeStr<T, U, Ts...> {
	static std::string str() {
		const std::string current = MultipleTypeStr<T>::str();
		const std::string next = MultipleTypeStr<U, Ts...>::str();
		if (current != "" && next != "") {
			return current + ", " + next;
		}
		return current + next;
	}
};

// specialization for cpp types

template<> std::string TypeStr<void>::str(int trailing) { return "void"; }

// helper for scalar types prefix

template<ScalarType nType> struct TypePrefixStr {
	static std::string str();
};

template<> inline std::string TypePrefixStr<BOOL>::str() { return "b"; }
template<> inline std::string TypePrefixStr<INT>::str() { return "i"; }
template<> inline std::string TypePrefixStr<UINT>::str() { return "u"; }
template<> inline std::string TypePrefixStr<FLOAT>::str() { return ""; }
template<> inline std::string TypePrefixStr<DOUBLE>::str() { return "d"; }

//helper for sampler types

template<AccessType t>
struct AccessTypeInfo {
	static std::string str();
};
template<> inline std::string AccessTypeInfo<SAMPLER>::str() { return "sampler"; }
template<> inline std::string AccessTypeInfo<IMAGE>::str() { return "image"; }

template<SamplerType t> 
struct SamplerTypeInfo {
	static std::string str();
};
template<> inline std::string SamplerTypeInfo<BASIC>::str() { return ""; }
template<> inline std::string SamplerTypeInfo<CUBE>::str() { return "Cube"; }
template<> inline std::string SamplerTypeInfo<RECTANGLE>::str() { return "Rect"; }
template<> inline std::string SamplerTypeInfo<MULTI_SAMPLE>::str() { return "MS"; }
template<> inline std::string SamplerTypeInfo<BUFFER>::str() { return "Buffer"; }

// specialization for glsl types

//algebra types

template<ScalarType type, uint N>
struct TypeStr<Vec<type, N>> {
	static std::string str(int trailing = 0) {
		return TypePrefixStr<type>::str() + "vec" + std::to_string(N);
	}
};

template<ScalarType type, uint N, uint M>
struct TypeStr< Matrix<type, N, M> > {
	static std::string str(int trailing = 0) {
		return TypePrefixStr<type>::str() + "mat" + std::to_string(N) + (N == M ? std::string("") : "x" + std::to_string(M));
	}
};

template<> struct TypeStr<Bool> {
	static std::string str(int trailing = 0) { return "bool"; }
};

template<> struct TypeStr<Uint> {
	static std::string str(int trailing = 0) { return "uint"; }
};

template<> struct TypeStr<Int> {
	static std::string str(int trailing = 0) { return "int"; }
};

template<> struct TypeStr<Float> {
	static std::string str(int trailing = 0) { return "float"; }
};

template<> struct TypeStr<Double> {
	static std::string str(int trailing = 0) { return "double"; }
};

//for debug
template<> struct TypeStr<bool> {
	static std::string str(int trailing = 0) { return "std bool"; }
};
template<> struct TypeStr<int> {
	static std::string str(int trailing = 0) { return "std int"; }
};
template<> struct TypeStr<double> {
	static std::string str(int trailing = 0) { return "std double"; }
};

// sampler types

template<AccessType aType, ScalarType nType, uint N, SamplerType sType, uint flags>
struct TypeStr<Sampler<aType, nType, N, sType, flags>> {
	static std::string str(int trailing = 0) {
		return
			TypePrefixStr<nType>::str() + 
			AccessTypeInfo<aType>::str() +
			(N != 0 ? std::to_string(N) + "D" : "") + 
			SamplerTypeInfo<sType>::str() +
			(flags & IS_ARRAY ? "Array" : "") +
			(flags & IS_SHADOW ? "Shadow" : "");
	}
};

template<> std::string TypeStr<atomic_uint>::str(int trailing) { return "atomic_uint"; }

// layout types

template<LayoutArgIntType t>
struct LayoutArgIntStr {
	static std::string str();
};
template<> inline std::string LayoutArgIntStr<OFFSET>::str() { return "offset"; }
template<> inline std::string LayoutArgIntStr<BINDING>::str() { return "binding"; }
template<> inline std::string LayoutArgIntStr<LOCATION>::str() { return "location"; }

template<LayoutArgBoolType t>
struct LayoutArgBoolStr {
	static std::string str();
};
template<> inline std::string LayoutArgBoolStr<STD140>::str() { return "std140"; }
template<> inline std::string LayoutArgBoolStr<STD430>::str() { return "std430"; }
template<> inline std::string LayoutArgBoolStr<SHARED>::str() { return "shared"; }
template<> inline std::string LayoutArgBoolStr<PACKED>::str() { return "packed"; }

template<QualifierType t>
struct QualifierTypeStr {
	static std::string str();
};
template<> inline std::string QualifierTypeStr<UNIFORM>::str() { return "uniform"; }
template<> inline std::string QualifierTypeStr<IN>::str() { return "in" ; }
template<> inline std::string QualifierTypeStr<OUT>::str() { return "out"; }

template<LayoutArgIntType type, int N>
struct TypeStr<LayoutArgInt<type,N>> {
	static std::string str(int trailing = 0) {
		if (N < 0) {
			return "";
		} else {
			return LayoutArgIntStr<type>::str() + " = " + std::to_string(N);
		}
	}
};

template<LayoutArgBoolType type, bool b>
struct TypeStr<LayoutArgBool<type, b>> {
	static std::string str(int trailing = 0) {
		return b ? LayoutArgBoolStr<type>::str() : std::string("");
	}
};

template< typename ... LayoutArgs> 
struct TypeStr< LayoutCleanedArg<LayoutArgs... > > {
	static const bool empty = Layout<LayoutArgs...>::empty;
	static std::string str() {
		if (empty) {
			return "";
		} else {
			return "layout(" + MultipleTypeStr<LayoutArgs...>::str() + ") ";
		}
	}
};

template<QualifierType qType, typename T, typename ... LayoutArgs>
struct TypeStr < Qualifier<qType, T, Layout<LayoutArgs...> > > {
	static std::string str(int trailing = 0) {
		return TypeStr<typename Layout<LayoutArgs...>::CleanedArgs>::str() + QualifierTypeStr<qType>::str() + " " + TypeStr<T>::str();
	}
};

//array 
template<typename T, uint N>
struct TypeStr< Array<T, N> > {

	static std::string array_str() {
		return "[" + std::to_string(N) + "]";
	}

	static std::string str(int trailing = 0) {
		return TypeStr<T>::str();
	}
};

template<typename T, uint N>
struct TypeNamingStr< Array<T, N> > {
	static std::string str() {
		return "array_" + TypeNamingStr<T>::str();
	}
};


enum GLVersion { 
	GLSL_110, GLSL_120, GLSL_130, GLSL_140, GLSL_150,
	GLSL_330,
	GLSL_400, GLSL_410, GLSL_420, GLSL_430, GLSL_440, GLSL_450
};

template<GLVersion v>
struct GLVersionStr;

template<GLVersion v>
std::string gl_version_str() { return GLVersionStr<v>::str(); }

#define GL_VERSION_STR(ver) \
template<>  struct GLVersionStr<GLSL_ ## ver> { \
	static std::string str() { return #ver; } \
}

#define GL_VERSION_IT(r, data, i, elem) GL_VERSION_STR(elem);
BOOST_PP_SEQ_FOR_EACH_I(GL_VERSION_IT, , \
	(110) (120) (130) (140) (150) \
	(330) \
	(400) (410) (420) (430) (440) (450) );

