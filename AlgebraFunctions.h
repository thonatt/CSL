#pragma once

#include "Algebra.h"

template<numberType type, unsigned int N, typename = std::enable_if_t<isFP<type>::value> >
Vec<type, 1> length(const Vec<type, N> & v) {
	release(v);
	return createDummy<Vec<type, 1>>("length(" + strFromObj(v) + ")", true);
}

template<numberType type, unsigned int Nrows, unsigned int Ncols, typename = std::enable_if_t< Nrows >= 2 && Ncols >= 2 > >
const Matrix<type, Ncols, Nrows> transpose(const Matrix<type, Nrows, Ncols> & m) {
	release(m);
	return createDummy<Matrix<type, Ncols, Nrows>>("transpose("+ strFromObj(m) + ")");
}

template<numberType type, unsigned int Nrows, unsigned int Ncols, typename = std::enable_if_t< Nrows == Ncols && Nrows >= 2 && isFP<type>::value > >
const Scalar<type> determinant(const Matrix<type, Nrows, Ncols> & m) { 
	release(m);
	return createDummy<Scalar<type>>("determinant(" + strFromObj(m) + ")");
}

//const Bool XOR(const Bool & b, const Bool & c) { return Bool(); }

//template<unsigned int N> Vec<numberType::BOOL, N> Not(const Vec<numberType::BOOL, N> & v) { return Vec<numberType::BOOL, N>(); }

