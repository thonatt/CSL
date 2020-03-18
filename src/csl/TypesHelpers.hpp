#pragma once

#include <tuple>
#include <utility>

namespace csl {
	
	namespace core {

		// cpp helpers
		using uint = unsigned int;

		template< class T, class U >
		constexpr bool is_same_v = std::is_same<T, U>::value;

		template< class T >
		constexpr bool is_integral_v = std::is_integral<T>::value;

		template<uint A, uint B>
		constexpr uint MaxUint = A > B ? A : B;

		template<int First, int Second>
		constexpr int Last = (Second >= 0 ? Second : First);

		//template<typename T>
		//using CleanType = std::remove_const_t<std::remove_reference_t<T>>;

		//template<typename T>
		//using CT = std::remove_const_t<std::remove_reference_t<T>>;

		// matrix types forward declarations

		enum ScalarType { BOOL, INT, UINT, FLOAT, DOUBLE, VOID, INVALID };
		enum AssignType { ASSIGNABLE, NON_ASSIGNABLE };

		template<ScalarType type, uint Nrows, uint Ncols>
		class Matrix;

		template<ScalarType type, uint N>
		using Vec = Matrix<type, N, 1>;

		template<ScalarType type>
		using Scalar = Vec<type, 1>;

		using EmptyType = Matrix<FLOAT, 0, 0>;

		// samplers types forward decalrations

		enum AccessType { SAMPLER, IMAGE };
		enum SamplerType { BASIC, CUBE, RECTANGLE, MULTI_SAMPLE, BUFFER, ATOMIC };
		//enum SamplerIsArray { NOT_ARRAY, ARRAY };
		//enum SamplerIsShadow { NOT_SHADOW, SHADOW };

		enum SamplerFlags { IS_ARRAY = 1 << 1, IS_SHADOW = 1 << 2 };

		template<
			AccessType aType,
			ScalarType nType,
			uint N,
			SamplerType sType = BASIC,
			uint flags = 0
		>
			class Sampler;

		using atomic_uint = Sampler<SAMPLER, UINT, 0, ATOMIC >;

		// layout types forward declarations



		constexpr int maxQualifierValues = 8;

		enum LayoutQualifier : int {
			SHARED = 0 * maxQualifierValues, PACKED, STD140, STD430,
			ROW_MAJOR = 1 * maxQualifierValues, COLUMN_MAJOR,
			BINDING = 2 * maxQualifierValues,
			OFFSET = 3 * maxQualifierValues,
			LOCATION = 4 * maxQualifierValues,
			POINTS = 5 * maxQualifierValues, LINES, TRIANGLES, QUADS, ISOLINES,
			LINE_STRIP = 6 * maxQualifierValues, TRIANGLE_STRIP,
			MAX_VERTICES = 7 * maxQualifierValues,
			EARLY_FRAGMENT_TEST = 8 * maxQualifierValues,
			VERTICES = 9 * maxQualifierValues,
			EQUAL_SPACING = 10 * maxQualifierValues, FRACTIONAL_EVEN_SPACING, FRACTIONAL_ODD_SPACING,
			CW = 11*maxQualifierValues, CCW
		};

		template<LayoutQualifier lq> struct LayoutQArg;
		template<LayoutQualifier lq, uint N> struct LayoutQArgValue;

		enum QualifierType {
			IN, OUT, UNIFORM, INOUT, EMPTY_QUALIFIER
		};

		template<template<typename T, typename Layout> class Quali>
		struct GetTemplateQualifierT {
			static const QualifierType value = EMPTY_QUALIFIER;
		};


		template<typename ... LayoutCleanedArgs> struct LayoutCleanedArg;

		template<typename ... LayoutArgs> struct LayoutImpl;

		template<QualifierType qType, typename T, typename L>
		struct Qualifier;

		//helper for storing Type lists

		template<typename ... Args> struct TList {
			using Tuple = std::tuple<Args...>;
			static constexpr size_t size = sizeof...(Args);

			template<size_t id> using GetType = typename std::tuple_element<id, Tuple>::type;
		};


		// lambdas helpers

		template<typename T> struct FuncPtrInfo;

		template<typename ReturnType, typename Fun, typename... Args>
		struct FuncPtrInfo< ReturnType(Fun::*)(Args...) const> {
			using ArgTup = std::tuple<Args...>;
			using ArgTList = TList<Args...>;
		};

