#pragma once

#include <string>
#include <map>
#include <memory>
#include "TypesHelpers.hpp"


namespace csl {

	namespace core {

		using string = std::string;
		using stringPtr = ::std::shared_ptr<string>;

		template<typename T>
		struct TypeStr {
			static string str(int trailing) { return T::typeStr(trailing); }
		};

		template<typename T>
		string getTypeStr(int trailing = 0) {
			return TypeStr<T>::str(trailing);
		};

		template<typename T>
		struct TypeNamingStr {
			static string str(int trailing = 0) { return T::typeNamingStr(trailing); }
		};

		enum class NamingCaterogy { SCALAR, VECTOR, MATRIX, SAMPLER, ARRAY, FUNCTION, OTHER };

		struct CounterData {
			size_t value = 0;
			bool is_tracked = false;
		};

		template<NamingCaterogy cat>
		class NamingCounter {

		public:
			static string getNextName();

		protected:
			static string baseName();
			static CounterData counterData;
		};
		template<NamingCaterogy cat> CounterData NamingCounter<cat>::counterData = {};

		template<> inline string NamingCounter<NamingCaterogy::SCALAR>::baseName() {
			return "x";
		}
		template<> inline string NamingCounter<NamingCaterogy::VECTOR>::baseName() {
			return "v";
		}
		template<> inline string NamingCounter<NamingCaterogy::MATRIX>::baseName() {
			return "m";
		}
		template<> inline string NamingCounter<NamingCaterogy::SAMPLER>::baseName() {
			return "s";
		}
		template<> inline string NamingCounter<NamingCaterogy::ARRAY>::baseName() {
			return "A";
		}
		template<> inline string NamingCounter<NamingCaterogy::FUNCTION>::baseName() {
			return "f";
		}
		template<> inline string NamingCounter<NamingCaterogy::OTHER>::baseName() {
			return "X";
		}
		template<typename T> struct AutoNaming {
			using Type = NamingCounter<NamingCaterogy::OTHER>;
		};

		template<ScalarType type> struct AutoNaming<Matrix<type, 1, 1>> {
			using Type = NamingCounter<NamingCaterogy::SCALAR>;
		};

		template<ScalarType type, uint N> struct AutoNaming<Vec<type, N>> {
			using Type = NamingCounter<NamingCaterogy::VECTOR>;
		};

		template<ScalarType type, uint N, uint M> struct AutoNaming<Matrix<type, N, M> > {
			using Type = NamingCounter<NamingCaterogy::MATRIX>;
		};

		template<typename T>
		string getTypeNamingStr() {
			return TypeNamingStr<T>::str();
		}

		template<typename T>
		struct TypeStrRHS {
			static string str() { return getTypeStr<T>(); }
		};

		template<uint N>
		string arrayStr() { return "[" + (N > 0 ? std::to_string(N) : "") + "]"; }

		template<typename T, uint N>
		struct TypeStrRHS<Array<T, N>> {
			static string str() { return TypeStrRHS<T>::str() + arrayStr<N>(); }
		};

		template<typename ... Ts> struct MultipleTypeStr;

		template<> struct MultipleTypeStr<>
		{
			static string str() {
				return "";
			}
		};

		template<typename T> struct MultipleTypeStr<T> {
			static string str() {
				return TypeStr<T>::str();
			}
		};

		template<typename T, typename U, typename ... Ts> struct MultipleTypeStr<T, U, Ts...> {
			static string str() {
				return TypeStr<T>::str() + ", " + MultipleTypeStr<U, Ts...>::str();
			}
		};

		// specialization for cpp types

		template<> inline string TypeStr<void>::str(int trailing) { return "void"; }

		// helper for scalar types prefix

		template<ScalarType nType> struct TypePrefixStr {
			static string str();
		};

		template<> inline string TypePrefixStr<BOOL>::str() { return "b"; }
		template<> inline string TypePrefixStr<INT>::str() { return "i"; }
		template<> inline string TypePrefixStr<UINT>::str() { return "u"; }
		template<> inline string TypePrefixStr<FLOAT>::str() { return ""; }
		template<> inline string TypePrefixStr<DOUBLE>::str() { return "d"; }

		template<ScalarType nType> struct LitteralSufffixStr {
			static string str() { return ""; }
		};
		template<> inline string LitteralSufffixStr<UINT>::str() { return "u"; }

		//helper for sampler types

