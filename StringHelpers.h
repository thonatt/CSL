#pragma once

#include <string>
#include "TypesHelpers.h"

template<typename T>
struct TypeStr {
	static std::string str() { return T::typeStr(); }
};

template<typename T>
std::string getTypeStr() {
	return TypeStr<T>::str();
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
	static std::string str(bool previous_str = false) {
		return "";
	}
};

template<typename T> struct MultipleTypeStr<T> {
	static std::string str(bool previous_str = false) {
		return TypeStr<T>::str();
	}
};

template<typename T, typename U, typename ... Ts> struct MultipleTypeStr<T, U, Ts...> {
	static std::string str(bool previous_str = false) {
		const bool empty = (MultipleTypeStr<T>::str() == "");

		std::string out = MultipleTypeStr<U, Ts...>::str(!empty || previous_str);
		if (empty) {
			return out;
		} else {
			return (previous_str ? ", " : " ") + MultipleTypeStr<T>::str() + out;
		}
	}
};

// specialization for cpp types

template<> std::string TypeStr<void>::str() { return "void"; }

// helper for scalar types prefix

template<ScalarType nType> struct TypePrefixStr {
	static std::string str();
};

template<> std::string TypePrefixStr<BOOL>::str() { return "b"; }
template<> std::string TypePrefixStr<INT>::str() { return "i"; }
template<> std::string TypePrefixStr<UINT>::str() { return "u"; }
template<> std::string TypePrefixStr<FLOAT>::str() { return ""; }
template<> std::string TypePrefixStr<DOUBLE>::str() { return "d"; }

//helper for sampler types

template<AccessType t>
struct AccessTypeInfo {
	static std::string str();
};
template<> std::string AccessTypeInfo<SAMPLER>::str() { return "sampler"; }
template<> std::string AccessTypeInfo<IMAGE>::str() { return "image"; }

template<SamplerType t> 
struct SamplerTypeInfo {
	static std::string str();
};
template<> std::string SamplerTypeInfo<BASIC>::str() { return ""; }
template<> std::string SamplerTypeInfo<CUBE>::str() { return "Cube"; }
template<> std::string SamplerTypeInfo<RECTANGLE>::str() { return "Rect"; }
template<> std::string SamplerTypeInfo<MULTI_SAMPLE>::str() { return "MS"; }
template<> std::string SamplerTypeInfo<BUFFER>::str() { return "Buffer"; }

// specialization for glsl types

//algebra types

template<ScalarType type, uint N>
struct TypeStr<Vec<type, N>> {
	static std::string str() {
		return TypePrefixStr<type>::str() + "vec" + std::to_string(N);
	}
};

template<ScalarType type, uint N, uint M>
struct TypeStr< Matrix<type, N, M> > {
	static std::string str() {
		return TypePrefixStr<type>::str() + "mat" + std::to_string(N) + (N == M ? std::string("") : "x" + std::to_string(M));
	}
};

template<> struct TypeStr<Bool> {
	static std::string str() { return "bool"; }
};

template<> struct TypeStr<Uint> {
	static std::string str() { return "uint"; }
};

template<> struct TypeStr<Int> {
	static std::string str() { return "int"; }
};

template<> struct TypeStr<Float> {
	static std::string str() { return "float"; }
};

template<> struct TypeStr<Double> {
	static std::string str() { return "double"; }
};

//for debug
template<> struct TypeStr<bool> {
	static std::string str() { return "std bool"; }
};
template<> struct TypeStr<int> {
	static std::string str() { return "std int"; }
};
template<> struct TypeStr<double> {
	static std::string str() { return "std double"; }
};

// sampler types

template<AccessType aType, ScalarType nType, uint N, SamplerType sType, uint flags>
struct TypeStr<Sampler<aType, nType, N, sType, flags>> {
	static std::string str() {
		return
			TypePrefixStr<nType>::str() + 
			AccessTypeInfo<aType>::str() +
			(N != 0 ? std::to_string(N) + "D" : "") + 
			SamplerTypeInfo<sType>::str() +
			(flags & IS_ARRAY ? "Array" : "") +
			(flags & IS_SHADOW ? "Shadow" : "");
	}
};

template<> std::string TypeStr<atomic_uint>::str() { return "atomic_uint"; }

// layout types

template<LayoutArgIntType t>
struct LayoutArgIntStr {
	static std::string str();
};
template<> std::string LayoutArgIntStr<OFFSET>::str() { return "offset"; }
template<> std::string LayoutArgIntStr<BINDING>::str() { return "binding"; }
template<> std::string LayoutArgIntStr<LOCATION>::str() { return "location"; }

template<LayoutArgBoolType t>
struct LayoutArgBoolStr {
	static std::string str();
};
template<> std::string LayoutArgBoolStr<STD140>::str() { return "std140"; }
template<> std::string LayoutArgBoolStr<STD430>::str() { return "std430"; }
template<> std::string LayoutArgBoolStr<SHARED>::str() { return "shared"; }
template<> std::string LayoutArgBoolStr<PACKED>::str() { return "packed"; }

template<QualifierType t>
struct QualifierTypeStr {
	static std::string str();
};
template<> std::string QualifierTypeStr<UNIFORM>::str() { return "uniform "; }
template<> std::string QualifierTypeStr<IN>::str() { return "in " ; }
template<> std::string QualifierTypeStr<OUT>::str() { return "out "; }

template<LayoutArgIntType type, int N>
struct TypeStr<LayoutArgInt<type,N>> {
	static std::string str() {
		return N < 0 ? std::string("") :
			(LayoutArgIntStr<type>::str() +" = " + std::to_string(N));
	}
};

template<LayoutArgBoolType type, bool b>
struct TypeStr<LayoutArgBool<type, b>> {
	static std::string str() {
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
			return "layout(" + MultipleTypeStr<LayoutArgs...>::str() + " ) ";
		}
	}
};

template<QualifierType qType, typename T, typename ... LayoutArgs>
struct TypeStr < Qualifier<qType, T, Layout<LayoutArgs...> > > {
	static std::string str() {	
		return TypeStr<typename Layout<LayoutArgs...>::CleanedArgs>::str() + QualifierTypeStr<qType>::str() + TypeStr<T>::str();
	}
};

//array 
template<typename T, uint N>
struct TypeStr< Array<T, N> > {
	static std::string str() {
		return TypeStr<T>::str();
	}
};

template<typename T, uint N>
struct TypeNamingStr< Array<T, N> > {
	static std::string str() {
		return "array_" + TypeNamingStr<T>::str();
	}
};