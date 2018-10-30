#pragma once

#include "ExpressionsTest.h"
#include "Swizzles.h"

template<ScalarType type, uint Nrows, uint Ncols>
struct MatrixInit {
	MatrixInit(const Ex & ex, const std::string & s);

	std::string name;
	Ex exp;
};

/// matrix class

template<ScalarType type, uint NR, uint NC, AssignType assignable>
class Matrix : public NamedObject<Matrix<type, NR, NC, assignable>> {
public:
	using NamedObjectBase::exp;
	using NamedObjectBase::namePtr;
	using NamedObjectBase::isUsed;
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

	explicit Matrix(const std::string & _name = "", NamedObjectTracking _track = TRACKED, NamedObjectBase * _parent = nullptr, bool _isUsed = true)
		: NamedObject<Matrix>(_name, _track, _parent, _isUsed) {
		exp = createDeclaration<Matrix>(NamedObjectBase::myNamePtr());
		if (_parent || !_track) {
			//std::static_pointer_cast<CtorBase>(exp->op)->firstStr = false;
			areNotInit(*this);
		}
	}

	template <std::size_t N>
	explicit Matrix(const char(&s)[N]) : NamedObject<Matrix>(s) {
		exp = createDeclaration<Matrix>(NamedObjectBase::myNamePtr());
	}

	// constructor from cpp types (bool, int, and double)
	template<typename U, typename = std::enable_if_t<!isBool && (NR == 1 && NC == 1) && AreValid<U> && !Infos<U>::is_glsl_type > >
	Matrix( U && u, const std::string & s) : NamedObject<Matrix>(s) {
		exp = createInit<Matrix, HIDE, IN_FRONT, NO_PARENTHESIS>(namePtr, getExp<U>(u));
	}

	//glsl constructors