		template<AccessType t>
		struct AccessTypeInfo {
			static string str();
		};
		template<> inline string AccessTypeInfo<SAMPLER>::str() { return "sampler"; }
		template<> inline string AccessTypeInfo<IMAGE>::str() { return "image"; }

		template<SamplerType t>
		struct SamplerTypeInfo {
			static string str();
		};
		template<> inline string SamplerTypeInfo<BASIC>::str() { return ""; }
		template<> inline string SamplerTypeInfo<CUBE>::str() { return "Cube"; }
		template<> inline string SamplerTypeInfo<RECTANGLE>::str() { return "Rect"; }
		template<> inline string SamplerTypeInfo<MULTI_SAMPLE>::str() { return "MS"; }
		template<> inline string SamplerTypeInfo<BUFFER>::str() { return "Buffer"; }

		// specialization for glsl types

		//algebra types

		template<ScalarType type, uint N>
		struct TypeStr<Vec<type, N>> {
			static string str(int trailing = 0) {
				return TypePrefixStr<type>::str() + "vec" + std::to_string(N);
			}
		};

		template<ScalarType type, uint N, uint M>
		struct TypeStr< Matrix<type, N, M> > {
			static string str(int trailing = 0) {
				return TypePrefixStr<type>::str() + "mat" + std::to_string(N) + (N == M ? string("") : "x" + std::to_string(M));
			}
		};

		template<ScalarType type, uint N, uint M>
		struct TypeNamingStr<Matrix<type, N, M>> {
			static string str(int trailing = 0) { return TypeStr<Matrix<type, N, M>>::str(trailing); }
		};

		template<> struct TypeStr<Bool> {
			static string str(int trailing = 0) { return "bool"; }
		};

		template<> struct TypeStr<Uint> {
			static string str(int trailing = 0) { return "uint"; }
		};

		template<> struct TypeStr<Int> {
			static string str(int trailing = 0) { return "int"; }
		};

		template<> struct TypeStr<Float> {
			static string str(int trailing = 0) { return "float"; }
		};

		template<> struct TypeStr<Double> {
			static string str(int trailing = 0) { return "double"; }
		};

		//for debug
		template<> struct TypeStr<bool> {
			static string str(int trailing = 0) { return "std bool"; }
		};
		template<> struct TypeStr<int> {
			static string str(int trailing = 0) { return "std int"; }
		};
		template<> struct TypeStr<double> {
			static string str(int trailing = 0) { return "std double"; }
		};

		// sampler types

		template<AccessType aType, ScalarType nType, uint N, SamplerType sType, uint flags>
		struct TypeStr<Sampler<aType, nType, N, sType, flags>> {
			static string str(int trailing = 0) {
				return
					TypePrefixStr<nType>::str() +
					AccessTypeInfo<aType>::str() +
					(N != 0 ? std::to_string(N) + "D" : "") +
					SamplerTypeInfo<sType>::str() +
					(flags & IS_ARRAY ? "Array" : "") +
					(flags & IS_SHADOW ? "Shadow" : "");
			}
		};

		template<AccessType aType, ScalarType nType, uint N, SamplerType sType, uint flags>
		struct AutoNaming<Sampler<aType, nType, N, sType, flags>> {
			using Type = NamingCounter<NamingCaterogy::SAMPLER>;
		};


		template<> inline string TypeStr<atomic_uint>::str(int trailing) { return "atomic_uint"; }

		template<AccessType aType, ScalarType nType, uint N, SamplerType sType, uint flags>
		struct TypeNamingStr<Sampler<aType, nType, N, sType, flags>> {
			static string str(int trailing = 0) { return TypeStr<Sampler<aType, nType, N, sType, flags>>::str(trailing); }
		};

		// layout types

		static const ::std::map<LayoutQualifier, string> layoutQualifiersKeywords = {
			{SHARED, "shared"}, {PACKED, "packed"}, {STD140, "std140"}, {STD430, "std430"},
			{ROW_MAJOR,"row_major"}, {COLUMN_MAJOR, "column_major"},
			{BINDING, "binding"},
			{LOCATION, "location"},
			{POINTS, "points"}, {LINES, "lines"}, {TRIANGLES, "triangles"},
			{LINE_STRIP, "line_strip"}, {TRIANGLE_STRIP, "triangle_strip"},
			{MAX_VERTICES, "max_vertices"},
			{EARLY_FRAGMENT_TEST, "early_fragment_tests"}
		};