		template<typename F>
		using LambdaInfos = FuncPtrInfo<decltype(&F::operator())>;

		template<typename F>
		using GetArgTList = typename LambdaInfos<F>::ArgTList;

		// arrays
		template<typename T, uint N = 0> struct ArrayImpl;

		// types infos

		template<typename T> struct Infos {
			static const bool is_numeric_type = false;
			static const bool is_glsl_type = false;
			static const uint rows = 0;
			static const uint cols = 0;
			static const uint array_size = 0;
			static const ScalarType scalar_type = INVALID;
		};

		template<typename T> struct Infos<T&> : Infos<T> {};
		template<typename T> struct Infos<const T> : Infos<T> {};
		template<typename T> struct Infos<const T &> : Infos<T> {};

		template<ScalarType type, uint Nrows, uint Ncols>
		struct Infos<Matrix<type, Nrows, Ncols>> {
			static const bool is_numeric_type = type != BOOL;
			static const bool is_glsl_type = true;
			static const uint rows = Nrows;
			static const uint cols = Ncols;
			static const uint array_size = 1;
			static const ScalarType scalar_type = type;
		};


		template<typename T, uint N>
		struct Infos<ArrayImpl<T, N>> {
			static const bool is_numeric_type = Infos<T>::is_numeric_type;
			static const bool is_glsl_type = Infos<T>::is_glsl_type;
			static const uint rows = Infos<T>::rows;
			static const uint cols = Infos<T>::cols;
			static const uint array_size = N;
			static const ScalarType scalar_type = Infos<T>::scalar_type;
		};

		template<> struct Infos<double> {
			static const bool is_numeric_type = true;
			static const bool is_glsl_type = false;
			static const uint rows = 1;
			static const uint cols = 1;
			static const uint array_size = 1;
			static const ScalarType scalar_type = FLOAT;
		};

		template<> struct Infos<int> {
			static const bool is_numeric_type = true;
			static const bool is_glsl_type = false;
			static const uint rows = 1;
			static const uint cols = 1;
			static const uint array_size = 1;
			static const ScalarType scalar_type = INT;
		};

		template<> struct Infos<uint> {
			static const bool is_numeric_type = true;
			static const bool is_glsl_type = false;
			static const uint rows = 1;
			static const uint cols = 1;
			static const uint array_size = 1;
			static const ScalarType scalar_type = UINT;
		};

		template<> struct Infos<bool> {
			static const bool is_numeric_type = true;
			static const bool is_glsl_type = false;
			static const uint rows = 1;
			static const uint cols = 1;
			static const uint array_size = 1;
			static const ScalarType scalar_type = BOOL;
		};

		template<> struct Infos<void> {
			static const bool is_numeric_type = false;
			static const bool is_glsl_type = false;
			static const uint rows = 0;
			static const uint cols = 0;
			static const uint array_size = 0;
			static const ScalarType scalar_type = VOID;

		};

		template<QualifierType qType, typename T, typename L>
		struct Infos<Qualifier<qType, T, L> > : Infos<T> { };

		template<typename T>
		struct SamplerInfos {
			static const bool is_sampler = false;
		};

		template<typename T> struct SamplerInfos<T&> : SamplerInfos<T> {};
		template<typename T> struct SamplerInfos<const T> : SamplerInfos<T> {};
		template<typename T> struct SamplerInfos<const T &> : SamplerInfos<T> {};


		template<AccessType aType, ScalarType nType, uint N, SamplerType sType, uint sflags> //SamplerIsArray isArray, SamplerIsShadow isShadow>
		struct SamplerInfos<Sampler<aType, nType, N, sType, sflags>> {
			static const bool is_sampler = true;
			static const uint size = N;
			static const AccessType access_type = aType;
			static const ScalarType scalar_type = nType;
			static const SamplerType type = sType;
			static const uint flags = sflags;
		};

		template<QualifierType qType, typename T, typename L>
		struct SamplerInfos<Qualifier<qType, T, L> > : SamplerInfos<T> { };

		struct RunTimeInfos {

			template<typename T>
			void fromT() {
				is_numeric_type = Infos<T>::is_numeric_type;
				is_glsl_type = Infos<T>::is_glsl_type;
				rows = Infos<T>::rows;
				cols = Infos<T>::cols;
				scalar_type = Infos<T>::scalar_type;
			}

