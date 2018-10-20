#pragma once

#include "Algebra.h"
#include "ExpressionsTest.h"

template<numberType type, unsigned int Nrows, unsigned int Ncols>
struct Infos<MatrixT<type, Nrows, Ncols>> {
	static const bool numeric_type = true;
	static const bool glsl_type = false;
	static const unsigned int rows = Nrows;
	static const unsigned int cols = Ncols;
	static const numberType scalar_type = type;
};

template<typename ...Ts> struct MatElementsT;
template<typename ...Ts> constexpr unsigned int MatElements = MatElementsT<Ts...>::value;

template<> struct MatElementsT<> {
	static const unsigned int value = 0;
};

template<typename T, typename ...Ts> struct MatElementsT<T,Ts...> {
	static const unsigned int value = NumElements<T> + MatElements<Ts...>;
};


template<numberType type, unsigned int Nrows, unsigned int Ncols>
struct TinitT {
	TinitT(const MatrixT<type,Nrows,Ncols> & m, const std::string & s);

	std::string name;
	Ex exp;
};

/// matrix class

template<numberType type, unsigned int NR, unsigned int NC>
class MatrixT : public NamedObjectT<MatrixT<type, NR, NC>> {
public:
	using NamedObjectBaseT::exp;
	using NamedObjectBaseT::namePtr;
	using NamedObjectBaseT::isUsed;

protected:
	static const bool isBool = (type == numberType::BOOL);

public:
	//~MatrixT() {
	//	if (auto ctor = std::dynamic_pointer_cast<CtorBase>(exp->op)) {
	//		MainListener::overmind.addEvent(createExp(std::make_shared<DtorBase>(), exp));
	//	} else {
	//		std::cout << " !!!!  " << std::endl;
	//	}
	//}

	MatrixT(Matrix_Track track, const std::string & _name = "") : NamedObjectT<MatrixT>(_name) {
		exp = createDeclaration<MatrixT>(NamedObjectBaseT::myNamePtr());
		areNotInit(*this);
		
	}

	static const std::string typeStr() { return TypeStrT<MatrixT>::str(); }

	explicit MatrixT(const std::string & _name = "") : NamedObjectT<MatrixT>(_name) {
		exp = createDeclaration<MatrixT>(NamedObjectBaseT::myNamePtr());
		
	}

	explicit MatrixT(const std::string & _name, NamedObjectBaseT * _parent, bool _isUsed = true) : NamedObjectT<MatrixT>(_name,_parent, _isUsed) {
		exp = createDeclaration<MatrixT>(NamedObjectBaseT::myNamePtr());
		if (_parent) {
			//std::static_pointer_cast<CtorBase>(exp->op)->firstStr = false;
			areNotInit(*this);
		}
	}

	template <std::size_t N>
	explicit MatrixT(const char(&s)[N]) : NamedObjectT<MatrixT>(s) {
		exp = createDeclaration<MatrixT>(NamedObjectBaseT::myNamePtr());
		
	}

	template<typename U, typename = std::enable_if_t<!isBool && (NR == 1 && NC == 1) && AreValid<U> && !Infos<U>::glsl_type > >
	MatrixT( U && u, const std::string & s) : NamedObjectT<MatrixT>(s) {
		exp = createInit<MatrixT, HIDE, IN_FRONT, NO_PARENTHESIS>(namePtr, getExp<U>(u));
	}

	// matXY(int/float) and matXY(matWZ)
	template<typename R_T, typename T = CleanType<R_T>, typename = std::enable_if_t< 
		AreValid<T> && (
			!Infos<T>::glsl_type  || //matXY(int/float)
		(!isBool && NC != 1 && NR != 1 && Infos<T>::cols != 1 && Infos<T>::rows != 1) //matXY(matWZ)
			) > >
	MatrixT( R_T && x) : NamedObjectT<MatrixT>() {
		
		checkForTemp<R_T>(x);
		if ( EqualMat<MatrixT,T> ) {
			exp = createInit<MatrixT, HIDE, IN_FRONT, NO_PARENTHESIS>(namePtr, getExp<R_T>(x));
		} else {
			exp = createInit<MatrixT, DISPLAY, IN_FRONT, USE_PARENTHESIS>(namePtr, getExp<R_T>(x));
		}
	}

	/////////////////////////////////////////////////////
	// Used for tuples, need to figure out why
	/////////////////////////////////////////////////////

	MatrixT(MatrixT&& other) : NamedObjectT<MatrixT>(other) {
		//parent = other.parent;
		//exp = other.exp;
		//std::static_pointer_cast<CtorBase>(exp->op)->firstStr = false;
		//std::cout << " CTOR &&" << std::endl;
		
	}