#define GLSL_BVEC_BINARY_OPERATOR_NO_BOOL(op)														\
template<numberType type, unsigned int N, typename = std::enable_if_t< notBool<type> > >		\
const Vec<numberType::BOOL, N> op(const Vec<type, N> & v1, const Vec<type, N> & v2) {				\
	release(v1, v2);																				\
	return createDummy<Vec<numberType::BOOL, N>>(std::string(#op) + "(" + strFromObj(v1, v2) + ")");		\
}																									

GLSL_BVEC_BINARY_OPERATOR_NO_BOOL(lessThan);
GLSL_BVEC_BINARY_OPERATOR_NO_BOOL(lessThanEqual);
GLSL_BVEC_BINARY_OPERATOR_NO_BOOL(greaterThan);
GLSL_BVEC_BINARY_OPERATOR_NO_BOOL(greaterThanEqual);

#define GLSL_BVEC_BINARY_OPERATOR_WITH_BOOL(op)														\
template<numberType type, unsigned int N>															\
const Vec<numberType::BOOL, N> op(const Vec<type, N> & v1, const Vec<type, N> & v2) {				\
	release(v1, v2);																				\
	return createDummy<Vec<numberType::BOOL, N>>(std::string(#op) + "(" + strFromObj(v1, v2) + ")");		\
}																									

GLSL_BVEC_BINARY_OPERATOR_WITH_BOOL(equal);
GLSL_BVEC_BINARY_OPERATOR_WITH_BOOL(notEqual);

#define GLSL_BVEC_UNARY_OPERATOR(op)										\
template<unsigned int N>													\
const Bool op(const Vec<numberType::BOOL, N> & v1) {						\
	release(v1);															\
	return createDummy<Bool>(std::string(#op) + "(" + strFromObj(v1) + ")");		\
}																			

GLSL_BVEC_UNARY_OPERATOR(all);
GLSL_BVEC_UNARY_OPERATOR(any);

#define GLSL_VEC_UNARY_OPERATOR(op)																\
template<unsigned int N>																		\
const Vec<numberType::FLOAT,N> op(const Vec<numberType::FLOAT,N> & v) {							\
	release(v);																					\
	return createDummy<Vec<numberType::FLOAT,N>>(std::string(#op) + "(" + strFromObj(v) + ")");		\
}																								

GLSL_VEC_UNARY_OPERATOR(radians);
GLSL_VEC_UNARY_OPERATOR(degrees);

GLSL_VEC_UNARY_OPERATOR(sin);
GLSL_VEC_UNARY_OPERATOR(cos);
GLSL_VEC_UNARY_OPERATOR(tan);
GLSL_VEC_UNARY_OPERATOR(asin);
GLSL_VEC_UNARY_OPERATOR(acos);
GLSL_VEC_UNARY_OPERATOR(atan);

GLSL_VEC_UNARY_OPERATOR(sinh);
GLSL_VEC_UNARY_OPERATOR(cosh);
GLSL_VEC_UNARY_OPERATOR(tanh);
GLSL_VEC_UNARY_OPERATOR(asinh);
GLSL_VEC_UNARY_OPERATOR(acosh);
GLSL_VEC_UNARY_OPERATOR(atanh);

GLSL_VEC_UNARY_OPERATOR(exp);
GLSL_VEC_UNARY_OPERATOR(log);
GLSL_VEC_UNARY_OPERATOR(exp2);
GLSL_VEC_UNARY_OPERATOR(log2);

#define GLSL_VEC_BINARY_OPERATOR(op)												\
template<unsigned int N>															\
const VecF<N> op(const VecF<N> & v1, const VecF<N> & v2) {							\
	release(v1,v2);																	\
	return createDummy<VecF<N>>(std::string(#op) + "(" + strFromObj(v1,v2) + ")");		\
}		

GLSL_VEC_BINARY_OPERATOR(atan);
GLSL_VEC_BINARY_OPERATOR(pow);


#define GLSL_FP_VEC_UNARY_OPERATOR(op)															\
template<numberType type, unsigned int N, typename = std::enable_if_t<isFP<type>::value> >		\
const Vec<type,N> op(const Vec<type,N> & v) {													\
	release(v);																					\
	return createDummy<Vec<type,N>>(std::string(#op) + "(" + strFromObj(v) + ")");					\
}			

GLSL_FP_VEC_UNARY_OPERATOR(sqrt);
GLSL_FP_VEC_UNARY_OPERATOR(inversesqrt);
GLSL_FP_VEC_UNARY_OPERATOR(floor);
GLSL_FP_VEC_UNARY_OPERATOR(trunc);
GLSL_FP_VEC_UNARY_OPERATOR(round);
GLSL_FP_VEC_UNARY_OPERATOR(roundEven);
GLSL_FP_VEC_UNARY_OPERATOR(ceil);
GLSL_FP_VEC_UNARY_OPERATOR(fract);

GLSL_FP_VEC_UNARY_OPERATOR(normalize);

#define GLSL_FP_FP_VEC_BINARY_OPERATOR(op)														\
template<numberType type, unsigned int N, typename = std::enable_if_t<isFP<type>::value> >		\
const Scalar<type> op(const Vec<type,N> & v1, const Vec<type,N> & v2) {							\
	release(v1, v2);																			\
	return createDummy<Scalar<type>>(std::string(#op) + "(" + strFromObj(v1,v2) + ")");				\
}			

GLSL_FP_FP_VEC_BINARY_OPERATOR(dot);

#define GLSL_VEC_FP_VEC_BINARY_OPERATOR(op)														\
template<numberType type, unsigned int N,														\
typename = std::enable_if_t< N!=1 && isFP<type>::value> >										\
const Vec<type,N> op(const Vec<type,N> & v1, const Vec<type,N> & v2) {							\
	release(v1, v2);																			\
	return createDummy<Vec<type,N>>(std::string(#op) + "(" + strFromObj(v1,v2) + ")");				\
}			

GLSL_FP_FP_VEC_BINARY_OPERATOR(max);

#define GLSL_VEC_FP_VEC_SCALAR_BINARY_OPERATOR(op)												\
template<numberType type, unsigned int N, typename = std::enable_if_t<isFP<type>::value> >		\
const Vec<type,N> op(const Vec<type,N> & v1, const Scalar<type> & s) {							\
	release(v1, s);																				\
	return createDummy<Vec<type,N>>(std::string(#op) + "(" + strFromObj(v1,s) + ")");				\
}			

GLSL_VEC_FP_VEC_SCALAR_BINARY_OPERATOR(max);

#define GLSL_VEC_FP_VEC_RSCALAR_BINARY_OPERATOR(op)												\
template<numberType type, unsigned int N, typename = std::enable_if_t<isFP<type>::value> >		\
const Vec<type,N> op(const Vec<type,N> & v1, const double & s) {								\
	release(v1);																				\
	return createDummy<Vec<type,N>>(std::string(#op) + "(" + strFromObj(v1) + "," + std::to_string(s) + ")");	\
}			

GLSL_VEC_FP_VEC_RSCALAR_BINARY_OPERATOR(max);



#undef GLSL_BVEC_BINARY_OPERATOR_NO_BOOL
#undef GLSL_BVEC_BINARY_OPERATOR_WITH_BOOL
#undef GLSL_BVEC_UNARY_OPERATOR
#undef GLSL_VEC_UNARY_OPERATOR
#undef GLSL_VEC_BINARY_OPERATOR
#undef GLSL_FP_VEC_UNARY_OPERATOR