			bool is_void() const {
				return scalar_type == VOID;
			}

			bool isConvertibleTo(const RunTimeInfos & other) const {
				bool valid = scalar_type != INVALID && other.scalar_type != INVALID;
				bool same_dim = rows == other.rows && cols == other.cols;
				bool higher_type = scalar_type <= other.scalar_type;
				return valid && same_dim && higher_type;
			}

			bool is_numeric_type;
			bool is_glsl_type;
			uint rows;
			uint cols;
			ScalarType scalar_type;
		};

		template<typename T>
		RunTimeInfos getRunTimeInfos() {
			RunTimeInfos i;
			i.fromT<T>();
			return i;
		}


		//// enable_if helpers

		// ScalarType helpers

		template<ScalarType A, ScalarType B>
		constexpr ScalarType HigherScalarType = (A > B ? A : B);

		template<ScalarType A, ScalarType B>
		constexpr ScalarType LowerScalarType = (A > B ? B : A);

		template<typename A, typename B>
		constexpr ScalarType HigherType = HigherScalarType<Infos<A>::scalar_type, Infos<B>::scalar_type>;

		template<typename A, typename B>
		constexpr ScalarType LowerType = LowerScalarType<Infos<A>::scalar_type, Infos<B>::scalar_type>;

		template<typename A, typename B>
		constexpr bool EqualType = (Infos<A>::scalar_type != INVALID) && Infos<A>::scalar_type == Infos<B>::scalar_type;

		template<typename A, typename ...Bs> struct EqualTypesT;
		template<typename A, typename ...Bs> constexpr bool EqualTypes = EqualTypesT<A, Bs...>::value;

		template<typename A> struct EqualTypesT<A> {
			static constexpr bool value = true;
		};

		template<typename A, typename B, typename ...Bs>
		struct EqualTypesT<A, B, Bs...> {
			static constexpr bool value = EqualType<A, B> && EqualTypes<A, Bs...>;
		};

		template<typename A>
		constexpr bool IsValid = Infos<A>::scalar_type != INVALID;

		template<typename A>
		constexpr bool NotBool = Infos<A>::is_numeric_type;

		template<typename A, typename B>
		constexpr bool NoBools = NotBool<A> && NotBool<B>;

		// rows/cols helpers
		template<typename A, typename B>
		constexpr uint MaxRows = MaxUint<Infos<A>::rows, Infos<B>::rows>;

		template<typename A, typename B>
		constexpr uint MaxCols = MaxUint<Infos<A>::cols, Infos<B>::cols>;

		template<typename A>
		constexpr uint NumElements = Infos<A>::rows*Infos<A>::cols;

		template<typename A>
		constexpr bool IsVector = IsValid<A> && Infos<A>::cols == 1;

		template<typename A>
		constexpr bool IsScalar = IsVector<A> && Infos<A>::rows == 1;

		template<typename A, typename B>
		constexpr bool ValidForMatMultiplication = Infos<A>::cols == Infos<B>::rows && !(IsScalar<A> || IsScalar<B>);

		template<typename ...Ts> struct MatElementsT;
		template<typename ...Ts> constexpr uint MatElements = MatElementsT<Ts...>::value;

		template<> struct MatElementsT<> {
			static const uint value = 0;
		};

		template<typename T, typename ...Ts> struct MatElementsT<T, Ts...> {
			static const uint value = NumElements<T> +MatElements<Ts...>;
		};

		// other helpers
		template<typename A>
		constexpr bool IsSquare = Infos<A>::cols == Infos<A>::rows;

		template<typename A, ScalarType type>
		constexpr bool IsVec = Infos<A>::cols == 1 && Infos<A>::scalar_type == type;

		template<typename A>
		constexpr bool IsVecD = IsVec<A, DOUBLE>;

		template<typename A>
		constexpr bool IsVecF = IsVec<A, FLOAT>;

		template<typename A>
		constexpr bool IsVecI = IsVec<A, INT>;

		template<typename A>
		constexpr bool IsVecU = IsVec<A, UINT>;

		template<typename A>
		constexpr bool IsVecB = IsVec<A, BOOL>;

		template<typename A>
		constexpr bool IsVecInteger = IsVecU<A> || IsVecI<A>;