	MatrixT (const MatrixT& other) : NamedObjectT<MatrixT>(other) {
		//parent = other.parent;
		//exp = other.exp;
		//std::static_pointer_cast<CtorBase>(exp->op)->firstStr = false;
		//std::cout << " CTOR const&" << std::endl;
		
	}
	/////////////////////////////////////////////////////

	void operator=(const MatrixT& other) {
		//std::cout << " = const&" << other.name << std::endl;
		checkForTemp<MatrixT>(other);
		listen().addEvent(createExp(std::make_shared<SingleCharBinaryOp<'=', NO_PARENTHESIS>>(), createExp(std::make_shared<Alias>(NamedObjectBaseT::myNamePtr())), getExp<const MatrixT &>(other)));
		//return *this;
	}
	void operator=( MatrixT&& other) {
		//std::cout << " = && " << other.name << std::endl;
		checkForTemp<MatrixT&&>(other);
		listen().addEvent(createExp(std::make_shared<SingleCharBinaryOp<'=', NO_PARENTHESIS>>(), createExp(std::make_shared<Alias>(NamedObjectBaseT::myNamePtr())), getExp<MatrixT &&>(other)));
		//return *this;
	}

	template<typename U, typename = std::enable_if_t < 
		!EqualDim<MatrixT,U> ||
		(Infos<U>::scalar_type > type)
	> >
	MatrixT & operator=(const  U & u) = delete;

	template<typename R_U, typename R_V, typename U = CleanType<R_U>, typename V = CleanType<R_V>, typename ...R_Us,
		typename = std::enable_if_t < AreValid<U, V, CleanType<R_Us>... > && MatElements<U, V, CleanType<R_Us>... > == NR * NC > >
	explicit MatrixT(R_U && u, R_V && v, R_Us && ...us) : NamedObjectT<MatrixT>() {
		//std::cout << "multictor " << std::endl; 
		//areNotInit(u, v, us...);
		checkForTemp<R_U, R_V, R_Us...>(u, v, us...);
		exp = createInit<MatrixT>(namePtr, getExp<R_U>(u), getExp<R_V>(v), getExp<R_Us>(us)...);
	}

	MatrixT(const Ex & _exp)  : NamedObjectT<MatrixT>() {
		//std::cout << " from exp " << std::endl;
		exp = createInit<MatrixT, HIDE, IN_FRONT, NO_PARENTHESIS>(namePtr, _exp);
		isUsed = false;
	}

	MatrixT(const TinitT<type,NR,NC> & t) : NamedObjectT<MatrixT>(t.name) {
		//std::cout << "from TiniT" << std::endl;
		exp = createInit<MatrixT, HIDE, IN_FRONT, NO_PARENTHESIS>(namePtr, t.exp);
		//std::cout << std::dynamic_pointer_cast<CtorBase>(exp->op)->status << t.exp->str() <<   std::endl;
	}

	MatrixT & operator=(const TinitT < type, NR, NC> & other) = delete;


	///////////////////

	template<bool b = !isBool, typename = std::enable_if_t<b> >
	MatrixT operator++() const & {
		return MatrixT(createExp(std::make_shared<FunctionOp<IN_FRONT,NO_PARENTHESIS>>("++"), getExp<MatrixT,false>(*this)));
	}
	template<bool b = !isBool, typename = std::enable_if_t<b> >
	MatrixT operator++() const && {
		return MatrixT(createExp(std::make_shared<FunctionOp<IN_FRONT, NO_PARENTHESIS>>("++"), getExp<MatrixT, true>(*this)));
	}

	template<bool b = !isBool, typename = std::enable_if_t<b> >
	MatrixT operator++(int) const &  {
		return MatrixT(createExp(std::make_shared<FunctionOp<BEHIND>>("++ "), getExp<MatrixT, false>(*this)));
	}
	template<bool b = !isBool, typename = std::enable_if_t<b> >
	MatrixT operator++(int) const && {
		return MatrixT(createExp(std::make_shared<FunctionOp<BEHIND>>("++ "), getExp<MatrixT, true>(*this)));
	}


	template<bool b = !isBool, typename = std::enable_if_t<b> >
	MatrixT operator--() const & {
		return MatrixT(createExp(std::make_shared<FunctionOp<IN_FRONT, NO_PARENTHESIS>>(" --"), getExp<MatrixT, false>(*this)));
	}
	template<bool b = !isBool, typename = std::enable_if_t<b> >
	MatrixT operator--() const && {
		areNotInit(*this);
		return MatrixT(createExp(std::make_shared<FunctionOp<IN_FRONT, NO_PARENTHESIS>>(" --"), getExp<MatrixT, true>(*this)));
	}

