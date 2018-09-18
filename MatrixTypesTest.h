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

public:
	~MatrixT() {
		if (auto ctor = std::dynamic_pointer_cast<CtorBase>(exp->op)) {
			MainListener::overmind.addEvent(createExp(std::make_shared<DtorBase>(), exp));
		} else {
			std::cout << " !!!!  " << std::endl;
		}
	}

	MatrixT(const std::string & s = "") : NamedObjectT<MatrixT>(s) {
		exp = createInit<MatrixT>(name);
	}

	template <std::size_t N>
	MatrixT(const char(&s)[N]) : NamedObjectT<MatrixT>(s) {
		exp = createInit<MatrixT>(name);
	}


	template<typename U, typename = std::enable_if_t<(NR == 1 && NC == 1) && AreValid<U> && !Infos<U>::glsl_type > >
	MatrixT(const U & u, const std::string & s) : NamedObjectT<MatrixT>(s) {
		exp = createInit<MatrixT,NONE, NO_PARENTHESIS>(name, getExp(u));
	}

	template<typename U, typename ...Us, typename = std::enable_if_t<AreValid<U, Us...> && MatElements<U,Us...> ==  NR*NC> >
	MatrixT(const U & u, const Us & ...us) : NamedObjectT<MatrixT>() {
		//std::cout << "multictor " << std::endl; 
		if (sizeof...(us) == 0 && !Infos<U>::glsl_type ) {
			exp = createInit<MatrixT,NONE,NO_PARENTHESIS>(name, getExp(u), getExp(us)...);
		} else {
			exp = createInit<MatrixT>(name, getExp(u), getExp(us)...);
		}
		
	}

	//////


	MatrixT(const Ex & _exp) : NamedObject<MatrixT>() {
		exp = createInit<MatrixT, NONE, NO_PARENTHESIS>(name, _exp);
	}

	MatrixT(const TinitT<type,NR,NC> & t) : NamedObjectT<MatrixT>(t.name) {
		exp = createInit<MatrixT, NONE, NO_PARENTHESIS>(name, t.exp);
	}
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