		template<typename A>
		constexpr bool IsInt = IsScalar<A> && Infos<A>::scalar_type == INT;

		template<typename A>
		constexpr bool IsUInt = IsScalar<A> && Infos<A>::scalar_type == UINT;

		template<typename A>
		constexpr bool IsInteger = IsInt<A> || IsUInt<A>;

		template<typename A>
		constexpr bool IsFloat = IsScalar<A> && Infos<A>::scalar_type == FLOAT;

		template<typename T, QualifierType type>
		struct IsQualifierImpl {
			static constexpr bool value = false;
		};

		template<typename T, typename L, QualifierType type>
		struct IsQualifierImpl<Qualifier<type, T, L>, type> {
			static constexpr bool value = true;
		};

		template<typename T, QualifierType type>
		constexpr bool IsQuali = IsQualifierImpl<T, type>::value;

		template<typename T>
		constexpr bool FunctionOut = std::is_lvalue_reference<T>::value;

		template<typename A, typename B>
		constexpr bool SuperiorType = Infos<B>::scalar_type >= Infos<A>::scalar_type;

		template<typename A, typename B>
		constexpr bool EqualDim = IsValid<A> && IsValid<B> &&
			(Infos<A>::rows == Infos<B>::rows) && (Infos<A>::cols == Infos<B>::cols) && (Infos<A>::array_size == Infos<B>::array_size);

		template<typename A, typename B>
		constexpr bool EqualMat = EqualType<A, B> && EqualDim<A, B>;

		template<typename A, typename B>
		constexpr bool IsConvertibleTo = EqualDim<A, B> && ((EqualType<A, bool> && EqualType<B, bool>) || SuperiorType<A, B>);

		template<typename A, typename B>
		using ArithmeticBinaryReturnType = Matrix< HigherType<A, B>, MaxRows<A, B>, MaxCols<A, B> >;

		template<typename A, typename B>
		using MultiplicationReturnType = Matrix< HigherType<A, B>, Infos<A>::rows, Infos<B>::cols >;

		// variadic helpers

		template<bool ... bs> struct AllTrueT;
		template<bool ... bs> constexpr bool AllTrue = AllTrueT<bs...>::value;

		template<> struct AllTrueT<> {
			static const bool value = true;
		};
		template<bool b, bool ... bs> struct AllTrueT<b, bs...> {
			static const bool value = b && AllTrueT<bs...>::value;
		};

		template<bool ... bs> struct AnyTrueT;
		template<bool ... bs> constexpr bool AnyTrue = AnyTrueT<bs...>::value;

		template<> struct AnyTrueT<> {
			static const bool value = false;
		};
		template<bool b, bool ... bs> struct AnyTrueT<b, bs...> {
			static const bool value = b || AnyTrueT<bs...>::value;
		};

		template<typename T, typename ... Ts>
		constexpr bool ContainsType = AnyTrue<is_same_v<T, Ts>...>;

		template<typename ...Ts> struct AreValidT {
			static const bool value = AllTrue<IsValid<Ts>...>;
		};
		template<typename ...Ts> constexpr bool AreValid = AreValidT<Ts...>::value;


		template<typename ...Ts> struct SameScalarTypeT;
		template<typename ...Ts> constexpr bool SameScalarType = SameScalarTypeT<Ts...>::value;

		template<typename T> struct SameScalarTypeT<T> {
			static const bool value = true;
		};

		template<typename T, typename U, typename ...Ts> struct SameScalarTypeT<T, U, Ts...> {
			static const bool value = EqualType<T, U> && SameScalarType<U, Ts...>;
		};

		template<SamplerType input, SamplerType ... sources>
		struct IsAnySamplerType {
			static const bool value = AnyTrue<(input == sources)...>;
		};

		// TList utils

		template<typename A, size_t first, typename Range>
		struct SubsetImpl;

		template<typename A, size_t first, size_t last>
		using Subset = typename SubsetImpl<A, first, ::std::make_index_sequence<last - first>>::Type;

		template<size_t first, size_t ... Is, typename ... Ts>
		struct SubsetImpl< TList<Ts...>, first, ::std::index_sequence<Is...> > {
			using Type = TList<::std::tuple_element_t<first + Is, ::std::tuple<Ts...>> ...>;
		};