		static inline const string& layoutQualifierKeyword(LayoutQualifier layoutQualifier) {
			return layoutQualifiersKeywords.at(layoutQualifier);
		}

		template<QualifierType t>
		struct QualifierTypeStr {
			static string str();
		};
		template<> inline string QualifierTypeStr<UNIFORM>::str() { return "uniform "; }
		template<> inline string QualifierTypeStr<IN>::str() { return "in "; }
		template<> inline string QualifierTypeStr<OUT>::str() { return "out "; }
		template<> inline string QualifierTypeStr<INOUT>::str() { return "inout "; }
		template<> inline string QualifierTypeStr<EMPTY_QUALIFIER>::str() { return ""; }

		template<LayoutQualifier lq>
		struct TypeStr<LayoutQArg<lq>> {
			static string str() { return layoutQualifierKeyword(lq); }
		};

		template<LayoutQualifier lq, uint N>
		struct TypeStr<LayoutQArgValue<lq, N> > {
			static string str() {
				return layoutQualifierKeyword(lq) + " = " + std::to_string(N);
			}
		};


		template< typename ... LayoutArgs>
		struct TypeStr< TList<LayoutArgs... > > {
			static string str() {
				if (sizeof...(LayoutArgs) == 0) {
					return "";
				} else {
					return "layout(" + MultipleTypeStr<LayoutArgs...>::str() + ") ";
				}
			}
		};

		template<QualifierType qType, typename T, typename ... LayoutArgs>
		struct TypeStr < Qualifier<qType, T, LayoutImpl<LayoutArgs...> > > {
			static string str(int trailing = 0) {
				return TypeStr<typename LayoutImpl<LayoutArgs...>::CleanupArgs>::str() + QualifierTypeStr<qType>::str() + TypeStr<T>::str(trailing);
			}
		};

		template<QualifierType qType, typename T, typename ... LayoutArgs>
		struct AutoNaming<Qualifier<qType, T, LayoutImpl<LayoutArgs...> >> {
			using Type = typename AutoNaming<T>::Type;
		};

		template<QualifierType qType, typename T, typename ... LayoutArgs>
		struct TypeNamingStr < Qualifier<qType, T, LayoutImpl<LayoutArgs...> > > {
			static string str(int trailing = 0) { return TypeStr<T>::str(trailing); }
		};

		template<typename T, uint N>
		struct AutoNaming<Array<T, N>> {
			using Type = NamingCounter<NamingCaterogy::ARRAY>;
		};

		template<typename T, uint N>
		struct TypeNamingStr< Array<T, N> > {
			static string str() {
				return "array_" + TypeNamingStr<T>::str() + "_" + std::to_string(N);
			}
		};

		template<typename T>
		struct DeclarationStr {
			static string str(const string & name, int trailing = 0) { return getTypeStr<T>(trailing) + " " + name; }
		};

		template<> struct DeclarationStr<EmptyType> {
			static string str(const string & name, int trailing = 0) { return name; }
		};

		template<QualifierType qType, typename T, typename Layout>
		struct DeclarationStr<Qualifier<qType, T, Layout> > {
			static string str(const string & name, int trailing = 0) {
				return TypeStr<typename Layout::CleanupArgs>::str() + QualifierTypeStr<qType>::str() + DeclarationStr<T>::str(name, trailing);
			}
		};

		template<typename T, uint N>
		struct DeclarationStr<Array<T, N>> {
			static string str(const string & name, int trailing = 0) {
				return DeclarationStr<T>::str(name, trailing) + arrayStr<N>();
			}
		};

		template<QualifierType qType, typename T, uint N, typename Layout>
		struct DeclarationStr<Qualifier<qType, Array<T, N>, Layout> > {
			static string str(const string & name, int trailing = 0) {
				return DeclarationStr<Array<Qualifier<qType, T, Layout >, N>>::str(name, trailing);
			}
		};


		enum GLVersion : uint {
			GLSL_110 = 110, GLSL_120 = 120, GLSL_130 = 130, GLSL_140 = 140, GLSL_150 = 150,
			GLSL_330 = 330,
			GLSL_400 = 400, GLSL_410 = 410, GLSL_420 = 420, GLSL_430 = 430, GLSL_440 = 440, GLSL_450 = 450,

			SHADERTOY
		};

		template<GLVersion v>
		constexpr string gl_version_str() { return std::to_string(v); }
	
	}

} //namespace csl