	// matXY(int/float) and matXY(matWZ)
	template<typename R_T, typename T = CleanType<R_T>, typename = std::enable_if_t< 
		AreValid<T> && (
			isScalar || IsScalar<T>  || //matXY(int/float)
		(!isBool && NC != 1 && NR != 1 && Infos<T>::cols != 1 && Infos<T>::rows != 1) //matXY(matWZ)
			) > >
	Matrix( R_T && x) : NamedObject<Matrix>() {
		
		//checkForTemp<R_T>(x);
		if ( EqualMat<Matrix,T> ) {
			exp = createInit<Matrix, HIDE, IN_FRONT, NO_PARENTHESIS>(namePtr, getExp<R_T>(x));
		} else {
			exp = createInit<Matrix, DISPLAY, IN_FRONT, USE_PARENTHESIS>(namePtr, getExp<R_T>(x));
		}
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

	Matrix & operator=(const Matrix & other) & {
		listen().addEvent(createExp(std::make_shared<SingleCharBinaryOp<'=', NO_PARENTHESIS>>(),
			getExp<Matrix, false>(*this),
			getExp<Matrix, false>(other))
		);
		return *this;
	}


	/////////////////////////////////////////////////////

	template<AssignType other_assignable, bool valid = isAssignable, typename = std::enable_if_t<valid>  >
	void operator=(const OtherMat<other_assignable>& other) & {
		listen().addEvent(createExp(std::make_shared<SingleCharBinaryOp<'=', NO_PARENTHESIS>>(), 
			getExp<Matrix, false>(*this),
			getExp<OtherMat<other_assignable>,false>(other))
		);
	}

	template<AssignType other_assignable, bool valid = isAssignable, typename = std::enable_if_t<valid> >
	void operator=(OtherMat<other_assignable>&& other) & {
		listen().addEvent(createExp(std::make_shared<SingleCharBinaryOp<'=', NO_PARENTHESIS>>(),
			getExp<Matrix, false>(*this),
			getExp<OtherMat<other_assignable>, true>(other))
		);
	}

	template<AssignType other_assignable, bool valid = isAssignable, typename = std::enable_if_t<valid> >
	void operator=(const OtherMat<other_assignable>& other) && {
		listen().addEvent(createExp(std::make_shared<SingleCharBinaryOp<'=', NO_PARENTHESIS>>(),
			getExp<Matrix, true>(*this),
			getExp<OtherMat<other_assignable>, false>(other))
		);
	}

	template<AssignType other_assignable, bool valid = isAssignable, typename = std::enable_if_t<valid>  >
	void operator=(OtherMat<other_assignable>&& other) && {
		listen().addEvent(createExp(std::make_shared<SingleCharBinaryOp<'=', NO_PARENTHESIS>>(),
			getExp<Matrix,true>(*this),
			getExp<OtherMat<other_assignable>, true>(other))
		);
	}
	
	template<typename U, typename = std::enable_if_t < 
		!EqualDim<Matrix,U> ||
		(Infos<U>::scalar_type > type)
	> >
	Matrix & operator=(const  U & u) = delete;

	template<typename R_U, typename R_V, typename U = CleanType<R_U>, typename V = CleanType<R_V>, typename ...R_Us,
		typename = std::enable_if_t < AreValid<U, V, CleanType<R_Us>... > && MatElements<U, V, CleanType<R_Us>... > == NR * NC > >
	explicit Matrix(R_U && u, R_V && v, R_Us && ...us) : NamedObject<Matrix>() {
		//std::cout << "multictor " << std::endl; 
		//areNotInit(u, v, us...);
		//checkForTemp<R_U, R_V, R_Us...>(u, v, us...);
		exp = createInit<Matrix>(namePtr, getExp<R_U>(u), getExp<R_V>(v), getExp<R_Us>(us)...);
	}

	Matrix(const Ex & _exp)  : NamedObject<Matrix>() {
		//std::cout << " from exp " << std::endl;
		exp = createInit<Matrix, HIDE, IN_FRONT, NO_PARENTHESIS>(namePtr, _exp);
		isUsed = false;
	}

	Matrix(const MatrixInit<type,NR,NC> & t) : NamedObject<Matrix>(t.name) {
		//std::cout << "from TiniT" << std::endl;
		exp = createInit<Matrix, HIDE, IN_FRONT, NO_PARENTHESIS>(namePtr, t.exp);
		//std::cout << std::dynamic_pointer_cast<CtorBase>(exp->op)->status << t.exp->str() <<   std::endl;
	}

	Matrix & operator=(const MatrixInit < type, NR, NC> & other) = delete;


	///////////////////

	template<bool b = !isBool, typename = std::enable_if_t<b> >
	Matrix operator++() const & {
		return Matrix(createExp(std::make_shared<FunctionOp<IN_FRONT,NO_PARENTHESIS>>("++"), getExp<Matrix,false>(*this)));
	}
	template<bool b = !isBool, typename = std::enable_if_t<b> >
	Matrix operator++() const && {
		return Matrix(createExp(std::make_shared<FunctionOp<IN_FRONT, NO_PARENTHESIS>>("++"), getExp<Matrix, true>(*this)));
	}

	template<bool b = !isBool, typename = std::enable_if_t<b> >
	Matrix operator++(int) const &  {
		return Matrix(createExp(std::make_shared<FunctionOp<BEHIND>>("++ "), getExp<Matrix, false>(*this)));
	}
	template<bool b = !isBool, typename = std::enable_if_t<b> >
	Matrix operator++(int) const && {
		return Matrix(createExp(std::make_shared<FunctionOp<BEHIND>>("++ "), getExp<Matrix, true>(*this)));
	}


	template<bool b = !isBool, typename = std::enable_if_t<b> >
	Matrix operator--() const & {
		return Matrix(createExp(std::make_shared<FunctionOp<IN_FRONT, NO_PARENTHESIS>>(" --"), getExp<Matrix, false>(*this)));
	}
	template<bool b = !isBool, typename = std::enable_if_t<b> >
	Matrix operator--() const && {
		areNotInit(*this);
		return Matrix(createExp(std::make_shared<FunctionOp<IN_FRONT, NO_PARENTHESIS>>(" --"), getExp<Matrix, true>(*this)));
	}

	template<bool b = !isBool, typename = std::enable_if_t<b> >
	Matrix operator--(int) const & {
		return Matrix(createExp(std::make_shared<FunctionOp<BEHIND>>("-- "), getExp<Matrix, false>(*this)));
	}
	template<bool b = !isBool, typename = std::enable_if_t<b> >
	Matrix operator--(int) const && {
		areNotInit(*this);
		return Matrix(createExp(std::make_shared<FunctionOp<BEHIND>>("-- "), getExp<Matrix, true>(*this)));
	}

	template<bool b = !isBool, typename = std::enable_if_t<b> >
	Matrix operator-() const & {
		return Matrix(createExp(std::make_shared<FunctionOp<IN_FRONT, NO_PARENTHESIS>>("-"), getExp<Matrix, false>(*this)));
	}
	template<bool b = !isBool, typename = std::enable_if_t<b> >
	Matrix operator-() const && {
		areNotInit(*this);
		return Matrix(createExp(std::make_shared<FunctionOp<IN_FRONT, NO_PARENTHESIS>>("-"), getExp<Matrix, true>(*this)));
	}

	template<bool b = isBool, typename = std::enable_if_t<b> >
	Matrix operator!() const & {
		return Matrix(createExp(std::make_shared<FunctionOp<IN_FRONT, NO_PARENTHESIS>>("!"), getExp<Matrix, false>(*this)));
	}
	template<bool b = isBool, typename = std::enable_if_t<b> >
	Matrix operator!() const && {
		areNotInit(*this);
		return Matrix(createExp(std::make_shared<FunctionOp<IN_FRONT>>("!"), getExp<Matrix, true>(*this)));
	}

	//operators X=
	template<typename R_A, typename A = CleanType<R_A>,
		typename = std::enable_if_t<isAssignable && NotBool<A> && (EqualMat<Matrix, A> || IsScalar<A>)  >  >
	void operator+=(R_A&& a) const & {
		listen().addEvent(createExp(std::make_shared<FunctionOp<IN_BETWEEN, NO_PARENTHESIS>>("+="), getExp<Matrix, false>(*this), getExp<R_A>(a)));
	}

	template<typename R_A, typename A = CleanType<R_A>,
		typename = std::enable_if_t<isAssignable && NotBool<A> && (EqualMat<Matrix, A> || IsScalar<A>)  >  >
		void operator+=(R_A&& a) const && {
		listen().addEvent(createExp(std::make_shared<FunctionOp<IN_BETWEEN, NO_PARENTHESIS>>("+="), getExp<Matrix, true>(*this), getExp<R_A>(a)));
	}
	//template<typename R_A, typename A = CleanType<R_A>,
	//	typename = std::enable_if_t<!assignable && NotBool<A> && (EqualMat<Matrix, A> || IsScalar<A>)  >  >
	//	void operator+=(R_A&& a) const && = delete;

	template<typename R_A, typename A = CleanType<R_A>,
		typename = std::enable_if_t<isAssignable && NotBool<A> && (EqualMat<Matrix, A> || IsScalar<A>)  >  >
		void operator-=(R_A&& a) const && {
		listen().addEvent(createExp(std::make_shared<FunctionOp<IN_BETWEEN, NO_PARENTHESIS>>("-="), getExp<Matrix, true>(*this), getExp<R_A>(a)));
	}
	template<typename R_A, typename A = CleanType<R_A>,
		typename = std::enable_if_t<isAssignable && NotBool<A> && (EqualMat<Matrix, A> || IsScalar<A>)  >  >
		void operator-=(R_A&& a) const & {
		listen().addEvent(createExp(std::make_shared<FunctionOp<IN_BETWEEN, NO_PARENTHESIS>>("-="), getExp<Matrix, false>(*this), getExp<R_A>(a)));
	}

	//template<typename R_A, typename A = CleanType<R_A>,
	//	typename = std::enable_if_t<!assignable && NotBool<A> && (EqualMat<Matrix, A> || IsScalar<A>)  >  >
	//	void operator-=(R_A&& a) const && = delete;

	template<typename R_A, typename A = CleanType<R_A>,
		typename = std::enable_if_t< NotBool<A> && (EqualMat<Matrix, A> || IsScalar<A>)  >  >
		void operator*=(R_A&& a) const & {
		listen().addEvent(createExp(std::make_shared<FunctionOp<IN_BETWEEN, NO_PARENTHESIS>>("*="), getExp<Matrix, false>(*this), getExp<R_A>(a)));
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
			createExp(std::make_shared<FunctionOp<IN_BETWEEN_NOSPACE, NO_PARENTHESIS>>("."),
				getExp<Matrix, false>(*this),
				createExp(std::make_shared<Alias>(swizzle.s))
			)
		);
	}