	template<bool b = !isBool, typename = std::enable_if_t<b> >
	MatrixT operator--(int) const & {
		return MatrixT(createExp(std::make_shared<FunctionOp<BEHIND>>("-- "), getExp<MatrixT, false>(*this)));
	}
	template<bool b = !isBool, typename = std::enable_if_t<b> >
	MatrixT operator--(int) const && {
		areNotInit(*this);
		return MatrixT(createExp(std::make_shared<FunctionOp<BEHIND>>("-- "), getExp<MatrixT, true>(*this)));
	}

	template<bool b = !isBool, typename = std::enable_if_t<b> >
	MatrixT operator-() const & {
		return MatrixT(createExp(std::make_shared<FunctionOp<IN_FRONT, NO_PARENTHESIS>>("-"), getExp<MatrixT, false>(*this)));
	}
	template<bool b = !isBool, typename = std::enable_if_t<b> >
	MatrixT operator-() const && {
		areNotInit(*this);
		return MatrixT(createExp(std::make_shared<FunctionOp<IN_FRONT, NO_PARENTHESIS>>("-"), getExp<MatrixT, true>(*this)));

	}

	template<typename R_A, typename A = CleanType<R_A>,
		typename = std::enable_if_t< NotBool<A> && (EqualMat<MatrixT, A> || IsScalar<A>)  >  >
	void operator+=(R_A&& a) const & {
		checkForTemp<R_A>(a);
		listen().addEvent(createExp(std::make_shared<FunctionOp<IN_BETWEEN, NO_PARENTHESIS>>("+="), getExp<MatrixT, false>(*this), getExp<R_A>(a)));
	}

	template<typename R_A, typename A = CleanType<R_A>,
		typename = std::enable_if_t< NotBool<A> && (EqualMat<MatrixT, A> || IsScalar<A>)  >  >
		void operator+=(R_A&& a) const && = delete;

	template<bool b = isBool && NC == 1 && NR == 1, typename = std::enable_if_t<b> >
	operator bool() const {
		return false;
	}



};

//template<typename R_B, typename>
//void IfController::begin_if(R_B && b) {
//	current_if = std::make_shared<IfInstruction>();
//	current_if->bodies.push_back({ std::make_shared<Block>(currentBlock), std::make_shared<Statement>(getExp<R_B>(b)) });
//	currentBlock->instructions.push_back(std::static_pointer_cast<InstructionBase>(current_if));
//	currentBlock = current_if->bodies.back().body;
//	waiting_for_else = false;
//}

//template<typename R_B, typename>
//void IfController::begin_else_if(R_B && b) {
//	current_if->bodies.push_back({ std::make_shared<Block>(currentBlock->parent), std::make_shared<Statement>(getExp<R_B>(b)) });
//	currentBlock = current_if->bodies.back().body;
//	waiting_for_else = false;
//}



template<typename A, typename B> using ArithmeticBinaryReturnTypeT = MatrixT< MaxType<A, B>, MaxRow<A, B>, MaxCol<A, B> >;
//
//template<numberType type, unsigned int N, unsigned int M>
//Ex getExp(const MatrixT<type, N, M> & m) {
//	if (auto ctor = std::dynamic_pointer_cast<CtorBase>(m.exp->op)) {
//		std::cout << m.myName() << " " << m.exp.use_count() << std::endl;
//	} else {
//		std::cout << "not ctor" << m.exp.use_count() << std::endl;
//	}
//	return m.exp;
//}

//template<numberType type, unsigned int N, unsigned int M>
//Ex getExp( const MatrixT<type, N, M> & m) {
//	std::cout << "ref get exp " <<  m.myName() << " " << m.exp.use_count() << std::endl;
//	return m.exp;
//}

//
//
//template<numberType type, unsigned int N, unsigned int M>
//Ex getExp(MatrixT<type, N, M> m) {
//	std::cout << "get exp temp " << m.myName() << std::endl;
//	return m.exp;
//}

template<numberType type, unsigned int NR, unsigned int NC>
TinitT<type,NR,NC> operator<<(const MatrixT<type,NR,NC> & m, const std::string & s) {
	//areNotInit(m); 
	return TinitT<type,NR,NC>(m, s);
}

//template<numberType type, unsigned int NR, unsigned int NC>
//TinitT<type, NR, NC> operator<<(MatrixT<type, NR, NC> && m, const std::string & s) {
//	//areNotInit(m);
//	return TinitT<type, NR, NC>(std::forward< MatrixT<type, NR, NC>>(m), s);
//}

