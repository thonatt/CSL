#pragma once

// cpp helpers
using uint = unsigned int;

template<uint A, uint B>
constexpr uint MaxUint = A > B ? A : B;

template<int First, int Second> 
constexpr int Last = (Second >= 0 ? Second : First);

template<typename T>
using CleanType = std::remove_const_t<std::remove_reference_t<T>>;

// matrix types forward declarations

enum ScalarType { BOOL, INT, UINT, FLOAT, DOUBLE, VOID, INVALID };
enum AssignType { ASSIGNABLE, NON_ASSIGNABLE };

template<ScalarType type, uint Nrows, uint Ncols, AssignType a = ASSIGNABLE>
class Matrix;

template<ScalarType type, uint N, AssignType assignable = ASSIGNABLE>
using Vec = Matrix<type, N, 1, assignable>;

template<ScalarType type>
using Scalar = Vec<type, 1>;

using Double = Scalar<DOUBLE>;
using Float = Scalar<FLOAT>;
using Bool = Scalar<BOOL>;
using Uint = Scalar<UINT>;
using Int = Scalar<INT>;

using vec2 = Vec<FLOAT, 2>;
using vec3 = Vec<FLOAT, 3>;
using vec4 = Vec<FLOAT, 4>;

using mat2x2 = Matrix<FLOAT, 2, 2>;
using mat2x3 = Matrix<FLOAT, 2, 3>;
using mat2x4 = Matrix<FLOAT, 2, 4>;
using mat3x2 = Matrix<FLOAT, 3, 2>;
using mat3x3 = Matrix<FLOAT, 3, 3>;
using mat3x4 = Matrix<FLOAT, 3, 4>;
using mat4x2 = Matrix<FLOAT, 4, 2>;
using mat4x3 = Matrix<FLOAT, 4, 3>;
using mat4x4 = Matrix<FLOAT, 4, 4>;

using mat2 = mat2x2;
using mat3 = mat3x3;
using mat4 = mat4x4;

using dvec2 = Vec<DOUBLE, 2>;
using dvec3 = Vec<DOUBLE, 3>;
using dvec4 = Vec<DOUBLE, 4>;

// samplers types forward decalrations

enum AccessType { SAMPLER, IMAGE };
enum SamplerType { BASIC, CUBE, RECTANGLE, MULTI_SAMPLE, BUFFER, ATOMIC };
enum SamplerIsArray { NOT_ARRAY, ARRAY };
enum SamplerIsShadow { NOT_SHADOW, SHADOW };

template<
	AccessType aType,
	ScalarType nType,
	uint N,
	SamplerType sType = BASIC,
	SamplerIsArray isArray = NOT_ARRAY,
	SamplerIsShadow isShadow = NOT_SHADOW
>
class Sampler;

// layout types forward declarations

enum LayoutArgBoolType { SHARED, PACKED, STD140, STD430 };
enum LayoutArgIntType { OFFSET, BINDING, LOCATION };
enum QualifierType { IN, OUT, UNIFORM };

template<LayoutArgBoolType layoutArg, bool b> struct LayoutArgBool;
template<LayoutArgIntType layoutArg, int N> struct LayoutArgInt;

template<typename ... LayoutCleanedArgs> struct LayoutCleanedArg;

template<typename ... LayoutArgs> struct Layout;

template<QualifierType qType, typename T, typename L>
struct Qualifier;

// types infos

template<typename T> struct Infos {
	static const bool is_numeric_type = false;
	static const bool is_glsl_type = false;
	static const uint rows = 0;
	static const uint cols = 0;
	static const ScalarType scalar_type = INVALID;
};

template<ScalarType type, uint Nrows, uint Ncols, AssignType assignable>
struct Infos<Matrix<type, Nrows, Ncols, assignable>> {
	static const bool is_numeric_type = type != BOOL;
	static const bool is_glsl_type = true;
	static const uint rows = Nrows;
	static const uint cols = Ncols;
	static const ScalarType scalar_type = type;
};

template<> struct Infos<double> {
	static const bool is_numeric_type = true;
	static const bool is_glsl_type = false;
	static const uint rows = 1;
	static const uint cols = 1;
	static const ScalarType scalar_type = FLOAT;
};

template<> struct Infos<int> {
	static const bool is_numeric_type = true;
	static const bool is_glsl_type = false;
	static const uint rows = 1;
	static const uint cols = 1;
	static const ScalarType scalar_type = INT;
};

template<> struct Infos<bool> {
	static const bool is_numeric_type = true;
	static const bool is_glsl_type = false;
	static const uint rows = 1;
	static const uint cols = 1;
	static const ScalarType scalar_type = BOOL;
};

template<> struct Infos<void> {
	static const bool is_numeric_type = false;
	static const bool is_glsl_type = false;
	static const uint rows = 0;
	static const uint cols = 0;
	static const ScalarType scalar_type = VOID;

};

template<QualifierType qType, typename T, typename L>
struct Infos<Qualifier<qType, T, L> > {
	static const bool is_numeric_type = Infos<T>::is_numeric_type;
	static const bool is_glsl_type = Infos<T>::is_glsl_type;
	static const uint rows = Infos<T>::rows;
	static const uint cols = Infos<T>::cols;
	static const ScalarType scalar_type = Infos<T>::scalar_type;
};

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
constexpr bool EqualType = Infos<A>::scalar_type == Infos<B>::scalar_type;

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
constexpr bool ValidForMatMultiplication = Infos<A>::cols == Infos<B>::rows && !(IsScalar<A> && IsScalar<B> );

template<typename ...Ts> struct MatElementsT;
template<typename ...Ts> constexpr uint MatElements = MatElementsT<Ts...>::value;

template<> struct MatElementsT<> {
	static const uint value = 0;
};

template<typename T, typename ...Ts> struct MatElementsT<T, Ts...> {
	static const uint value = NumElements<T> + MatElements<Ts...>;
};

// other helpers
template<typename A> 
constexpr bool IsVecF = Infos<A>::cols == 1 && Infos<A>::scalar_type == FLOAT;


template<typename ... A>
constexpr bool IsFloat = false;

template<typename A>
constexpr bool IsFloat<A> = IsVecF<A> && Infos<A>::rows == 1;


template<typename A, typename B>
constexpr bool SuperiorType = Infos<B>::scalar_type >= Infos<A>::scalar_type;

template<typename A, typename B>
constexpr bool EqualDim = (Infos<A>::rows == Infos<B>::rows) && (Infos<A>::cols == Infos<B>::cols);

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
	static const bool value = EqualType<T,U> && SameScalarType<U, Ts...>;
};

template<SamplerType input, SamplerType ... sources>
struct IsAnySamplerType {
	static const bool value = AnyTrue<(input == sources)...>;
};