	template<uint Dim, uint Size, SwizzleSet Set, uint Bytes,
		typename = std::enable_if_t<NC == 1 && NR != 1 && Set != MIXED_SET && Dim <= NR && Size <= NR > >
		Vec<type, Size, NON_ASSIGNABLE> operator[](const SwizzlePack<Dim, Size, Set, Bytes, true> & swizzle) const && {
		return Vec<type, Size, NON_ASSIGNABLE>(
			createExp(std::make_shared<FunctionOp<IN_BETWEEN_NOSPACE, NO_PARENTHESIS>>("."),
				getExp<Matrix, true>(*this),
				createExp(std::make_shared<Alias>(swizzle.s))
			)
		);
	}

	// non repeated symbols, assignable
	template<uint Dim, uint Size, SwizzleSet Set, uint Bytes,
		typename = std::enable_if_t<NC == 1 && NR != 1 && Set != MIXED_SET && Dim <= NR && Size <= NR > >
		Vec<type, Size> operator[](const SwizzlePack<Dim, Size, Set, Bytes, false> & swizzle) const & {
		return Vec<type, Size>(
			createExp(std::make_shared<FunctionOp<IN_BETWEEN_NOSPACE, NO_PARENTHESIS>>("."),
				getExp<Matrix, false>(*this),
				createExp(std::make_shared<Alias>(swizzle.s))
			)
		);
	}

