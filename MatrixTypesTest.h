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
	~MatrixT() {
		if (auto ctor = std::dynamic_pointer_cast<CtorBase>(exp->op)) {
			MainListener::overmind.addEvent(createExp(std::make_shared<DtorBase>(), exp));
		} else {
			std::cout << " !!!!  " << std::endl;
		}
	}

	explicit MatrixT(const std::string & s = "") : NamedObjectT<MatrixT>(s) {
		exp = createInit<MatrixT>(name);
	}

	template <std::size_t N>
	explicit MatrixT(const char(&s)[N]) : NamedObjectT<MatrixT>(s) {
		exp = createInit<MatrixT>(name);
	}

	template<typename U, typename = std::enable_if_t<!isBool && (NR == 1 && NC == 1) && AreValid<U> && !Infos<U>::glsl_type > >
	MatrixT(const U & u, const std::string & s) : NamedObjectT<MatrixT>(s) {
		exp = createInit<MatrixT,NONE, NO_PARENTHESIS>(name, getExp(u));
	}

	//matX from matY
	template<numberType otype, unsigned int oNR, unsigned int oNC, typename = std::enable_if_t < 
		!isBool && ( (NC == 1 && NR == 1 ) || (NC != 1 && NR != 1 && oNR != 1 && oNC != 1 ) )
	> >
	explicit MatrixT(const MatrixT<otype, oNR, oNC>& m) : NamedObjectT<MatrixT>() {
		exp = createInit<MatrixT>(name, getExp(m));
	}

	//template<typename U, typename = std::enable_if_t < AreValid<U> && MatElements<U> == 1 > >
	//explicit MatrixT(const U & u) : NamedObjectT<MatrixT>() {
	//	//std::cout << "multictor " << std::endl; 
	//	if (NC == 1 && NR == 1 && Infos<U>::scalar_type < type) {
	//		exp = createInit<MatrixT,NONE,NO_PARENTHESIS>(name, getExp(u));
	//	} else {
	//		exp = createInit<MatrixT>(name, getExp(u));
	//	}
	//}

	template<bool b = !isBool && type >= numberType::INT, typename = std::enable_if_t <b> >
	//explicit
	MatrixT(const int & i) : NamedObjectT<MatrixT>() {
		if (type == numberType::INT) {
			exp = createInit<MatrixT, NONE, NO_PARENTHESIS>(name, getExp(i));
		} else {
			exp = createInit<MatrixT>(name, getExp(i));
		}
	}

	template<bool b = !isBool &&  type >= numberType::FLOAT, typename = std::enable_if_t <b> >
	//explicit 
	MatrixT(const double & d) : NamedObjectT<MatrixT>() {
		if (type == numberType::FLOAT) {
			exp = createInit<MatrixT, NONE, NO_PARENTHESIS>(name, getExp(d));
		} else {
			exp = createInit<MatrixT>(name, getExp(d));
		}
	}

	template<bool b = isBool && NR == 1 && NC == 1, typename = std::enable_if_t <b> >
	MatrixT(const bool & bo) : NamedObjectT<MatrixT>() {
		if (type == numberType::BOOL) {
			exp = createInit<MatrixT, NONE, NO_PARENTHESIS>(name, getExp(bo));
		} else {
			exp = createInit<MatrixT>(name, getExp(bo));
		}

	}
	MatrixT & operator=(const MatrixT& other) {
		//std::cout << " op = " << std::endl;
		isNotInit(other.exp);
		listen().addEvent(createExp(std::make_shared<SingleCharBinaryOp<'=', NO_PARENTHESIS>>(), createExp(std::make_shared<Alias>(name)), other.exp));
		return *this;
	}

	//template<typename U, typename = std::enable_if_t <
	//	EqualDim<MatrixT, U> && (Infos<U>::scalar_type <= type)
	//> >
	//MatrixT & operator=(const std::conditional_t< EqualDim<MatrixT, U> && (Infos<U>::scalar_type < type), U,int>  & u) {
	//	std::cout << "here " << std::endl;
	//	listen().addEvent(createExp(std::make_shared<SingleCharBinaryOp<'=', NO_PARENTHESIS>>(), createExp(std::make_shared<Alias>(name)), getExp(u)));
	//	return *this;
	//}

	template<typename U, typename = std::enable_if_t < 
		!EqualDim<MatrixT,U> ||
		(Infos<U>::scalar_type > type)
	> >
	MatrixT & operator=(const  U & u) = delete;

	template<typename U, typename V, typename ...Us, typename = std::enable_if_t < 
		AreValid<U, V, Us...> && MatElements<U, V, Us...> == NR * NC /*&& SameScalarType<MatrixT,U,V,Us...> */ > >
	explicit MatrixT(const U & u, const V & v, const Us & ...us) : NamedObjectT<MatrixT>() {
		//std::cout << "multictor " << std::endl; 
		exp = createInit<MatrixT>(name, getExp(u), getExp(v), getExp(us)...);
	}

	//
	//if (sizeof...(us) == 0 && !Infos<U>::glsl_type) {
	//	exp = createInit<MatrixT, NONE, NO_PARENTHESIS>(name, getExp(u));
	//} else {
	//
	//////


	MatrixT(const Ex & _exp) : NamedObject<MatrixT>() {
		exp = createInit<MatrixT, NONE, NO_PARENTHESIS>(name, _exp);
	}

	MatrixT(const TinitT<type,NR,NC> & t) : NamedObjectT<MatrixT>(t.name) {
		exp = createInit<MatrixT, NONE, NO_PARENTHESIS>(name, t.exp);
	}

	MatrixT & operator=(const TinitT < type, NR, NC> & other) = delete;

};

template<numberType type, unsigned int NR, unsigned int NC>
TinitT<type,NR,NC> operator<<(const MatrixT<type,NR,NC> & m, const std::string & s) {
	return TinitT<type,NR,NC>(m, s);
}

template<numberType type, unsigned int NR, unsigned int NC>
TinitT<type, NR, NC>::TinitT(const MatrixT<type, NR, NC> & m, const std::string & s) : name(s), exp(m.exp) { }


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

template<numberType type, unsigned int N, unsigned int M>  
Ex getExp(const MatrixT<type, N, M> & m) {
	return m.exp;
}