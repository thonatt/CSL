#pragma once

#include <string>


template<unsigned char ... cs> struct SizeOfCharsT;
template<unsigned char ... cs> constexpr unsigned int SizeOfChars = SizeOfCharsT<cs...>::value;

template<> struct SizeOfCharsT<> {
	static const unsigned int value = 0;
};

template<unsigned char first, unsigned char  ... rest> struct SizeOfCharsT < first, rest... > {
	static const unsigned int value = 1 + SizeOfChars<rest...>;
};

template<unsigned int...> struct SumOf;
template<unsigned int... uints> constexpr unsigned int SumOfV = SumOf<uints...>::value;

template<> struct SumOf<> {
	static const unsigned int value = 0;
};

template<unsigned int First, unsigned int ... Rest> struct SumOf < First, Rest... > {
	static const unsigned int value = First + SumOfV<Rest...>;
};

template<unsigned int A, unsigned int B> struct MaxOf {
	static const unsigned int value = A > B ? A : B;
};
template<unsigned int... uints> constexpr unsigned int MaxOfV = MaxOf<uints...>::value;


template<unsigned int index, unsigned char... cs> struct ExctactChar;

template<unsigned char c, unsigned char ... rest> struct ExctactChar<0, c, rest...> {
	static const unsigned char value = c;
};

template<unsigned int index, unsigned char c, unsigned char ... rest> struct ExctactChar<index, c, rest...> {
	static_assert(index >= 0 && index <= SizeOfChars<rest...>, "wrong index");
	static const unsigned char value = ExctactChar<index - 1, rest...>::value;
};


enum numberType { BOOL, UINT, INT, FLOAT, DOUBLE, INVALID};

template<numberType type, unsigned int Nrows, unsigned int Ncols>
class MatrixT;

template<numberType type, unsigned int N> using VecT = MatrixT<type, N, 1>;

template<numberType type> using ScalarT = VecT<type, 1>;

using DoubleT = ScalarT<numberType::DOUBLE>;
using FloatT = ScalarT<numberType::FLOAT>;
using BoolT = ScalarT<numberType::BOOL>;
using UintT = ScalarT<numberType::UINT>;
using IntT = ScalarT<numberType::INT>;

using vec2T = VecT<numberType::FLOAT, 2>;
using vec3T = VecT<numberType::FLOAT, 3>;
using vec4T = VecT<numberType::FLOAT, 4>;

using mat2x2T = MatrixT<numberType::FLOAT, 2, 2>;
using mat2x3T = MatrixT<numberType::FLOAT, 2, 3>;
using mat2x4T = MatrixT<numberType::FLOAT, 2, 4>;
using mat3x2T = MatrixT<numberType::FLOAT, 2, 2>;
using mat3x3T = MatrixT<numberType::FLOAT, 3, 3>;
using mat3x4T = MatrixT<numberType::FLOAT, 3, 4>;
using mat4x2T = MatrixT<numberType::FLOAT, 4, 2>;
using mat4x3T = MatrixT<numberType::FLOAT, 4, 3>;
using mat4x4T = MatrixT<numberType::FLOAT, 4, 4>;

using mat2T = mat2x2T;
using mat3T = mat3x3T;
using mat4T = mat4x4T;


template<numberType type> struct isFP {
	static const bool value = false;
};

template<> struct isFP<numberType::FLOAT> {
	static const bool value = true;
};

template<> struct isFP<numberType::DOUBLE> {
	static const bool value = true;
};

template<numberType type> struct isInt {
	static const bool value = false;
};

template<> struct isInt<numberType::INT> {
	static const bool value = true;
};

template<> struct isInt<numberType::UINT> {
	static const bool value = true;
};

template<numberType type> struct isBool {
	static const bool value = false;
};

template<> struct isBool<numberType::BOOL> {
	static const bool value = true;
};

template<numberType type> struct notBoolT {
	static const bool value = isInt<type>::value || isFP<type>::value;
};

template<numberType type> constexpr bool notBool = notBoolT<type>::value;

template<bool A, bool B> constexpr bool CT_XOR = (A && B) || (!A && !B);

//template<numberType tA, numberType tB> struct HigherType {
//	static const numberType type = tA;
//};
//
//template<numberType tB> struct HigherType<numberType::DOUBLE