	template<uint Dim, uint Size, SwizzleSet Set, uint Bytes,
		typename = std::enable_if_t<NC == 1 && NR != 1 && Set != MIXED_SET && Dim <= NR && Size <= NR > >
		Vec<type, Size> operator[](const SwizzlePack<Dim, Size, Set, Bytes, false> & swizzle) const && {
		return Vec<type, Size>(
			createExp(std::make_shared<FunctionOp<IN_BETWEEN_NOSPACE, NO_PARENTHESIS>>("."),
				getExp<Matrix, true>(*this),
				createExp(std::make_shared<Alias>(swizzle.s))
			)
		);
	}

	// array subscript accessors
	template<bool b = !isScalar, typename SubscriptType = typename std::conditional_t< NC == 1, Scalar<type>, Vec<type, NR> >,
		typename = std::enable_if_t<b> >
		SubscriptType operator[](uint i) const & {
		return SubscriptType(
			createExp(std::make_shared<FunctionOp<IN_FRONT, ARGS_BRACKETS>>(getExp<Matrix, false>(*this)->str()),
				createExp(std::make_shared < Litteral<uint> >(i))
			)
		);
	}

	template<bool b = !isScalar, typename SubscriptType = typename std::conditional_t< NC == 1, Scalar<type>, Vec<type, NR> >,
		typename = std::enable_if_t<b> >
		SubscriptType operator[](uint i) const && {
		return SubscriptType(
			createExp(std::make_shared<FunctionOp<IN_FRONT, ARGS_BRACKETS>>(getExp<Matrix, true>(*this)->str()),
				createExp(std::make_shared < Litteral<uint> >(i))
			)
		);
	}
////

