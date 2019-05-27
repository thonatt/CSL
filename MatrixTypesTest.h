#pragma once

#include "ExpressionsTest.h"
#include "Swizzles.h"

//template<ScalarType type, uint Nrows, uint Ncols>
//struct MatrixInit {
//	MatrixInit(const Ex & ex, const std::string & s);
//
//	std::string name;
//	Ex exp;
//};

/// matrix class

template<ScalarType type, uint NR, uint NC, AssignType assignable>
class Matrix : public NamedObject<Matrix<type, NR, NC, assignable>> {
public:
	//using NamedObjectBase::exp;
	//using NamedObjectBase::namePtr;
	//using NamedObjectBase::isUsed;
	template<AssignType other_assignable> using OtherMat = Matrix<type, NR, NC, other_assignable>;

	using UnderlyingType = Matrix;

protected:
	static const bool isBool = (type == BOOL);
	static const bool isScalar = (NC == 1 && NR == 1);
	static const bool isAssignable = (assignable == ASSIGNABLE);
public:

	static const std::string typeStr() { return TypeStr<Matrix>::str(); }

	// special constructor for function args as tuples
	//explicit Matrix(const std::string & _name = "", ) : NamedObject<Matrix>(_name,_tracked) {
	//	exp = createDeclaration<Matrix>(NamedObjectBase::myNamePtr());
	//	if (_tracked == NOT_TRACKED) {
	//		areNotInit(*this);
	//	}
	//}

