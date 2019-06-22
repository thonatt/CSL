#pragma once

#include "ExpressionsTest.h"
#include "Swizzles.h"

/// matrix class

template<ScalarType type, uint NR, uint NC>
class Matrix : public NamedObject<Matrix<type, NR, NC>> {

public:
	using UnderlyingType = Matrix;

protected:
	static const bool isBool = (type == BOOL);
	static const bool isScalar = (NC == 1 && NR == 1);

public:

	static const std::string typeStr() { return TypeStr<Matrix>::str(); }

	explicit Matrix(const std::string & _name = "", uint flags = IS_TRACKED)
		: NamedObject<Matrix>(_name, flags)
	{	
	}

	template <std::size_t N>
	explicit Matrix(const char(&s)[N], uint flags = IS_TRACKED)
		: NamedObject<Matrix>(s)
	{
	}

	Matrix(const Ex & _ex, uint ctor_flags = 0, uint obj_flags = IS_TRACKED, const std::string & s = "")
		: NamedObject<Matrix>(_ex, ctor_flags, obj_flags, s)
	{
	}

	// constructor from cpp types (bool, int, and double)
	template<typename U, typename = std::enable_if_t<IsValid<U> && !Infos<U>::is_glsl_type && isScalar && !isBool > >
	Matrix(U && u, const std::string & s)
		: NamedObject<Matrix>(
			EqualMat<U,Matrix> ? 0 : ( DISPLAY_TYPE | PARENTHESIS),
			IS_TRACKED , s,
			EX(U,u) )
	{
	}

	Matrix(const NamedObjectInit<Matrix> & obj) : NamedObject<Matrix>(obj) {}

	Matrix(const Matrix & other) : NamedObject<Matrix>(other) {}

	Matrix(Matrix && other) : NamedObject<Matrix>(other) {}

	//glsl constructors

	// matXY(a,b,...)
	template<typename R_U, typename R_V, typename ...R_Us,
		typename = std::enable_if_t < AreValid<CT<R_U>, CT<R_V>, CT<R_Us>... > && MatElements<CT<R_U>, CT<R_V>, CT<R_Us>... > == NR * NC > >
		explicit Matrix(R_U && u, R_V && v, R_Us && ...us)
		: NamedObject<Matrix>(
			PARENTHESIS | DISPLAY_TYPE, IS_TRACKED, "",
			EX(R_U, u), EX(R_V, v), EX(R_Us, us)...)
	{
	}


	// matXY(int/float) and matXY(matWZ)
	template<typename R_T, typename T = CleanType<R_T>, typename = std::enable_if_t< 
		AreValid<T> && (
			isScalar || IsScalar<T>  || //matXY(int/float)
		(!isBool && (NC != 1 || NR != 1) &&  (Infos<T>::cols != 1 || Infos<T>::rows != 1) ) //matXY(matWZ)
			) > >
	Matrix( R_T && x)
		: NamedObject<Matrix>(
			EqualMat<Matrix, T> ? 0 : (DISPLAY_TYPE | PARENTHESIS),
			IS_TRACKED, "",
			EX(R_T, x))
	{
	}

	// operators =

	Matrix operator=(Matrix && other) & {
		return { createExp<MiddleOperator<ASSIGNMENT>>(" = ", NamedObjectBase::getExRef(), other.getExTmp() ) };
	}

	Matrix operator=(Matrix && other) && {
		return { createExp<MiddleOperator<ASSIGNMENT>>(" = ", NamedObjectBase::getExTmp(), other.getExTmp() ) };
	}

	Matrix operator=(const Matrix & other) & {
		return { createExp<MiddleOperator<ASSIGNMENT>>(" = ", NamedObjectBase::getExRef(), other.getExRef() ) };
	}