	template<bool b = !isScalar, typename = std::enable_if_t<b> >
	const Int length() const & {
		return Int(createExp(std::make_shared<MemberFunctionOp>(NamedObjectBase::myNamePtr(),"length")));
	}
	
	const Int length() const && = delete; 

	template<typename R_A, typename A = CleanType<R_A>,
		typename = std::enable_if_t< NotBool<A> && (EqualMat<Matrix, A> || IsScalar<A>)  >  >
		void operator*=(R_A&& a) const && = delete;

	template<bool b = isBool && NC == 1 && NR == 1, typename = std::enable_if_t<b> >
	operator bool() const {
		return false;
	}

	template<bool b = (assignable == NON_ASSIGNABLE), typename = std::enable_if_t<b> >
	operator Matrix<type,NR,NC,ASSIGNABLE>() const & {
		return Matrix<type, NR, NC, ASSIGNABLE>(getExpForced<false,Matrix<type, NR, NC, NON_ASSIGNABLE>>(*this));
	}

};

//// MatrixInit
template<ScalarType type, unsigned int NR, unsigned int NC>
MatrixInit<type,NR,NC> operator<<(const Matrix<type,NR,NC> & m, const std::string & s) {
	return MatrixInit<type,NR,NC>(getExp< Matrix<type, NR, NC>,false>(m), s);
}

template<ScalarType type, unsigned int NR, unsigned int NC>
MatrixInit<type, NR, NC> operator<<(Matrix<type, NR, NC> && m, const std::string & s) {
	return MatrixInit<type, NR, NC>(getExp<Matrix<type,NR,NC>,true>(m), s);
}

template<ScalarType type, unsigned int NR, unsigned int NC>
MatrixInit<type, NR, NC>::MatrixInit(const Ex & _exp, const std::string & s) 
	: name(s), exp(_exp) {
}


// Bool operators
template<typename R_A, typename A = CleanType<R_A>, typename R_B, typename B = CleanType<R_B>,
	typename = std::enable_if_t< EqualMat<A, Bool> &&  EqualMat<B, Bool>  > >
Bool operator&&(R_A && b1, R_B && b2) 
{
	return Bool(createExp(std::make_shared<FunctionOp<IN_BETWEEN>>("&&"), getExp<R_A>(b1), getExp<R_B>(b2)));
}

template<typename R_A, typename A = CleanType<R_A>, typename R_B, typename B = CleanType<R_B>,
	typename = std::enable_if_t< EqualMat<A, Bool> &&  EqualMat<B, Bool>  > >
Bool operator||(R_A && b1, R_B && b2) 
{
	return Bool(createExp(std::make_shared<FunctionOp<IN_BETWEEN>>("||"), getExp<R_A>(b1), getExp<R_B>(b2)));
}

template<typename R_A, typename A = CleanType<R_A>, typename R_B, typename B = CleanType<R_B>,
	typename = std::enable_if_t< NoBools<A,B> && EqualDim<A,B> > >
	Bool operator==(R_A && b1, R_B && b2)
{
	return Bool(createExp(std::make_shared<FunctionOp<IN_BETWEEN>>("=="), getExp<R_A>(b1), getExp<R_B>(b2)));
}

// > and < operators
template<typename R_A, typename A = CleanType<R_A>, typename R_B, typename B = CleanType<R_B>, 
	typename = std::enable_if_t< NoBools<A, B> && IsScalar<A> && IsScalar<B> > >
Bool operator>(R_A && a, R_B && b) 
{
	return Bool(createExp(std::make_shared<FunctionOp<IN_BETWEEN, NO_PARENTHESIS>>(">"), getExp<R_A>(a), getExp<R_B>(b)));
}

template<typename R_A, typename A = CleanType<R_A>, typename R_B, typename B = CleanType<R_B>,
	typename = std::enable_if_t< NoBools<A, B> && IsScalar<A> && IsScalar<B> > >