	//// constructors for declarations
	//explicit Matrix(const std::string & _name = "") : NamedObject<Matrix>(_name) {
	//	exp = createDeclaration<Matrix>(NamedObjectBase::myNamePtr());
	//}

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
			EX(U,u)
			)
	{
		//exp = createInit<Matrix, HIDE, IN_FRONT, NO_PARENTHESIS>(namePtr, getExp<U>(u));
		//exp = createInit<Matrix, HIDE, NO_PARENTHESIS>(namePtr, getExp<U>(u));
	}

	Matrix(const NamedObjectInit<Matrix> & obj) : NamedObject<Matrix>(obj) {}


	//glsl constructors

	// matXY(int/float) and matXY(matWZ)
	template<typename R_T, typename T = CleanType<R_T>, typename = std::enable_if_t< 
		AreValid<T> && (
			isScalar || IsScalar<T>  || //matXY(int/float)
		(!isBool && NC != 1 && NR != 1 && Infos<T>::cols != 1 && Infos<T>::rows != 1) //matXY(matWZ)
			) > >
	Matrix( R_T && x)
		: NamedObject<Matrix>(
			EqualMat<Matrix, T> ? 0 : (DISPLAY_TYPE | PARENTHESIS),
			IS_TRACKED,
			"",
			EX(R_T, x)			
			)
	{
		
		////checkForTemp<R_T>(x);
		//if ( EqualMat<Matrix,T> ) {
		//	std::cout << "conversion " << getTypeStr<T>() << " -> " << getTypeStr<Matrix>() << std::endl <<
		//		" : " << name() << " " << getExp<R_T>(x)->str() << std::endl;
		//	//exp = createInit<Matrix, HIDE, IN_FRONT, NO_PARENTHESIS>(namePtr, getExp<R_T>(x));
		//	//exp = createInit<Matrix,HIDE, NO_PARENTHESIS>(namePtr, getExp<R_T>(x));
		//	exp = getExp<R_T>(x);
		//} else {
		//	//exp = createInit<Matrix, DISPLAY, IN_FRONT, USE_PARENTHESIS>(namePtr, getExp<R_T>(x));
		//	exp = createInit<Matrix>(namePtr, getExp<R_T>(x));
		//}
	}

	/////////////////////////////////////////////////////
	// Used for tuples, need to figure out why
	/////////////////////////////////////////////////////

	Matrix(Matrix && other) : NamedObject<Matrix>(other) {
		//parent = other.parent;
		//exp = other.exp;
		//std::static_pointer_cast<CtorBase>(exp->op)->firstStr = false;
		//std::cout << " CTOR &&" << std::endl;
		
	}

	Matrix (const Matrix & other) : NamedObject<Matrix>(other) {
		//parent = other.parent;
		//exp = other.exp;
		//std::static_pointer_cast<CtorBase>(exp->op)->firstStr = false;
		//std::cout << " CTOR const&" << std::endl;
	}

	//Matrix & operator=(Matrix && other) & {
	//	std::cout << "getE(other) " << getE(other)->str() << std::endl;
	//	listen().addEvent(
	//		createExp<MiddleOperator<ASSIGNMENT>>(
	//			" = ",
	//			getEx(),
	//			EX(Matrix, other)
	//		)
	//	);
	//	return *this;
	//}

	//Matrix & operator=(const Matrix & other) & {
	//	listen().addEvent(
	//		createExp<MiddleOperator<ASSIGNMENT>>(
	//			" = ",
	//			getEx(),
	//			getE(other)
	//		)
	//	);
	//	return *this;
	//}

	//Matrix & operator=(Matrix && other) & {
	//	listen().addEvent(
	//		createExp<MiddleOperator<ASSIGNMENT>>(
	//			" = ",
	//			getExp<Matrix, false>(*this),
	//			getExp<Matrix, true>(other)
	//			)
	//	);
	//	return *this;
	//}

	/////////////////////////////////////////////////////

	template<AssignType other_assignable, bool temp, bool otherTemp>
	void assign_base(const OtherMat<other_assignable>& other) {
		listen().addEvent(
			createExp<MiddleOperator<ASSIGNMENT>>(
				" = ",
				getExp<Matrix, temp>(*this),
				getExp<OtherMat<other_assignable>, otherTemp>(other)
				)
		);
	}

	template<AssignType other_assignable, bool valid = isAssignable, typename = std::enable_if_t<valid>  >
	void operator=(const OtherMat<other_assignable>& other) & {
		assign_base<other_assignable, false, false>(other);
	}

	template<AssignType other_assignable, bool valid = isAssignable, typename = std::enable_if_t<valid> >
	void operator=(OtherMat<other_assignable>&& other) & {
		assign_base<other_assignable, false, true>(other);
	}

	template<AssignType other_assignable, bool valid = isAssignable, typename = std::enable_if_t<valid> >
	void operator=(const OtherMat<other_assignable>& other) && {
		assign_base<other_assignable, true, false>(other);
	}

	template<AssignType other_assignable, bool valid = isAssignable, typename = std::enable_if_t<valid>  >
	void operator=(OtherMat<other_assignable>&& other) && {
		assign_base<other_assignable, true, true>(other);
	}
	
	template<typename U, typename = std::enable_if_t < 
		!EqualDim<Matrix,U> ||
		(Infos<U>::scalar_type > type)
	> >
	Matrix & operator=(const  U & u) = delete;

	template<typename R_U, typename R_V, typename ...R_Us,
		typename = std::enable_if_t < AreValid<CT<R_U>, CT<R_V>, CT<R_Us>... > && MatElements<CT<R_U>, CT<R_V>, CT<R_Us>... > == NR * NC > >
	explicit Matrix(R_U && u, R_V && v, R_Us && ...us) 
		: NamedObject<Matrix>(
		 PARENTHESIS | DISPLAY_TYPE, IS_TRACKED, "", 
			EX(R_U, u), EX(R_V, v), EX(R_Us, us)... )
	{
		//std::cout << "multictor " << std::endl; 
		//areNotInit(u, v, us...);
		//checkForTemp<R_U, R_V, R_Us...>(u, v, us...);
		//exp = createInit<Matrix>(namePtr, getExp<R_U>(u), getExp<R_V>(v), getExp<R_Us>(us)...);
	}

	//Matrix(const Ex & _exp, NamedObjectTracking _track = TRACKED, NamedObjectInit _init = INIT )  : NamedObject<Matrix>() {
	//	//std::cout << " from exp " << std::endl;
	//
	//	if (_init) {
	//		exp = createInit<Matrix, HIDE, NO_PARENTHESIS>(namePtr, _exp);
	//	} else {
	//		exp = _exp;
	//	}
	//	
	//	
	//	if (!_track) {
	//		areNotInit(*this);
	//	}

	//	//exp = _exp;
	//	isUsed = false;
	//}

	//Matrix(const MatrixInit<type,NR,NC> & t)
	//	: NamedObject<Matrix>(t.exp, t) 
	//{
	//	//std::cout << "from TiniT" << std::endl;
	//	//exp = createInit<Matrix, HIDE, NO_PARENTHESIS>(namePtr, t.exp);
	//	//std::cout << std::dynamic_pointer_cast<CtorBase>(exp->op)->status << t.exp->str() <<   std::endl;
	//}

	//Matrix & operator=(const MatrixInit < type, NR, NC> & other) = delete;


	///////////////////

	template<bool b = !isBool, typename = std::enable_if_t<b> >
	Matrix operator++() const & {
		return Matrix(createExp<PrefixUnary>("++", getExp<Matrix,false>(*this)));
	}
	template<bool b = !isBool, typename = std::enable_if_t<b> >
	Matrix operator++() const && {
		return Matrix(createExp<PrefixUnary>("++", getExp<Matrix, true>(*this)));
	}

	template<bool b = !isBool, typename = std::enable_if_t<b> >
	Matrix operator++(int) const &  {
		return Matrix(createExp<PostfixUnary>("++ ", getExp<Matrix, false>(*this)));
	}
	template<bool b = !isBool, typename = std::enable_if_t<b> >
	Matrix operator++(int) const && {
		return Matrix(createExp<PostfixUnary>("++ ", getExp<Matrix, true>(*this)));
	}


	template<bool b = !isBool, typename = std::enable_if_t<b> >
	Matrix operator--() const & {
		return Matrix(createExp<PrefixUnary>("--", getExp<Matrix, false>(*this)));
	}
	template<bool b = !isBool, typename = std::enable_if_t<b> >
	Matrix operator--() const && {
		areNotInit(*this);
		return Matrix(createExp<PrefixUnary>("--", getExp<Matrix, true>(*this)));
	}

	template<bool b = !isBool, typename = std::enable_if_t<b> >
	Matrix operator--(int) const & {
		return Matrix(createExp<PostfixUnary>("-- ", getExp<Matrix, false>(*this)));
	}
	template<bool b = !isBool, typename = std::enable_if_t<b> >
	Matrix operator--(int) const && {
		areNotInit(*this);
		return Matrix(createExp<PostfixUnary>("-- ", getExp<Matrix, true>(*this)));
	}

	template<bool b = !isBool, typename = std::enable_if_t<b> >
	Matrix operator-() const & {
		return Matrix(createExp<PrefixUnary>("-", getExp<Matrix, false>(*this)));
	}
	template<bool b = !isBool, typename = std::enable_if_t<b> >
	Matrix operator-() const && {
		areNotInit(*this);
		return Matrix(createExp<PrefixUnary>("-", getExp<Matrix, true>(*this)));
	}

	template<bool b = isBool, typename = std::enable_if_t<b> >
	Matrix operator!() const & {
		return Matrix(createExp<PrefixUnary>("!", getExp<Matrix, false>(*this)));
	}
	template<bool b = isBool, typename = std::enable_if_t<b> >
	Matrix operator!() const && {
		areNotInit(*this);
		return Matrix(createExp<PrefixUnary>("!", getExp<Matrix, true>(*this)));
	}

	//operators X=
	template<typename R_A, typename A = CleanType<R_A>,
		typename = std::enable_if_t<isAssignable && NotBool<A> && (EqualMat<Matrix, A> || IsScalar<A>)  >  >
	void operator+=(R_A&& a) const & {
		listen().addEvent(createExp<MiddleOperator<ASSIGNMENT>>(" += ", getExp<Matrix, false>(*this), getExp<R_A>(a)));
	}

	template<typename R_A, typename A = CleanType<R_A>,
		typename = std::enable_if_t<isAssignable && NotBool<A> && (EqualMat<Matrix, A> || IsScalar<A>)  >  >
		void operator+=(R_A&& a) const && {
		listen().addEvent(createExp<MiddleOperator<ASSIGNMENT>>("+=", getExp<Matrix, true>(*this), getExp<R_A>(a)));
	}
	//template<typename R_A, typename A = CleanType<R_A>,
	//	typename = std::enable_if_t<!assignable && NotBool<A> && (EqualMat<Matrix, A> || IsScalar<A>)  >  >
	//	void operator+=(R_A&& a) const && = delete;

	template<typename R_A, typename A = CleanType<R_A>,
		typename = std::enable_if_t<isAssignable && NotBool<A> && (EqualMat<Matrix, A> || IsScalar<A>)  >  >
		void operator-=(R_A&& a) const && {
		listen().addEvent(createExp<MiddleOperator<ASSIGNMENT>>(" -= ", getExp<Matrix, true>(*this), getExp<R_A>(a)));
	}
	template<typename R_A, typename A = CleanType<R_A>,
		typename = std::enable_if_t<isAssignable && NotBool<A> && (EqualMat<Matrix, A> || IsScalar<A>)  >  >
		void operator-=(R_A&& a) const & {
		listen().addEvent(createExp<MiddleOperator<ASSIGNMENT>>(" -= ", getExp<Matrix, false>(*this), getExp<R_A>(a)));
	}

	//template<typename R_A, typename A = CleanType<R_A>,
	//	typename = std::enable_if_t<!assignable && NotBool<A> && (EqualMat<Matrix, A> || IsScalar<A>)  >  >
	//	void operator-=(R_A&& a) const && = delete;

	template<typename R_A, typename A = CleanType<R_A>,
		typename = std::enable_if_t< NotBool<A> && (EqualMat<Matrix, A> || IsScalar<A>)  >  >
		void operator*=(R_A&& a) const & {
		listen().addEvent(createExp<MiddleOperator<ASSIGNMENT>>(" *= ", getExp<Matrix, false>(*this), getExp<R_A>(a)));
	}

	// swizzles accessors
	// repeated symbols, not assignable
	//template<uint Dim, uint Size, SwizzleSet Set, uint Bytes,
	//	typename = std::enable_if_t<NC == 1 && NR != 1 && Set != MIXED_SET && Dim <= NR && Size <= NR > >
	//	Vec<type, Size> operator[](const SwizzlePack<Dim, Size, Set, Bytes, true> & swizzle) const & {
	//	auto exp = createExp(std::make_shared<FunctionOp<IN_BETWEEN_NOSPACE, NO_PARENTHESIS>>("."),
	//		getExp<Matrix, false>(*this),
	//		createExp(std::make_shared<Alias>(swizzle.s))
	//	);
	//	return Vec<type, Size>(Matrix_Track::UNTRACKED, exp->str());
	//}

	template<uint Dim, uint Size, SwizzleSet Set, uint Bytes,
		typename = std::enable_if_t<NC == 1 && NR != 1 && Set != MIXED_SET && Dim <= NR && Size <= NR > >
		Vec<type, Size, NON_ASSIGNABLE> operator[](const SwizzlePack<Dim, Size, Set, Bytes, true> & swizzle) const & {
		return Vec<type, Size, NON_ASSIGNABLE>(
			createExp<FieldSelector>(getExp<Matrix, false>(*this), swizzle.s)
		);
	}

	template<uint Dim, uint Size, SwizzleSet Set, uint Bytes,
		typename = std::enable_if_t<NC == 1 && NR != 1 && Set != MIXED_SET && Dim <= NR && Size <= NR > >
		Vec<type, Size, NON_ASSIGNABLE> operator[](const SwizzlePack<Dim, Size, Set, Bytes, true> & swizzle) const && {
		return Vec<type, Size, NON_ASSIGNABLE>(
			createExp<FieldSelector>(getExp<Matrix, true>(*this), swizzle.s)
		);
	}

	// non repeated symbols, assignable
	template<uint Dim, uint Size, SwizzleSet Set, uint Bytes,
		typename = std::enable_if_t<NC == 1 && NR != 1 && Set != MIXED_SET && Dim <= NR && Size <= NR > >
		Vec<type, Size> operator[](const SwizzlePack<Dim, Size, Set, Bytes, false> & swizzle) const & {
		return Vec<type, Size>(
			createExp<FieldSelector>(getExp<Matrix, false>(*this), swizzle.s)
		);
	}

	template<uint Dim, uint Size, SwizzleSet Set, uint Bytes,
		typename = std::enable_if_t<NC == 1 && NR != 1 && Set != MIXED_SET && Dim <= NR && Size <= NR > >
		Vec<type, Size> operator[](const SwizzlePack<Dim, Size, Set, Bytes, false> & swizzle) const && {
		return Vec<type, Size>(
			createExp<FieldSelector>(getExp<Matrix, true>(*this), swizzle.s)
		);
	}

	// array subscript accessors
	template<bool b = !isScalar, typename SubscriptType = typename std::conditional_t< NC == 1, Scalar<type>, Vec<type, NR> >,
		typename = std::enable_if_t<b> >
		SubscriptType operator[](uint i) const & {
		return SubscriptType(
			createExp<ArraySubscript>(
				getExp<Matrix, false>(*this),
				createExp<Litteral<uint>>(i)
			)
		);
	}

	template<bool b = !isScalar, typename SubscriptType = typename std::conditional_t< NC == 1, Scalar<type>, Vec<type, NR> >,
		typename = std::enable_if_t<b> >
		SubscriptType operator[](uint i) const && {
		return SubscriptType(
			createExp<ArraySubscript>(
				getExp<Matrix, true>(*this),
				createExp<Litteral<uint>>(i)
			)
		);
	}
