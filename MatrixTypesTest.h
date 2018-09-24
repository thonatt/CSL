#pragma once

#include "Algebra.h"
#include "ExpressionsTest.h"


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
	using NamedObjectBaseT::name;

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

	explicit MatrixT(const std::string & s = "") : NamedObjectT<MatrixT>(s) {
		exp = createInit<MatrixT>(name);
	}

	template <std::size_t N>
	explicit MatrixT(const char(&s)[N]) : NamedObjectT<MatrixT>(s) {
		exp = createInit<MatrixT>(name);
	}

	template<typename U, typename = std::enable_if_t<!isBool && (NR == 1 && NC == 1) && AreValid<U> && !Infos<U>::glsl_type > >
	MatrixT(const U & u, const std::string & s) : NamedObjectT<MatrixT>(s) {
		exp = createInit<MatrixT, HIDE, IN_FRONT, NO_PARENTHESIS>(name, getExp(u));
	}

	//matXY from matWZ
	template<numberType otype, unsigned int oNR, unsigned int oNC, typename O_MatrixT = MatrixT<otype, oNR, oNC>, typename = std::enable_if_t <
		(NC == 1 && NR == 1) || (!isBool && NC != 1 && NR != 1 && oNR != 1 && oNC != 1)
	> >
		explicit MatrixT(O_MatrixT && m) : NamedObjectT<MatrixT>() {
		checkForTemp< O_MatrixT >(m);
		exp = createInit<MatrixT>(name, getExp(m));
	}

	template<typename R_T, typename T = CleanType<R_T>, typename = std::enable_if_t< AreValid<T> && !Infos<T>::glsl_type > >
	MatrixT( R_T && x) : NamedObjectT<MatrixT>() {
		checkForTemp<T>(x);
		if ( EqualMat<MatrixT,T> ) {
			exp = createInit<MatrixT, HIDE, IN_FRONT, NO_PARENTHESIS>(name, getExp(x));
		} else {
			exp = createInit<MatrixT, DISPLAY>(name, getExp(x));
		}
	}


	MatrixT(MatrixT&& other) {
		std::cout << " CTOR &&" << std::endl;
	}
	MatrixT (const MatrixT& other) {
		std::cout << " CTOR const&" << std::endl;
	}

	MatrixT & operator=(const MatrixT& other) {
		//std::cout << " = const&" << std::endl;
		checkForTemp<MatrixT>(other);
		listen().addEvent(createExp(std::make_shared<SingleCharBinaryOp<'=', NO_PARENTHESIS>>(), createExp(std::make_shared<Alias>(name)), getExp(other)));
		return *this;
	}
	MatrixT & operator=( MatrixT&& other) {
		//std::cout << " = && " << std::endl;
		checkForTemp<MatrixT>(other);
		listen().addEvent(createExp(std::make_shared<SingleCharBinaryOp<'=', NO_PARENTHESIS>>(), createExp(std::make_shared<Alias>(name)), getExp(other)));
		return *this;
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
		exp = createInit<MatrixT>(name, getExp(u), getExp(v), getExp(us)...);
	}

	MatrixT(const Ex & _exp)  : NamedObjectT<MatrixT>() {
		//std::cout << " from exp " << std::endl;
		exp = createInit<MatrixT, HIDE, IN_FRONT, NO_PARENTHESIS>(name, _exp);
	}

	MatrixT(const TinitT<type,NR,NC> & t) : NamedObjectT<MatrixT>(t.name) {
		//std::cout << "from TiniT" << std::endl;
		exp = createInit<MatrixT, HIDE, IN_FRONT, NO_PARENTHESIS>(name, t.exp);
	}

	MatrixT & operator=(const TinitT < type, NR, NC> & other) = delete;


	///////////////////

	template<bool b = !isBool, typename = std::enable_if_t<b> >
	MatrixT operator++() const & {
		return MatrixT(createExp(std::make_shared<FunctionOp<IN_FRONT,NO_PARENTHESIS>>(" ++"), exp));
	}
	template<bool b = !isBool, typename = std::enable_if_t<b> >
	MatrixT operator++() const && {
		areNotInit(*this);
		return MatrixT(createExp(std::make_shared<FunctionOp<IN_FRONT, NO_PARENTHESIS>>(" ++"), exp));
	}

	template<bool b = !isBool, typename = std::enable_if_t<b> >
	MatrixT operator++(int) const &  {
		return MatrixT(createExp(std::make_shared<FunctionOp<BEHIND>>("++ "), exp));
	}
	template<bool b = !isBool, typename = std::enable_if_t<b> >
	MatrixT operator++(int) const && {
		areNotInit(*this);
		return MatrixT(createExp(std::make_shared<FunctionOp<BEHIND>>("++ "), exp));
	}


	template<bool b = !isBool, typename = std::enable_if_t<b> >
	MatrixT operator--() const & {
		return MatrixT(createExp(std::make_shared<FunctionOp<IN_FRONT, NO_PARENTHESIS>>(" --"), exp));
	}
	template<bool b = !isBool, typename = std::enable_if_t<b> >
	MatrixT operator--() const && {
		areNotInit(*this);
		return MatrixT(createExp(std::make_shared<FunctionOp<IN_FRONT, NO_PARENTHESIS>>(" --"), exp));
	}

	template<bool b = !isBool, typename = std::enable_if_t<b> >
	MatrixT operator--(int) const & {
		return MatrixT(createExp(std::make_shared<FunctionOp<BEHIND>>("-- "), exp));
	}
	template<bool b = !isBool, typename = std::enable_if_t<b> >
	MatrixT operator--(int) const && {
		areNotInit(*this);
		return MatrixT(createExp(std::make_shared<FunctionOp<BEHIND>>("-- "), exp));
	}

	template<bool b = !isBool, typename = std::enable_if_t<b> >
	MatrixT operator-() const & {
		return MatrixT(createExp(std::make_shared<FunctionOp<IN_FRONT, NO_PARENTHESIS>>("-"), exp));
	}
	template<bool b = !isBool, typename = std::enable_if_t<b> >
	MatrixT operator-() const && {
		areNotInit(*this);
		return MatrixT(createExp(std::make_shared<FunctionOp<IN_FRONT, NO_PARENTHESIS>>("-"), exp));

	}
	template<typename R_A, typename A = CleanType<R_A>,
		typename = std::enable_if_t< NotBool<A> && (EqualMat<MatrixT, A> || IsScalar<A>)  >  >
	void operator+=(R_A&& a) {
		checkForTemp<R_A>(a);
		listen().addEvent(createExp(std::make_shared<FunctionOp<IN_BETWEEN, NO_PARENTHESIS>>("+="), exp, getExp(a)));
	}






};


