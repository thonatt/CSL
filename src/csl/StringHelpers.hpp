#pragma once

#include <string>
#include <map>
#include "TypesHelpers.hpp"

#include <boost/preprocessor/seq/for_each_i.hpp>

namespace csl {

	using stringPtr = std::shared_ptr<std::string>;

	template<typename T>
	struct TypeStr {
		static std::string str(int trailing) { return T::typeStr(trailing); }
	};

	template<typename T>
	std::string getTypeStr(int trailing = 0) {
		return TypeStr<T>::str(trailing);
	};

	template<typename T>
	struct TypeNamingStr {
		static std::string str(int trailing = 0) { return T::typeNamingStr(trailing); }
	};

	template<typename T>
	std::string getTypeNamingStr() {
		return TypeNamingStr<T>::str();
	}

	template<typename T>
	struct TypeStrRHS {
		static std::string str() { return getTypeStr<T>(); }
	};

	template<uint N>
	std::string arrayStr() { return "[" + (N > 0 ? std::to_string(N) : "") + "]"; }

	template<typename T, uint N>
	struct TypeStrRHS<Array<T, N>> {
		static std::string str() { return TypeStrRHS<T>::str() + arrayStr<N>(); }
	};

	template<typename ... Ts> struct MultipleTypeStr;

	template<> struct MultipleTypeStr<>
	{
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
			return TypeStr<T>::str() + ", " + MultipleTypeStr<U, Ts...>::str();
		}
	};

	// specialization for cpp types

	template<> inline std::string TypeStr<void>::str(int trailing) { return "void"; }

	// helper for scalar types prefix

	template<ScalarType nType> struct TypePrefixStr {
		static std::string str();
	};

	template<> inline std::string TypePrefixStr<BOOL>::str() { return "b"; }
	template<> inline std::string TypePrefixStr<INT>::str() { return "i"; }
	template<> inline std::string TypePrefixStr<UINT>::str() { return "u"; }
	template<> inline std::string TypePrefixStr<FLOAT>::str() { return ""; }
	template<> inline std::string TypePrefixStr<DOUBLE>::str() { return "d"; }

	template<ScalarType nType> struct LitteralSufffixStr {
		static std::string str() { return ""; }
	};
	template<> inline std::string LitteralSufffixStr<UINT>::str() { return "u"; }

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

	//template<> struct TypeStr<EmptyType> {
	//	static std::string str(int trailing = 0) {
	//		return "";
	//	}
	//};

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

	template<ScalarType type, uint N, uint M>
	struct TypeNamingStr<Matrix<type, N, M>> {
		static std::string str(int trailing = 0) { return TypeStr<Matrix<type, N, M>>::str(trailing); }
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

	template<> inline std::string TypeStr<atomic_uint>::str(int trailing) { return "atomic_uint"; }

	template<AccessType aType, ScalarType nType, uint N, SamplerType sType, uint flags>
	struct TypeNamingStr<Sampler<aType, nType, N, sType, flags>> {
		static std::string str(int trailing = 0) { return TypeStr<Sampler<aType, nType, N, sType, flags>>::str(trailing); }
	};

	// layout types

	static std::map<LayoutQualifier, std::string> layoutQualifiersKeywords = {
		{SHARED, "shared"}, {PACKED, "packed"} , {STD140, "std140"}, {STD430, "std430"},
		{ROW_MAJOR,"row_major"}, {COLUMN_MAJOR, "column_major"},
		{BINDING, "binding"},
		{LOCATION, "location"},
		{POINTS, "points"},{LINES, "lines"},{TRIANGLES, "triangles"},
		{LINE_STRIP, "line_strip"},{TRIANGLE_STRIP, "triangle_strip"},
		{MAX_VERTICES, "max_vertices"}
	};

	static inline const std::string & layoutQualifierKeyword(LayoutQualifier layoutQualifier) {
		return layoutQualifiersKeywords[layoutQualifier];
	}

	template<QualifierType t>
	struct QualifierTypeStr {
		static std::string str();
	};
	template<> inline std::string QualifierTypeStr<UNIFORM>::str() { return "uniform "; }
	template<> inline std::string QualifierTypeStr<IN>::str() { return "in "; }
	template<> inline std::string QualifierTypeStr<OUT>::str() { return "out "; }
	template<> inline std::string QualifierTypeStr<EMPTY_QUALIFIER>::str() { return ""; }

	template<LayoutQualifier lq>
	struct TypeStr<LayoutQArg<lq>> {
		static std::string str() { return layoutQualifierKeyword(lq); }
	};

	template<LayoutQualifier lq, uint N>
	struct TypeStr<LayoutQArgValue<lq, N> > {
		static std::string str() {
			return layoutQualifierKeyword(lq) + " = " + std::to_string(N);
		}
	};


	template< typename ... LayoutArgs>
	struct TypeStr< TList<LayoutArgs... > > {
		static std::string str() {
			if (sizeof...(LayoutArgs) == 0) {
				return "";
			} else {
				return "layout(" + MultipleTypeStr<LayoutArgs...>::str() + ") ";
			}
		}
	};

	template<QualifierType qType, typename T, typename ... LayoutArgs>
	struct TypeStr < Qualifier<qType, T, Layout<LayoutArgs...> > > {
		static std::string str(int trailing = 0) {
			return TypeStr<typename Layout<LayoutArgs...>::CleanupArgs>::str() + QualifierTypeStr<qType>::str() + TypeStr<T>::str(trailing);
		}
	};

	template<QualifierType qType, typename T, typename ... LayoutArgs>
	struct TypeNamingStr < Qualifier<qType, T, Layout<LayoutArgs...> > > {
		static std::string str(int trailing = 0) { return TypeStr<T>::str(trailing); }
	};

	template<typename T, uint N>
	struct TypeNamingStr< Array<T, N> > {
		static std::string str() {
			return "array_" + TypeNamingStr<T>::str() + "_" + std::to_string(N);
		}
	};

	template<typename T>
	struct DeclarationStr {
		static std::string str(const std::string & name, int trailing = 0) { return getTypeStr<T>(trailing) + " " + name; }
	};

	template<> struct DeclarationStr<EmptyType> {
		static std::string str(const std::string & name, int trailing = 0) { return name; }
	};

	template<QualifierType qType, typename T, typename Layout>
	struct DeclarationStr<Qualifier<qType, T, Layout> > {
		static std::string str(const std::string & name, int trailing = 0) {
			return TypeStr<typename Layout::CleanupArgs>::str() + QualifierTypeStr<qType>::str() + DeclarationStr<T>::str(name, trailing);
		}
	};

	template<typename T, uint N>
	struct DeclarationStr<Array<T, N>> {
		static std::string str(const std::string & name, int trailing = 0) {
			return DeclarationStr<T>::str(name, trailing) + arrayStr<N>();
		}
	};

	template<QualifierType qType, typename T, uint N, typename Layout>
	struct DeclarationStr<Qualifier<qType, Array<T, N>, Layout> > {
		static std::string str(const std::string & name, int trailing = 0) {
			return DeclarationStr<Array<Qualifier<qType, T, Layout >, N>>::str(name, trailing);
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

#define GL_VERSION_STR_IT(r, data, i, elem) GL_VERSION_STR(elem);
	BOOST_PP_SEQ_FOR_EACH_I(GL_VERSION_STR_IT, , \
		(110) (120) (130) (140) (150) \
		(330) \
		(400) (410) (420) (430) (440) (450));

} //namespace csl