////

	template<bool b = !isScalar, typename = std::enable_if_t<b> >
	const Int length() const & {
		return Int(
			createExp<MemberSelector<0>>(
				getExp<Matrix,false>(*this),
				"length"
			)
		);
	}
	
	const Int length() const && = delete; 

	template<typename R_A, typename A = CleanType<R_A>,
		typename = std::enable_if_t< NotBool<A> && (EqualMat<Matrix, A> || IsScalar<A>)  >  >
		void operator*=(R_A&& a) const && = delete;

	template<bool b = isBool && NC == 1 && NR == 1, typename = std::enable_if_t<b> >
	operator bool() const {
		return false;
	}

	//template<bool b = (assignable == NON_ASSIGNABLE), typename = std::enable_if_t<b> >
	//operator Matrix<type,NR,NC,ASSIGNABLE>() const & {
	//	return Matrix<type, NR, NC, ASSIGNABLE>(getExpForced<false,Matrix<type, NR, NC, NON_ASSIGNABLE>>(*this));
	//}

};

//// MatrixInit
//template<ScalarType type, unsigned int NR, unsigned int NC>
//MatrixInit<type,NR,NC> operator<<(const Matrix<type,NR,NC> & m, const std::string & s) {
//	return MatrixInit<type,NR,NC>(getExp< Matrix<type, NR, NC>,false>(m), s);
//}
//
//template<ScalarType type, unsigned int NR, unsigned int NC>
//MatrixInit<type, NR, NC> operator<<(Matrix<type, NR, NC> && m, const std::string & s) {
//	return MatrixInit<type, NR, NC>(getExp<Matrix<type,NR,NC>,true>(m), s);
//}
//
//template<ScalarType type, unsigned int NR, unsigned int NC>
//MatrixInit<type, NR, NC>::MatrixInit(const Ex & _exp, const std::string & s) 
//	: name(s), exp(_exp) 
//{
//}


