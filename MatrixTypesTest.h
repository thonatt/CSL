#pragma once

//#include "Algebra.h"

#include "ExpressionsTest.h"

template<ScalarType type, uint Nrows, uint Ncols>
struct MatrixInit {
	MatrixInit(const Matrix<type,Nrows,Ncols> & m, const std::string & s);

	std::string name;
	Ex exp;
};

/// matrix class

template<ScalarType type, uint NR, uint NC>
class Matrix : public NamedObject<Matrix<type, NR, NC>> {
public:
	using NamedObjectBase::exp;
	using NamedObjectBase::namePtr;
	using NamedObjectBase::isUsed;

protected:
	static const bool isBool = (type == BOOL);
	static const bool isScalar = (NC == 1 && NR == 1);
public:
	//~Matrix() {
	//	if (auto ctor = std::dynamic_pointer_cast<CtorBase>(exp->op)) {
	//		MainListener::overmind.addEvent(createExp(std::make_shared<DtorBase>(), exp));
	//	} else {
	//		std::cout << " !!!!  " << std::endl;
	//	}
	//}

	Matrix(Matrix_Track track, const std::string & _name = "") : NamedObject<Matrix>(_name) {
		exp = createDeclaration<Matrix>(NamedObjectBase::myNamePtr());
		areNotInit(*this);
		
	}

	static const std::string typeStr() { return TypeStr<Matrix>::str(); }

	explicit Matrix(const std::string & _name = "") : NamedObject<Matrix>(_name) {
		exp = createDeclaration<Matrix>(NamedObjectBase::myNamePtr());
		
	}

	explicit Matrix(const std::string & _name, NamedObjectBase * _parent, bool _isUsed = true) : NamedObject<Matrix>(_name,_parent, _isUsed) {
		exp = createDeclaration<Matrix>(NamedObjectBase::myNamePtr());
		if (_parent) {
			//std::static_pointer_cast<CtorBase>(exp->op)->firstStr = false;
			areNotInit(*this);
		}
	}

	template <std::size_t N>
	explicit Matrix(const char(&s)[N]) : NamedObject<Matrix>(s) {
		exp = createDeclaration<Matrix>(NamedObjectBase::myNamePtr());
	}

	template<typename U, typename = std::enable_if_t<!isBool && (NR == 1 && NC == 1) && AreValid<U> && !Infos<U>::is_glsl_type > >
	Matrix( U && u, const std::string & s) : NamedObject<Matrix>(s) {
		exp = createInit<Matrix, HIDE, IN_FRONT, NO_PARENTHESIS>(namePtr, getExp<U>(u));
	}

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

	Matrix(Matrix&& other) : NamedObject<Matrix>(other) {
		//parent = other.parent;
		//exp = other.exp;
		//std::static_pointer_cast<CtorBase>(exp->op)->firstStr = false;
		//std::cout << " CTOR &&" << std::endl;
		
	}

	Matrix (const Matrix& other) : NamedObject<Matrix>(other) {
		//parent = other.parent;
		//exp = other.exp;
		//std::static_pointer_cast<CtorBase>(exp->op)->firstStr = false;
		//std::cout << " CTOR const&" << std::endl;
		
	}
	/////////////////////////////////////////////////////

	void operator=(const Matrix& other) {
		//std::cout << " = const&" << other.name << std::endl;
		//checkForTemp<Matrix>(other);
		listen().addEvent(createExp(std::make_shared<SingleCharBinaryOp<'=', NO_PARENTHESIS>>(), 
			createExp(std::make_shared<Alias>(NamedObjectBase::myNamePtr())),
			getExp<Matrix,false>(other))
		);
		//return *this;
	}
	void operator=( Matrix&& other) {
		//std::cout << " = && " << other.name << std::endl;
		//checkForTemp<Matrix&&>(other);
		listen().addEvent(createExp(std::make_shared<SingleCharBinaryOp<'=', NO_PARENTHESIS>>(),
			createExp(std::make_shared<Alias>(NamedObjectBase::myNamePtr())),
			getExp<Matrix,true>(other))
		);
		//return *this;
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

	template<typename R_A, typename A = CleanType<R_A>,
		typename = std::enable_if_t< NotBool<A> && (EqualMat<Matrix, A> || IsScalar<A>)  >  >
	void operator+=(R_A&& a) const & {
		//checkForTemp<R_A>(a);
		listen().addEvent(createExp(std::make_shared<FunctionOp<IN_BETWEEN, NO_PARENTHESIS>>("+="), getExp<Matrix, false>(*this), getExp<R_A>(a)));
	}

	template<typename R_A, typename A = CleanType<R_A>,
		typename = std::enable_if_t< NotBool<A> && (EqualMat<Matrix, A> || IsScalar<A>)  >  >
		void operator+=(R_A&& a) const && = delete;

	template<bool b = isBool && NC == 1 && NR == 1, typename = std::enable_if_t<b> >
	operator bool() const {
		return false;
	}



};

template<ScalarType type, unsigned int NR, unsigned int NC>
MatrixInit<type,NR,NC> operator<<(const Matrix<type,NR,NC> & m, const std::string & s) {
	return MatrixInit<type,NR,NC>(m, s);
}

//template<numberType type, unsigned int NR, unsigned int NC>
//TinitT<type, NR, NC> operator<<(Matrix<type, NR, NC> && m, const std::string & s) {
//	//areNotInit(m);
//	return TinitT<type, NR, NC>(std::forward< Matrix<type, NR, NC>>(m), s);
//}

template<ScalarType type, unsigned int NR, unsigned int NC>
MatrixInit<type, NR, NC>::MatrixInit(const Matrix<type, NR, NC> & m, const std::string & s) 
	: name(s), exp(getExp< const Matrix<type, NR, NC> & >(m)) {
}

//
//template<numberType type, unsigned int NR, unsigned int NC>
//TinitT<type, NR, NC>::TinitT(Matrix<type, NR, NC> && m, const std::string & s) : name(s), exp(getExp<Matrix<type, NR, NC> &&>(m)) {
//	std::cout << "TINIT &&" << std::endl;
//}


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
	typename = std::enable_if_t< NoBools<A, B> && IsScalar<A> && IsScalar<B> > >
Bool operator<(R_A && a, R_B && b) 
{
	return Bool(createExp(std::make_shared<FunctionOp<IN_BETWEEN, NO_PARENTHESIS>>("<"), getExp<R_A>(a), getExp<R_B>(b)));
}

template<typename R_A, typename A = CleanType<R_A>, typename R_B, typename B = CleanType<R_B>, 
	typename = std::enable_if_t< NoBools<A, B> && (EqualMat<A, B> || /* EqualType<A,B> && */ ( IsScalar<A> || IsScalar<B> ) ) > >
ArithmeticBinaryReturnType<A, B> operator+(R_A && a, R_B && b)
{
	return ArithmeticBinaryReturnType<A, B>(createExp(std::make_shared<FunctionOp<IN_BETWEEN,NO_PARENTHESIS>>("+"), getExp<R_A>(a), getExp<R_B>(b)));
}