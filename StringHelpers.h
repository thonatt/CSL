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

template<typename ... Ts> struct MultipleTypeStr {
	static const std::string str(bool previous_str = false) {
		return "";
	}
};

template<typename T> struct MultipleTypeStr<T> {
	static const std::string str(bool previous_str = false) {
		return TypeStr<T>::str();
	}
};

template<typename T, typename U, typename ... Ts> struct MultipleTypeStr<T, U, Ts...> {
	static const std::string str(bool previous_str = false) {
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

//for debug
template<> struct TypeStr<bool> {
	static const std::string str() { return "std bool"; }
};
template<> struct TypeStr<int> {
	static const std::string str() { return "std int"; }
};
template<> struct TypeStr<double> {
	static const std::string str() { return "std double"; }
};

// sampler types

template<AccessType aType, ScalarType nType, uint N, SamplerType sType, uint flags>
struct TypeStr<Sampler<aType, nType, N, sType, flags>> {
	static const std::string str() {
		return
			TypePrefixStr<nType>::str() + 
			AccessTypeInfo<aType>::str() +
			(N != 0 ? std::to_string(N) + "D" : "") + 
			SamplerTypeInfo<sType>::str() +
			(flags & IS_ARRAY ? "Array" : "") +
			(flags & IS_SHADOW ? "Shadow" : "");
	}
};

template<> const std::string TypeStr<atomic_uint>::str() { return "atomic_uint"; }

// layout types

template<LayoutArgIntType t>
struct LayoutArgIntStr {
	static const std::string str();
};
template<> const std::string LayoutArgIntStr<OFFSET>::str() { return "offset"; }
template<> const std::string LayoutArgIntStr<BINDING>::str() { return "binding"; }
template<> const std::string LayoutArgIntStr<LOCATION>::str() { return "location"; }

template<LayoutArgBoolType t>
struct LayoutArgBoolStr {
	static const std::string str();
};
template<> const std::string LayoutArgBoolStr<STD140>::str() { return "std140"; }
template<> const std::string LayoutArgBoolStr<STD430>::str() { return "std430"; }
template<> const std::string LayoutArgBoolStr<SHARED>::str() { return "shared"; }
template<> const std::string LayoutArgBoolStr<PACKED>::str() { return "packed"; }

template<QualifierType t>
struct QualifierTypeStr {
	static const std::string str();
};
template<> const std::string QualifierTypeStr<UNIFORM>::str() { return "uniform "; }
template<> const std::string QualifierTypeStr<IN>::str() { return "in " ; }
template<> const std::string QualifierTypeStr<OUT>::str() { return "out "; }

template<LayoutArgIntType type, int N>
struct TypeStr<LayoutArgInt<type,N>> {
	static const std::string str() {
		return N < 0 ? std::string("") :
			(LayoutArgIntStr<type>::str() +" = " + std::to_string(N));
	}
};

template<LayoutArgBoolType type, bool b>
struct TypeStr<LayoutArgBool<type, b>> {
	static const std::string str() {
		return b ? LayoutArgBoolStr<type>::str() : std::string("");
	}
};

template< typename ... LayoutArgs> 
struct TypeStr< LayoutCleanedArg<LayoutArgs... > > {
	static const bool empty = Layout<LayoutArgs...>::empty;
	static const std::string str() {
		if (empty) {
			return "";
		} else {
			return "layout(" + MultipleTypeStr<LayoutArgs...>::str() + " ) ";
		}
	}
};

template<QualifierType qType, typename T, typename ... LayoutArgs>
struct TypeStr < Qualifier<qType, T, Layout<LayoutArgs...> > > {
	static const std::string str() {	
		return TypeStr<typename Layout<LayoutArgs...>::CleanedArgs>::str() + QualifierTypeStr<qType>::str() + TypeStr<T>::str();
	}
};

//array 
template<typename T, uint N>
struct TypeStr< Array<T, N> > {
	static const std::string str() {
		return TypeStr<T>::str();
	}
};