// Bool operators
template<typename R_A, typename A = CleanType<R_A>, typename R_B, typename B = CleanType<R_B>,
	typename = std::enable_if_t< EqualMat<A, Bool> &&  EqualMat<B, Bool>  > >
Bool operator&&(R_A && b1, R_B && b2) 
{
	return Bool(createExp<MiddleOperator<LOGICAL_AND>>(" && ", getExp<R_A>(b1), getExp<R_B>(b2)));
}

template<typename R_A, typename A = CleanType<R_A>, typename R_B, typename B = CleanType<R_B>,
	typename = std::enable_if_t< EqualMat<A, Bool> &&  EqualMat<B, Bool>  > >
Bool operator||(R_A && b1, R_B && b2) 
{
	return Bool(createExp<MiddleOperator<LOGICAL_OR>>(" || ", getExp<R_A>(b1), getExp<R_B>(b2)));
}

template<typename R_A, typename A = CleanType<R_A>, typename R_B, typename B = CleanType<R_B>,
	typename = std::enable_if_t< NoBools<A,B> && EqualDim<A,B> > >
	Bool operator==(R_A && b1, R_B && b2)
{
	return Bool(createExp<MiddleOperator<EQUALITY>>(" == ", getExp<R_A>(b1), getExp<R_B>(b2)));
}