	Matrix operator=(const Matrix & other) && {
		return { createExp<MiddleOperator<ASSIGNMENT>>(" = ", NamedObjectBase::getExTmp(), other.getExRef() ) };
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// swizzles accessors

	template<uint Dim, SwizzleSet Set, uint Bytes, uint Size,
		typename = std::enable_if_t<NC == 1 && NR != 1 && Set != MIXED_SET && Dim <= NR && Size <= NR > >
		Vec<type, Size> operator[](const SwizzlePack<Set, Dim, Bytes, Size, REPEATED> & swizzle) const &
	{
		return { createExp<MemberAccessor>(NamedObjectBase::getExRef(), swizzle.getStrPtr()) };
	}

	template<uint Dim, SwizzleSet Set, uint Bytes, uint Size,
		typename = std::enable_if_t<NC == 1 && NR != 1 && Set != MIXED_SET && Dim <= NR && Size <= NR > >
		Vec<type, Size> operator[](const SwizzlePack<Set, Dim, Bytes, Size, NON_REPEATED> & swizzle) const &
	{
		return { createExp<MemberAccessor>(NamedObjectBase::getExRef(), swizzle.getStrPtr()) };
	}

	template<uint Dim, SwizzleSet Set, uint Bytes, uint Size,
		typename = std::enable_if_t<NC == 1 && NR != 1 && Set != MIXED_SET && Dim <= NR && Size <= NR > >
		Vec<type, Size> operator[](const SwizzlePack<Set, Dim, Bytes, Size, REPEATED> & swizzle) const &&
	{
		return { createExp<MemberAccessor>(NamedObjectBase::getExTmp(), swizzle.getStrPtr()) };
	}

	template<uint Dim, SwizzleSet Set, uint Bytes, uint Size,
		typename = std::enable_if_t<NC == 1 && NR != 1 && Set != MIXED_SET && Dim <= NR && Size <= NR > >
		Vec<type, Size> operator[](const SwizzlePack<Set, Dim, Bytes, Size, NON_REPEATED> & swizzle) const &&
	{
		return { createExp<MemberAccessor>(NamedObjectBase::getExTmp(), swizzle.getStrPtr()) };
	}

	// array subscript accessor
	template<typename U, typename = std::enable_if_t<!isScalar && IsInteger<CT<U>> > >
	typename std::conditional_t< NC == 1, Scalar<type>, Vec<type, NR> > operator[](U && u) &
	{
		return { createExp<ArraySubscript>(NamedObjectBase::getExRef(), EX(U,u) ) };
	}

	// array subscript accessor tmp
	template<typename U, typename = std::enable_if_t<!isScalar && IsInteger<U> > >
	typename std::conditional_t< NC == 1, Scalar<type>, Vec<type, NR> > operator[](U && u) &&
	{
		return { createExp<ArraySubscript>(NamedObjectBase::getExTmp(), EX(U,u) ) };
	}


	/////////////////////////////////////////////////////////////////////////////
	// unary operators

	template<bool b = !isBool, typename = std::enable_if_t<b> >
	Matrix operator++() & {
		return { createExp<PrefixUnary>("++", NamedObjectBase::getExRef()) };
	}
	template<bool b = !isBool, typename = std::enable_if_t<b> >
	Matrix operator++() && {
		return { createExp<PrefixUnary>("++", NamedObjectBase::getExTmp()) };
	}

	template<bool b = !isBool, typename = std::enable_if_t<b> >
	Matrix operator++(int) & {
		return { createExp<PostfixUnary>("++ ", NamedObjectBase::getExRef()) };
	}
	template<bool b = !isBool, typename = std::enable_if_t<b> >
	Matrix operator++(int) && {
		return { createExp<PostfixUnary>("++ ", NamedObjectBase::getExTmp()) };
	}

	template<bool b = !isBool, typename = std::enable_if_t<b> >
	Matrix operator--() & {
		return { createExp<PrefixUnary>("--", NamedObjectBase::getExRef()) };
	}
	template<bool b = !isBool, typename = std::enable_if_t<b> >
	Matrix operator--() && {
		return { createExp<PrefixUnary>("--", NamedObjectBase::getExTmp()) };
	}

	template<bool b = !isBool, typename = std::enable_if_t<b> >
	Matrix operator--(int) & {
		return { createExp<PostfixUnary>("-- ", NamedObjectBase::getExRef()) };
	}
	template<bool b = !isBool, typename = std::enable_if_t<b> >
	Matrix operator--(int) && {
		return { createExp<PostfixUnary>("-- ", NamedObjectBase::getExTmp()) };
	}

	template<bool b = !isBool, typename = std::enable_if_t<b> >
	Matrix operator-() & {
		return { createExp<PrefixUnary>("-", NamedObjectBase::getExRef()) };
	}
	template<bool b = !isBool, typename = std::enable_if_t<b> >
	Matrix operator-() && {
		return { createExp<PrefixUnary>("-", NamedObjectBase::getExTmp()) };
	}

	template<bool b = isBool, typename = std::enable_if_t<b> >
	Matrix operator!() & {
		return { createExp<PrefixUnary>("!", NamedObjectBase::getExRef()) };
	}
	template<bool b = isBool, typename = std::enable_if_t<b> >
	Matrix operator!() && {
		return { createExp<PrefixUnary>("!", NamedObjectBase::getExTmp()) };
	}

	////////////////////////////////////////////////////////////////////////////////
	////operators X=

	template<typename A,
		typename = std::enable_if_t<NotBool<A> && (EqualMat<Matrix, A> || IsScalar<A>)  >  >
		void operator+=(A && a) & {
		Matrix(createExp<MiddleOperator<ASSIGNMENT>>(" += ", NamedObjectBase::getExRef(), EX(A, a)));
	}

	template<typename A,
		typename = std::enable_if_t<NotBool<A> && (EqualMat<Matrix, A> || IsScalar<A>)  >  >
		void operator+=(A && a) && {
		Matrix(createExp<MiddleOperator<ASSIGNMENT>>(" += ", NamedObjectBase::getExTmp(), EX(A, a)));
	}

	template<typename A,
		typename = std::enable_if_t<NotBool<A> && (EqualMat<Matrix, A> || IsScalar<A>)  >  >
		void operator-=(A && a)  & {
		Matrix(createExp<MiddleOperator<ASSIGNMENT>>(" -= ", NamedObjectBase::getExRef(), EX(A, a)));
	}

	template<typename A,
		typename = std::enable_if_t<NotBool<A> && (EqualMat<Matrix, A> || IsScalar<A>)  >  >
		void operator-=(A && a) && {
		Matrix(createExp<MiddleOperator<ASSIGNMENT>>(" -= ", NamedObjectBase::getExTmp(), EX(A, a)));
	}

	template<typename A,
		typename = std::enable_if_t<NotBool<A> && (EqualMat<Matrix, A> || IsScalar<A>)  >  >
		void operator*=(A && a)  & {
		Matrix(createExp<MiddleOperator<ASSIGNMENT>>(" *= ", NamedObjectBase::getExRef(), EX(A, a)));
	}

	template<typename A,
		typename = std::enable_if_t<NotBool<A> && (EqualMat<Matrix, A> || IsScalar<A>)  >  >
		void operator*=(A && a) && {
		Matrix(createExp<MiddleOperator<ASSIGNMENT>>(" *= ", NamedObjectBase::getExTmp(), EX(A, a)));
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////
	// member functions

	template<bool b = !isScalar, typename = std::enable_if_t<b> >
	Int length() & {
		return { createExp<MemberFunctionAccessor<0>>(NamedObjectBase::getExRef(), "length") };
	}
	
	Int length() && = delete; 

	//template<typename R_A, typename A = CleanType<R_A>,
	//	typename = std::enable_if_t< NotBool<A> && (EqualMat<Matrix, A> || IsScalar<A>)  >  >
	//	void operator*=(R_A&& a) const && = delete;

	// needed for loops
	template<bool b = isBool && NC == 1 && NR == 1, typename = std::enable_if_t<b> >
	operator bool() & {
		//std::cout << " bool const & " << NamedObjectBase::getExRef()->str() << std::endl;
		//needed as any [variable;] in GL_FOR wont generate any instruction
		listen().stack_for_condition(NamedObjectBase::getExRef());
		return false;
	}

	template<bool b = isBool && NC == 1 && NR == 1, typename = std::enable_if_t<b> >
	operator bool() && {
		//std::cout << " bool const && " << NamedObjectBase::getExTmp()->str() << std::endl;
		return false;
	}

	operator int() const {
		static_assert( IsInteger<Matrix> , "must switch over an Int or Uint");
		return 0;
	}
};

// Bool operators
template<typename A, typename B,
	typename = std::enable_if_t< EqualMat<A, Bool> &&  EqualMat<B, Bool> > >
	Bool operator&&(A && a, B && b)
{
	return { createExp<MiddleOperator<LOGICAL_AND>>(" && ", EX(A,a), EX(B,b)) };
}

template<typename A, typename B,
	typename = std::enable_if_t< EqualMat<A, Bool> &&  EqualMat<B, Bool> > >
	Bool operator||(A && a, B && b)
{
	return { createExp<MiddleOperator<LOGICAL_OR>>(" || ", EX(A,a), EX(B,b)) };
}

template<typename A, typename B,
	typename = std::enable_if_t< NoBools<A, B> && EqualDim<A, B> > >
	Bool operator==(A && a, B && b)
{
	return { createExp<MiddleOperator<EQUALITY>>(" == ", EX(A, a), EX(B, b)) };
}

// > and < operators
template<typename A, typename B,
	typename = std::enable_if_t< NoBools<A, B> && IsScalar<A> && IsScalar<B> > >
	Bool operator>(A && a, B && b)
{
	return { createExp<MiddleOperator<RELATIONAL>>(" > ", EX(A, a), EX(B, b)) };
}

template<typename A, typename B,
	typename = std::enable_if_t< NoBools<A, B> && IsScalar<A> && IsScalar<B> > >
	Bool operator<(A && a, B && b)
{
	return { createExp<MiddleOperator<RELATIONAL>>(" < ", EX(A, a), EX(B, b)) };
}

template<typename A, typename B,
	typename = std::enable_if_t< NoBools<A, B> && IsScalar<A> && IsScalar<B> > >
	Bool operator<=(A && a, B && b)
{
	return { createExp<MiddleOperator<RELATIONAL>>(" <= ", EX(A, a), EX(B, b)) };
}

template<typename A, typename B,
	typename = std::enable_if_t< NoBools<A, B> && IsScalar<A> && IsScalar<B> > >
	Bool operator>=(A && a, B && b)
{
	return { createExp<MiddleOperator<RELATIONAL>>(" >= ", EX(A, a), EX(B, b)) };
}

// + and - operators
template<typename A, typename B, 
	typename = std::enable_if_t< NoBools<A, B> && (EqualMat<A, B> || ( IsScalar<A> || IsScalar<B> ) ) > >
	ArithmeticBinaryReturnType<A, B> operator+(A && a, B && b)
{
	return { createExp<MiddleOperator<ADDITION>>(" + ", EX(A, a), EX(B, b)) };
}

template<typename A, typename B, 
	typename = std::enable_if_t< NoBools<A, B> && (EqualMat<A, B> || (IsScalar<A> || IsScalar<B>)) > >
	ArithmeticBinaryReturnType<A, B> operator-(A && a, B && b)
{
	return { createExp<MiddleOperator<SUBSTRACTION>>(" - ", EX(A, a), EX(B, b)) };
}

// * operators
// matrix multiplication
template<typename A,typename B,
	typename = std::enable_if_t< NoBools<A, B> && ValidForMatMultiplication<A, B> > >
	MultiplicationReturnType<A, B> operator*(A && a, B && b)
{
	return { createExp<MiddleOperator<MULTIPLY>>("*", EX(A, a), EX(B, b)) };
}

// cwise multiplication
template<typename A, typename B,
	typename = std::enable_if_t< NoBools<A, B> && ( EqualDim<A,B> || IsScalar<A> || IsScalar<B>) > >
	ArithmeticBinaryReturnType<A, B> operator*(A && a, B && b)
{
	return { createExp<MiddleOperator<MULTIPLY>>("*", EX(A, a), EX(B, b)) };
}

// ()/() operator
template<typename A, typename B,
	typename = std::enable_if_t< NoBools<A, B> && (EqualMat<A, B> || (IsScalar<A> || IsScalar<B>)) > >
	ArithmeticBinaryReturnType<A, B> operator/(A && a, B && b)
{
	return { createExp<MiddleOperator<DIVISION>>("/", EX(A, a), EX(B, b)) };
}

////////////////////////////////////
// Arrrays
// N == 0 for unspecified size arrays

template<typename T, uint N>
struct Array : NamedObject<Array<T, N>> {

	template<bool b = (N != 0), typename = std::enable_if_t<b> >
	Array(const std::string & _name = "", uint flags = IS_TRACKED)
		: NamedObject<Array<T,N>>(_name, flags)
	{
	}

	Array(const Ex & _ex, uint ctor_flags = 0, uint obj_flags = IS_TRACKED, const std::string & s = "")
		: NamedObject<Array>(_ex, ctor_flags, obj_flags, s)
	{
	}

	template<typename ... Us, typename = std::enable_if_t<
		sizeof...(Us) != 0 && AllTrue<EqualMat<Us, T>...> && (N == 0 || sizeof...(Us) == N) 
	> >
		Array(const std::string & name, Us && ... us)
		: NamedObject<Array<T, N>>(DISPLAY_TYPE | PARENTHESIS, IS_TRACKED | IS_USED, name, EX(Us, us)...)
	{
	}

	template<typename ... Us,
		typename = std::enable_if_t< AllTrue<EqualMat<Us, T>...> && (N == 0 || sizeof...(Us) == N) > >
		Array(Us && ... us)
		: NamedObject<Array<T, N>>(DISPLAY_TYPE | PARENTHESIS, IS_TRACKED | IS_USED, "", EX(Us, us)...)
	{
	}
	
	Array(const NamedObjectInit<Array> & obj) : NamedObject<Array>(obj) 
	{
	}

	template<typename A,  
	typename = std::enable_if_t< IsInteger<A> > >
		typename T::UnderlyingType operator[](A && a) const & {
		return { createExp<ArraySubscript>(NamedObjectBase::getExRef(), EX(A,a)) };
	}

	template<typename A,
		typename = std::enable_if_t< IsInteger<A> > >
		typename T::UnderlyingType operator[](A && a) const && {
		return { createExp<ArraySubscript>(NamedObjectBase::getExTmp(), EX(A,a)) };
	}
};
