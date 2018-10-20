#pragma once

// cpp helpers
using uint = unsigned int;

template<uint A, uint B>
constexpr uint MaxUint = A > B ? A : B;

template<typename T>
using CleanType = std::remove_const_t<std::remove_reference_t<T>>;

// matrix types forward declarations

enum ScalarType { BOOL, UINT, INT, FLOAT, DOUBLE, INVALID };

template<ScalarType type, uint Nrows, uint Ncols>
class Matrix;

template<ScalarType type, uint N> using Vec = Matrix<type, N, 1>;

template<ScalarType type> using Scalar = Vec<type, 1>;

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
using mat3x2 = Matrix<FLOAT, 2, 2>;
using mat3x3 = Matrix<FLOAT, 3, 3>;
using mat3x4 = Matrix<FLOAT, 3, 4>;
using mat4x2 = Matrix<FLOAT, 4, 2>;
using mat4x3 = Matrix<FLOAT, 4, 3>;
using mat4x4 = Matrix<FLOAT, 4, 4>;

using mat2 = mat2x2;
using mat3 = mat3x3;
using mat4 = mat4x4;

// types infos

template<typename T> struct Infos {
	static const bool is_numeric_type = false;
	static const bool is_glsl_type = false;
	static const uint rows = 0;
	static const uint cols = 0;
	static const ScalarType scalar_type = INVALID;
};

template<ScalarType type, uint Nrows, uint Ncols>
struct Infos<Matrix<type, Nrows, Ncols>> {
	static const bool is_numeric_type = true;
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

template<typename ...Ts> struct MatElementsT;
template<typename ...Ts> constexpr uint MatElements = MatElementsT<Ts...>::value;

template<> struct MatElementsT<> {
	static const uint value = 0;
};

template<typename T, typename ...Ts> struct MatElementsT<T, Ts...> {
	static const uint value = NumElements<T> + MatElements<Ts...>;
};

// other helpers

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

// variadic helpers

template<typename ...Ts> struct AreValidT;
template<typename ...Ts> constexpr bool AreValid = AreValidT<Ts...>::value;

template<> struct AreValidT<> {
	static const bool value = true;
};

template<typename T, typename ...Ts> struct AreValidT<T, Ts...> {
	static const bool value = IsValid<T> && AreValidT<Ts...>::value;
};

template<typename ...Ts> struct SameScalarTypeT;
template<typename ...Ts> constexpr bool SameScalarType = SameScalarTypeT<Ts...>::value;

template<typename T> struct SameScalarTypeT<T> {
	static const bool value = true;
};

template<typename T, typename U, typename ...Ts> struct SameScalarTypeT<T, U, Ts...> {
	static const bool value = EqualType<T,U> && SameScalarType<U, Ts...>;
};