// > and < operators
template<typename R_A, typename A = CleanType<R_A>, typename R_B, typename B = CleanType<R_B>, 
	typename = std::enable_if_t< NoBools<A, B> && IsScalar<A> && IsScalar<B> > >
Bool operator>(R_A && a, R_B && b) 
{
	return Bool(createExp<MiddleOperator<RELATIONAL>>(" > ", getExp<R_A>(a), getExp<R_B>(b)));
}

template<typename R_A, typename A = CleanType<R_A>, typename R_B, typename B = CleanType<R_B>,
	typename = std::enable_if_t< NoBools<A, B> && IsScalar<A> && IsScalar<B> > >
Bool operator<(R_A && a, R_B && b)
{
	return Bool(createExp<MiddleOperator<RELATIONAL>>(" < ", getExp<R_A>(a), getExp<R_B>(b)));
}

// + and - operators
template<typename R_A, typename A = CleanType<R_A>, typename R_B, typename B = CleanType<R_B>, 
	typename = std::enable_if_t< NoBools<A, B> && (EqualMat<A, B> || ( IsScalar<A> || IsScalar<B> ) ) > >
ArithmeticBinaryReturnType<A, B> operator+(R_A && a, R_B && b)
{
	return { createExp<MiddleOperator<ADDITION>>(" + ", EX(R_A, a), EX(R_B, b)) };
}