Bool operator<(R_A && a, R_B && b)
{
	return Bool(createExp(std::make_shared<FunctionOp<IN_BETWEEN, NO_PARENTHESIS>>("<"), getExp<R_A>(a), getExp<R_B>(b)));
}

// + and - operators
template<typename R_A, typename A = CleanType<R_A>, typename R_B, typename B = CleanType<R_B>, 
	typename = std::enable_if_t< NoBools<A, B> && (EqualMat<A, B> || ( IsScalar<A> || IsScalar<B> ) ) > >
ArithmeticBinaryReturnType<A, B> operator+(R_A && a, R_B && b)
{
	return ArithmeticBinaryReturnType<A, B>(createExp(std::make_shared<FunctionOp<IN_BETWEEN,NO_PARENTHESIS>>("+"), getExp<R_A>(a), getExp<R_B>(b)));
}

template<typename R_A, typename A = CleanType<R_A>, typename R_B, typename B = CleanType<R_B>,
	typename = std::enable_if_t< NoBools<A, B> && (EqualMat<A, B> || (IsScalar<A> || IsScalar<B>)) > >
ArithmeticBinaryReturnType<A, B> operator-(R_A && a, R_B && b)
{
	return ArithmeticBinaryReturnType<A, B>(createExp(std::make_shared<FunctionOp<IN_BETWEEN, NO_PARENTHESIS>>("-"), getExp<R_A>(a), getExp<R_B>(b)));
}

// * operators
// matrix multiplication
template<typename R_A, typename A = CleanType<R_A>, typename R_B, typename B = CleanType<R_B>,
	typename = std::enable_if_t< NoBools<A, B> && ValidForMatMultiplication<A,B> > >
	MultiplicationReturnType<A, B> operator*(R_A && a, R_B && b)
{
	return MultiplicationReturnType<A, B>(createExp(std::make_shared<FunctionOp<IN_BETWEEN, NO_PARENTHESIS>>("*"), getExp<R_A>(a), getExp<R_B>(b)));
}

// cwise multiplication
template<typename R_A, typename A = CleanType<R_A>, typename R_B, typename B = CleanType<R_B>,
	typename = std::enable_if_t< NoBools<A, B> && ( IsScalar<A> || IsScalar<B> ) > >
	ArithmeticBinaryReturnType<A, B> operator*(R_A && a, R_B && b)
{
	return ArithmeticBinaryReturnType<A, B>(createExp(std::make_shared<FunctionOp<IN_BETWEEN, NO_PARENTHESIS>>("*"), getExp<R_A>(a), getExp<R_B>(b)));
}

// ()/() operator
template<typename R_A, typename A = CleanType<R_A>, typename R_B, typename B = CleanType<R_B>,
	typename = std::enable_if_t< NoBools<A, B> && (EqualMat<A, B> || (IsScalar<A> || IsScalar<B>)) > >
	ArithmeticBinaryReturnType<A, B> operator/(R_A && a, R_B && b)
{
	return ArithmeticBinaryReturnType<A, B>(createExp(std::make_shared<FunctionOp<IN_BETWEEN, ARGS_PARENTHESIS>>("/"), getExp<R_A>(a), getExp<R_B>(b)));
}




//

template<typename T, uint N>
struct Array : NamedObject<typename T::UnderlyingType> {
	using Type = typename T::UnderlyingType;

	explicit Array(const std::string & _name = "") : NamedObject<Type>(_name) {
		NamedObjectBase::exp = createArrayDeclaration<Array,N>(NamedObjectBase::myNamePtr());
	}

	template<typename R_A, typename A = CleanType<R_A>, 
		typename = std::enable_if_t< IsInteger<A> > >
		Type operator[](R_A && a) const & {
		return Type(
			createExp(std::make_shared<FunctionOp<IN_FRONT, ARGS_BRACKETS>>(getExp<Array, false>(*this)->str()),
				getExp<R_A>(a)
			)
		);
	}

};