		template<typename TLA, typename TLB> struct EqualListT;
		template<typename TLA, typename TLB> constexpr bool EqualList = EqualListT<TLA, TLB>::value;

		template<typename A, typename ... As, typename B, typename ... Bs>
		struct EqualListT<TList<A, As...>, TList<B, Bs...> > {
			static constexpr bool value =
				(std::is_same<A, B>::value || EqualMat<A, B>) &&
				EqualListT<TList<As...>, TList<Bs...> >::value;
		};

		template<> struct EqualListT<TList<>, TList<> > {
			static constexpr bool value = true;
		};
		template<typename ...As> struct EqualListT<TList<As...>, TList<> > {
			static constexpr bool value = false;
		};
		template<typename ...As> struct EqualListT<TList<>, TList<As...> > {
			static constexpr bool value = false;
		};

		template<typename FList, typename ArgsList, int Id = 0> 
		struct OverloadResolution;

		template<int Id, typename ArgsTList, typename F, typename ...Fs>
		struct OverloadResolution<TList<F, Fs...>, ArgsTList, Id> {
			static constexpr int value =
				EqualList< GetArgTList<F>, ArgsTList> ?
				Id :
				OverloadResolution<TList<Fs...>, ArgsTList, Id + 1>::value;
		};

		template<int Id, typename ArgsTList>
		struct OverloadResolution<TList<>, ArgsTList, Id> {
			static constexpr int value = -1;
		};

		template<typename ReturnTypeList, typename FList, typename ArgsList>
		struct OverloadResolutionTypeImpl {
			static constexpr int id = OverloadResolution<FList, ArgsList>::value;
			static_assert(id >= 0, "no overload founded");

			static constexpr size_t valid_id = (id >= 0 ? id : 0);
			using ReturnType = std::tuple_element_t<valid_id, typename ReturnTypeList::Tuple>;
		};

		template<typename ReturnTypeList, typename FList, typename ArgsList>
		using OverloadResolutionType = typename OverloadResolutionTypeImpl<ReturnTypeList, FList, ArgsList>::ReturnType;

	}

	//
	template <typename T, T value>
	using ConstExpr = std::integral_constant<T, value>;

	template<typename T, core::uint N = 0> 
	using Array = core::ArrayImpl<T, N>;

	using Double = core::Scalar<core::DOUBLE>;
	using Float = core::Scalar<core::FLOAT>;
	using Bool = core::Scalar<core::BOOL>;
	using Uint = core::Scalar<core::UINT>;
	using Int = core::Scalar<core::INT>;

	using vec2 = core::Vec<core::FLOAT, 2>;
	using vec3 = core::Vec<core::FLOAT, 3>;
	using vec4 = core::Vec<core::FLOAT, 4>;

	using mat2x2 = core::Matrix<core::FLOAT, 2, 2>;
	using mat2x3 = core::Matrix<core::FLOAT, 3, 2>;
	using mat2x4 = core::Matrix<core::FLOAT, 4, 2>;
	using mat3x2 = core::Matrix<core::FLOAT, 2, 3>;
	using mat3x3 = core::Matrix<core::FLOAT, 3, 3>;
	using mat3x4 = core::Matrix<core::FLOAT, 4, 3>;
	using mat4x2 = core::Matrix<core::FLOAT, 2, 4>;
	using mat4x3 = core::Matrix<core::FLOAT, 3, 4>;
	using mat4x4 = core::Matrix<core::FLOAT, 4, 4>;

	using mat2 = mat2x2;
	using mat3 = mat3x3;
	using mat4 = mat4x4;

	using dvec2 = core::Vec<core::DOUBLE, 2>;
	using dvec3 = core::Vec<core::DOUBLE, 3>;
	using dvec4 = core::Vec<core::DOUBLE, 4>;

	using ivec2 = core::Vec<core::INT, 2>;
	using ivec3 = core::Vec<core::INT, 3>;
	using ivec4 = core::Vec<core::INT, 4>;

	using uvec2 = core::Vec<core::UINT, 2>;
	using uvec3 = core::Vec<core::UINT, 3>;
	using uvec4 = core::Vec<core::UINT, 4>;

	using bvec2 = core::Vec<core::BOOL, 2>;
	using bvec3 = core::Vec<core::BOOL, 3>;
	using bvec4 = core::Vec<core::BOOL, 4>;

} //namespace csl