template<typename R_A, typename A = CleanType<R_A>, typename R_B, typename B = CleanType<R_B>,
	typename = std::enable_if_t< NoBools<A, B> && (EqualMat<A, B> || (IsScalar<A> || IsScalar<B>)) > >
ArithmeticBinaryReturnType<A, B> operator-(R_A && a, R_B && b)
{
	return ArithmeticBinaryReturnType<A, B>(createExp<MiddleOperator<SUBSTRACTION>>("-", getExp<R_A>(a), getExp<R_B>(b)));
}

// * operators
// matrix multiplication
template<typename R_A, typename A = CleanType<R_A>, typename R_B, typename B = CleanType<R_B>,
	typename = std::enable_if_t< NoBools<A, B> && ValidForMatMultiplication<A,B> > >
	MultiplicationReturnType<A, B> operator*(R_A && a, R_B && b)
{
	return { createExp<MiddleOperator<MULTIPLY>>("*", EX(R_A, a), EX(R_B, b) ) };
}

// cwise multiplication
template<typename R_A, typename A = CleanType<R_A>, typename R_B, typename B = CleanType<R_B>,
	typename = std::enable_if_t< NoBools<A, B> && ( IsScalar<A> || IsScalar<B> ) > >
	ArithmeticBinaryReturnType<A, B> operator*(R_A && a, R_B && b)
{
	return { createExp<MiddleOperator<MULTIPLY>>("*", EX(R_A, a), EX(R_B, b)) };
}

// ()/() operator
template<typename R_A, typename A = CleanType<R_A>, typename R_B, typename B = CleanType<R_B>,
	typename = std::enable_if_t< NoBools<A, B> && (EqualMat<A, B> || (IsScalar<A> || IsScalar<B>)) > >
	ArithmeticBinaryReturnType<A, B> operator/(R_A && a, R_B && b)
{
	return { createExp<MiddleOperator<DIVISION>>("/", EX(R_A, a), EX(R_B, b)) };
}


//

//template<typename T, uint N>
//struct Array : NamedObject<Array<T,N>> {
//	using Type = typename T::UnderlyingType;
//
//	explicit Array(const std::string & _name = "") : NamedObject<Array>(_name) {
//		NamedObjectBase::exp = createArrayDeclaration<Array>(NamedObjectBase::myNamePtr());
//	}
//
//	template<typename R_A, typename A = CleanType<R_A>, 
//		typename = std::enable_if_t< IsInteger<A> > >
//		const Type & operator[](R_A && a) const {
//
//		previous_calls.push_back(
//			Type(createExp<ArraySubscript>(
//				getExp<Array, false>(*this),
//				getExp<R_A>(a)
//				), NOT_TRACKED, NO_INIT
//			)
//		);
//		areNotInit(previous_calls.back());
//		return previous_calls.back();
//	}
//
//	template<typename R_A, typename A = CleanType<R_A>,
//		typename = std::enable_if_t< IsInteger<A> > >
//		Type & operator[](R_A && a) {
//		
//		previous_calls.push_back(
//			Type(createExp<ArraySubscript>(
//				getExp<Array, false>(*this),
//				getExp<R_A>(a)
//				), NOT_TRACKED, NO_INIT
//			)
//		);
//		//areNotInit(previous_calls.back());
//		
//		return previous_calls.back();
//	}
//
//	std::vector<Type> previous_calls;
//};