template<numberType type, unsigned int NR, unsigned int NC>
TinitT<type, NR, NC>::TinitT(const MatrixT<type, NR, NC> & m, const std::string & s) : name(s), exp(getExp< const MatrixT<type, NR, NC> & >(m)) { 
	//std::cout << "TINIT" << std::endl; 
}
//
//template<numberType type, unsigned int NR, unsigned int NC>
//TinitT<type, NR, NC>::TinitT(MatrixT<type, NR, NC> && m, const std::string & s) : name(s), exp(getExp<MatrixT<type, NR, NC> &&>(m)) {
//	std::cout << "TINIT &&" << std::endl;
//}


// Bool operators
template<typename R_A, typename A = CleanType<R_A>, typename R_B, typename B = CleanType<R_B>,
	typename = std::enable_if_t< EqualMat<A, BoolT> &&  EqualMat<B, BoolT>  > >
BoolT operator&&(R_A && b1, R_B && b2) {
	checkForTemp<R_A, R_B>(b1, b2);
	return BoolT(createExp(std::make_shared<FunctionOp<IN_BETWEEN>>("&&"), getExp<R_A>(b1), getExp<R_B>(b2)));
}

template<typename R_A, typename A = CleanType<R_A>, typename R_B, typename B = CleanType<R_B>,
	typename = std::enable_if_t< EqualMat<A, BoolT> &&  EqualMat<B, BoolT>  > >
	BoolT operator||(R_A && b1, R_B && b2) {
	checkForTemp<R_A, R_B>(b1, b2);
	return BoolT(createExp(std::make_shared<FunctionOp<IN_BETWEEN>>("||"), getExp<R_A>(b1), getExp<R_B>(b2)));
}

template<typename R_A, typename A = CleanType<R_A>, typename R_B, typename B = CleanType<R_B>, 
	typename = std::enable_if_t< NoBools<A, B> && IsScalar<A> && IsScalar<B> > >
BoolT operator<(R_A && a, R_B && b) {
	checkForTemp<R_A, R_B>(a, b);
	return BoolT(createExp(std::make_shared<FunctionOp<IN_BETWEEN, NO_PARENTHESIS>>("<"), getExp<R_A>(a), getExp<R_B>(b)));
}

template<typename R_A, typename A = CleanType<R_A>, typename R_B, typename B = CleanType<R_B>, 
	typename = std::enable_if_t< NoBools<A, B> && (EqualMat<A, B> || /* EqualType<A,B> && */ ( IsScalar<A> || IsScalar<B> ) ) > >
ArithmeticBinaryReturnTypeT<A, B> operator+(R_A && a, R_B && b) {
	checkForTemp<R_A, R_B>(a, b);
	return ArithmeticBinaryReturnTypeT<A, B>(createExp(std::make_shared<FunctionOp<IN_BETWEEN,NO_PARENTHESIS>>("+"), getExp<R_A>(a), getExp<R_B>(b)));
}

template<> struct TypeStrT<void> {
	static const std::string str() { return "void"; }
};

template<numberType nType> struct nTypeStrT;

template<> struct nTypeStrT<numberType::BOOL> {
	static const std::string str() { return "b"; }
};
template<> struct nTypeStrT<numberType::INT> {
	static const std::string str() { return "i"; }
};
template<> struct nTypeStrT<numberType::UINT> {
	static const std::string str() { return "u"; }
};
template<> struct nTypeStrT<numberType::FLOAT> {
	static const std::string str() { return ""; }
};
template<> struct nTypeStrT<numberType::DOUBLE> {
	static const std::string str() { return "d"; }
};

template<> struct TypeStrT<BoolT> {
	static const std::string str() { return "bool"; }
};

template<> struct TypeStrT<IntT> {
	static const std::string str() { return "int"; }
};

template<> struct TypeStrT<UintT> {
	static const std::string str() { return "uint"; }
};

template<> struct TypeStrT<FloatT> {
	static const std::string str() { return "float"; }
};

template<> struct TypeStrT<DoubleT> {
	static const std::string str() { return "double"; }
};


template<numberType type, unsigned int N> struct TypeStrT< VecT<type, N> > {
	static const std::string str() { return nTypeStrT<type>::str() + "vec" + std::to_string(N); }
};

template<numberType type, unsigned int N, unsigned int M> struct TypeStrT< MatrixT<type, N, M> > {
	static const std::string str() {
		return N == M ?
			nTypeStrT<type>::str() + "mat" + std::to_string(N)
			:
			nTypeStrT<type>::str() + "mat" + std::to_string(N) + "x" + std::to_string(M);
	}
};