template<numberType type, unsigned int N, unsigned int M>
Ex getExp(const MatrixT<type, N, M> & m) {
	return m.exp;
}

template<numberType type, unsigned int NR, unsigned int NC>
TinitT<type,NR,NC> operator<<(const MatrixT<type,NR,NC> & m, const std::string & s) {
	areNotInit(m);
	return TinitT<type,NR,NC>(m, s);
}

template<numberType type, unsigned int NR, unsigned int NC>
TinitT<type, NR, NC>::TinitT(const MatrixT<type, NR, NC> & m, const std::string & s) : name(s), exp(m.exp) { }


// Bool operators
template<typename R_A, typename A = CleanType<R_A>, typename R_B, typename B = CleanType<R_B>,
	typename = std::enable_if_t< EqualMat<A, BoolT> &&  EqualMat<B, BoolT>  > >
BoolT operator&&(R_A && b1, R_B && b2) {
	checkForTemp<R_A, R_B>(b1, b2);
	return BoolT(createExp(std::make_shared<FunctionOp<IN_BETWEEN>>("&&"), getExp(b1), getExp(b2)));
}

template<typename R_A, typename A = CleanType<R_A>, typename R_B, typename B = CleanType<R_B>,
	typename = std::enable_if_t< EqualMat<A, BoolT> &&  EqualMat<B, BoolT>  > >
	BoolT operator||(R_A && b1, R_B && b2) {
	checkForTemp<R_A, R_B>(b1, b2);
	return BoolT(createExp(std::make_shared<FunctionOp<IN_BETWEEN>>("||"), getExp(b1), getExp(b2)));
}

template<typename R_A, typename A = CleanType<R_A>, typename R_B, typename B = CleanType<R_B>, 
	typename = std::enable_if_t< NoBools<A, B> && IsScalar<A> && IsScalar<B> > >
BoolT operator<(R_A && a, R_B && b) {
	checkForTemp<R_A, R_B>(a, b);
	return BoolT(createExp(std::make_shared<FunctionOp<IN_BETWEEN, NO_PARENTHESIS>>("<"), getExp(a), getExp(b)));
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
