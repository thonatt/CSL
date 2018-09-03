#pragma once

#include "Context.h"
#include "HelperClasses.h"
#include <type_traits>

enum class SwizzleSet { RGBA, XYZW, STPQ, MIXED_SET };

template<bool b, SwizzleSet S1, SwizzleSet S2> struct WhatSwizzleSetT {
	static const SwizzleSet set = S1;
};

template<SwizzleSet S1, SwizzleSet S2> struct WhatSwizzleSetT<false, S1, S2> {
	static const SwizzleSet set = S2;
};

template<bool b, SwizzleSet S1, SwizzleSet S2> constexpr SwizzleSet WhatSet = WhatSwizzleSetT<b, S1, S2>::set;

template<unsigned int Dim, unsigned int Size, SwizzleSet Set, unsigned int Bytes, bool Repeated> class SwizzlePack {
public:
	SwizzlePack(const std::string & c) : s(c) { }

	template<unsigned int D, unsigned int S, SwizzleSet OtherSet, unsigned int otherBytes, bool otherRepeated >
	SwizzlePack<MaxOfV<Dim, D>, Size + S, WhatSet<Set == OtherSet, Set, SwizzleSet::MIXED_SET>, Bytes ^ otherBytes, Repeated || ((Bytes& otherBytes) != 0 ) >
	operator,(const SwizzlePack<D, S, OtherSet, otherBytes, otherRepeated> & other) const {
		return SwizzlePack<MaxOfV<Dim, D>, Size + S, WhatSet<Set == OtherSet, Set, SwizzleSet::MIXED_SET>, Bytes ^ otherBytes, Repeated || ((Bytes& otherBytes) != 0) >(s + other.s);
	}

	void cout() const { std::cout << s << std::endl; }

public:
	const std::string s;
};

template<typename T> const std::string strFromType() { return T::typeStr(); }
template<typename T> const std::string strFromType(const T &) { return T::typeStr(); }

template<typename ... Ts> const std::string strFromObj(const Ts & ... o);

template<typename T> const std::string strFromObj(const T & o) { return getName(o); }
template<> const std::string strFromObj(const double & d) { return std::to_string(d); }

template<typename TA, typename TB, typename ... Ts>
const std::string strFromObj(const TA & ta, const TB & tb, const Ts & ... ts) {
	return strFromObj(ta) + ", " + strFromObj(tb, ts...);
}

template<typename ...Ts> struct StrFromDecl;
template<> struct StrFromDecl<> {
	static const std::string get() { return ""; }
};
template<typename T> struct StrFromDecl<T> {
	static const std::string get(const T & o) { return strFromType(o) + " " + strFromObj(o); }
};
template<typename TA, typename TB, typename ... Ts> struct StrFromDecl<TA,TB,Ts...> {
	static const std::string get(const TA & ta, const TB & tb, const Ts & ... ts) {
		return StrFromDecl<TA>::get(ta) + ", " + StrFromDecl<TB,Ts...>::get(tb, ts...);
	}
};


template<typename T> const std::string strFromDecl(const T & o) { return strFromType<T>(o) + " " + strFromObj<T>(o); }

template<typename ...Ts> const std::string strFromDecl(const Ts & ... ts) {
	return StrFromDecl<Ts...>::get(ts...);
}

template<typename ...Ts> const std::string strFromDecl(const std::tuple<Ts...> & vs) {
	std::function<const std::string(Ts...)> f = StrFromDecl<Ts...>::get;
	return call_from_tuple(f, vs);
}

const SwizzlePack<1, 1, SwizzleSet::RGBA, 1, false> r("r");
const SwizzlePack<2, 1, SwizzleSet::RGBA, 2, false> g("g");
const SwizzlePack<3, 1, SwizzleSet::RGBA, 4, false> b("b");
const SwizzlePack<4, 1, SwizzleSet::RGBA, 8, false> a("a");

const SwizzlePack<1, 1, SwizzleSet::XYZW, 1, false> x("x");
const SwizzlePack<2, 1, SwizzleSet::XYZW, 2, false> y("y");
const SwizzlePack<3, 1, SwizzleSet::XYZW, 4, false> z("z");
const SwizzlePack<4, 1, SwizzleSet::XYZW, 8, false> w("w");

const SwizzlePack<1, 1, SwizzleSet::STPQ, 1, false> s("s");
const SwizzlePack<2, 1, SwizzleSet::STPQ, 2, false> t("t");
const SwizzlePack<3, 1, SwizzleSet::STPQ, 4, false> p("p");
const SwizzlePack<4, 1, SwizzleSet::STPQ, 8, false> q("q");

enum class Side { LEFT, RIGHT };

template<numberType type, unsigned int Nrows, unsigned int Ncols> class Matrix;

template<numberType type, unsigned int N> using Vec = Matrix<type, N, 1>;

template<numberType type> using Scalar = Vec<type, 1>;

using Double = Scalar<numberType::DOUBLE>;
using Float = Scalar<numberType::FLOAT>;
using Bool = Scalar<numberType::BOOL>;
using Uint = Scalar<numberType::UINT>;
using Int = Scalar<numberType::INT>;

using mat2x2 = Matrix<numberType::FLOAT, 2, 2>;
using mat2x3 = Matrix<numberType::FLOAT, 2, 3>;
using mat2x4 = Matrix<numberType::FLOAT, 2, 4>;
using mat3x2 = Matrix<numberType::FLOAT, 2, 2>;
using mat3x3 = Matrix<numberType::FLOAT, 3, 3>;
using mat3x4 = Matrix<numberType::FLOAT, 3, 4>;
using mat4x2 = Matrix<numberType::FLOAT, 4, 2>;
using mat4x3 = Matrix<numberType::FLOAT, 4, 3>;
using mat4x4 = Matrix<numberType::FLOAT, 4, 4>;

using mat2 = mat2x2;
using mat3 = mat3x3;
using mat4 = mat4x4;

using vec2 = Vec<numberType::FLOAT, 2>;
using vec3 = Vec<numberType::FLOAT, 3>;
using vec4 = Vec<numberType::FLOAT, 4>;

using dvec2 = Vec<numberType::DOUBLE, 2>;
using dvec3 = Vec<numberType::DOUBLE, 3>;
using dvec4 = Vec<numberType::DOUBLE, 4>;

using bvec2 = Vec<numberType::BOOL, 2>;
using bvec3 = Vec<numberType::BOOL, 3>;
using bvec4 = Vec<numberType::BOOL, 4>;

using ivec2 = Vec<numberType::INT, 2>;
using ivec3 = Vec<numberType::INT, 3>;
using ivec4 = Vec<numberType::INT, 4>;

using uvec2 = Vec<numberType::UINT, 2>;
using uvec3 = Vec<numberType::UINT, 3>;
using uvec4 = Vec<numberType::UINT, 4>;

template<numberType type, unsigned int Nrows, unsigned int Ncols> struct InitMatrix {
	InitMatrix(const std::string &s, const std::string & _exp) : name(s), expr(_exp) {}
	std::string name, expr;
};

template<typename T> T createDummy(const std::string & s, bool released = false) {
	bool enabled = Ctx().enable();
	Ctx().enable() = false;
	T t(s);
	t.released = released;
	Ctx().enable() = enabled;
	return t;
}

template<> void createDummy<void>(const std::string & s, bool released) {
	Ctx().addCmd(s + ";");
}

template<typename ... Ts> struct AreValidVecCtorT;

template<typename ...Ts> constexpr bool AreValidVecCtor = AreValidVecCtorT<Ts...>::value;
template<> struct AreValidVecCtorT<> {
	static const bool value = true;
};
template<typename T> struct AreValidVecCtorT<T> {
	static const bool value = false;
};
template<typename T, typename ...Ts> struct AreValidVecCtorT<T, Ts...> {
	static const bool value = AreValidVecCtor<T> && AreValidVecCtor<Ts...>;
};

template<numberType type, unsigned int N> struct AreValidVecCtorT<Vec<type, N>> {
	static const bool value = true;
};
template<> struct AreValidVecCtorT<double> {
	static const bool value = true;
};

template<typename ...Ts> struct getVecSizeT;

template<typename T> struct getVecSizeT<T> {
	static const unsigned int value = 17;
};
template<> struct getVecSizeT<> {
	static const unsigned int value = 0;
};
template<typename ...Ts> constexpr unsigned int getVecSize = getVecSizeT<Ts...>::value;

template<typename T, typename ...Ts> struct getVecSizeT<T, Ts...> {
	static const unsigned int value = getVecSize<T> +getVecSize<Ts...>;
};
template<numberType type, unsigned int N> struct getVecSizeT<Vec<type, N>> {
	static const unsigned int value = N;
};

template<> struct getVecSizeT<double> {
	static const unsigned int value = 1;
};

//// utilities struct

template<typename T> struct Infos {
	static const bool numeric_type = false;
	static const unsigned int rows = 0;
	static const unsigned int cols = 0;
	static const numberType scalar_type = numberType::ERROR;
};

template<numberType type, unsigned int Nrows, unsigned int Ncols>
struct Infos<Matrix<type, Nrows, Ncols>> {
	static const bool numeric_type = true;
	static const unsigned int rows = Nrows;
	static const unsigned int cols = Ncols;
	static const numberType scalar_type = type;
};

template<> struct Infos<double> {
	static const bool numeric_type = true;
	static const unsigned int rows = 1;
	static const unsigned int cols = 1;
	static const numberType scalar_type = numberType::FLOAT;
};

template<> struct Infos<int> {
	static const bool numeric_type = true;
	static const unsigned int rows = 1;
	static const unsigned int cols = 1;
	static const numberType scalar_type = numberType::INT;
};

template<typename A, typename B> constexpr bool EqualType = Infos<A>::scalar_type == Infos<B>::scalar_type;
template<typename A, typename B> constexpr bool EqualDim = (Infos<A>::rows == Infos<B>::rows) && (Infos<A>::cols == Infos<B>::cols);
template<typename A, typename B> constexpr bool EqualMat = (Infos<A>::scalar_type == Infos<B>::scalar_type) && EqualDim<A, B>;
template<numberType A, numberType B> constexpr numberType MinNumberType = (A > B ? B : A);

template<typename A, typename B> constexpr numberType MinType =
MinNumberType<Infos<A>::scalar_type, Infos<B>::scalar_type>;

template<unsigned int A, unsigned int B> constexpr unsigned int MaxUINT = A > B ? A : B;

template<typename A, typename B> constexpr unsigned int MaxRow = MaxUINT<Infos<A>::rows, Infos<B>::rows>;
template<typename A, typename B> constexpr unsigned int MaxCol = MaxUINT<Infos<A>::cols, Infos<B>::cols>;


template<typename A> constexpr bool NotBool = Infos<A>::numeric_type && Infos<A>::scalar_type > 0;
template<typename A, typename B> constexpr bool NoBools = NotBool<A> && NotBool<B>;

template<typename A> constexpr bool IsScalar = Infos<A>::numeric_type && Infos<A>::cols == 1 && Infos<A>::rows == 1;
template<typename A> constexpr bool IsVector = Infos<A>::numeric_type && Infos<A>::cols == 1;
template<typename A, typename B> constexpr bool AreScalar = IsScalar<A> && IsScalar<B>;
template<typename A, typename B> constexpr bool AreVector = IsVector<A> && IsVector<B>;

template<typename A, typename B> using ArithmeticBinaryReturnType = Matrix< MinType<A,B>, MaxRow<A,B>, MaxCol<A,B> >;

template<typename T> using MatrixType = Matrix<Infos<T>::scalar_type, Infos<T>::rows, Infos<T>::cols>;

template<typename A> constexpr bool IsIntLoose = Infos<A>::scalar_type == numberType::INT || Infos<A>::scalar_type == numberType::UINT;
template<typename A, typename B> constexpr bool AreIntLoose = IsIntLoose<A> && IsIntLoose<B>;

template<typename A> constexpr bool IsFPLoose = Infos<A>::scalar_type == numberType::FLOAT || Infos<A>::scalar_type == numberType::DOUBLE;
template<typename A, typename B> constexpr bool AreFPLoose = IsFPLoose<A> && IsFPLoose<B>;

template<typename A, typename B> constexpr bool EqualTypeLoose = EqualType<A, B> || AreFPLoose<A, B> || AreIntLoose<A, B>;


/// matrix class

template<numberType type, unsigned int Nrows, unsigned int Ncols>
class Matrix : public NamedObject<Matrix<type, Nrows, Ncols>> {
protected:
	//size related asserts
	static_assert(Nrows >= 1 && Ncols >= 1, "No empty dimensions allowed for matrices");
	static_assert(Nrows <= 4 && Ncols <= 4, "OpenGL does not support matrices strictly larger than 4");

	//type related assert
	static_assert(isFP<type>::value || Ncols == 1, "Non vector matrices only exist for FLOAT or DOUBLE");

	using Scalar = Matrix<type, 1, 1>;

protected:
	static constexpr bool scalar = IsScalar<Matrix>;
	static constexpr bool integral = scalar && (type == numberType::INT || type == numberType::UINT);
	static constexpr bool isbool = scalar && !notBool<type>;

public:
	using NamedObject<Matrix>::name;
	using NamedObject<Matrix>::released;

public:
	static const std::string typeStr() { return TypeStr<Matrix>::str(); }
	static const int numMembers() { return 0; } 

	~Matrix() {
		//std::cout << getName(*this) << std::endl;
		if (name != "" /* &&!released */) {
			//ctx.addCmd("dtor : " + name);
		}
		if (!released) {
			//std::cout << "not released " << name << std::endl;
			Ctx().addCmd(getName(*this) + ";");
		}
	}

	Matrix(const InitMatrix < type, Nrows, Ncols> & other) : NamedObject<Matrix>(other.name) {
		Ctx().addCmd(TypeStr<Matrix>::str() + " " + getName(*this) + " = " + other.expr + ";");
	}

	Matrix & operator=(const InitMatrix < type, Nrows, Ncols> & other) = delete;
	
	Matrix(const Matrix & m) : NamedObject<Matrix>(m.name) {
		release(m);
		//std::cout << "const M " << getName(m) << std::endl;
	}
	Matrix(Matrix && m) : NamedObject<Matrix>(m.name) {
		release(m);
		//std::cout << " M &&" << getName(m) << std::endl;
	}
	Matrix(const std::string & s = "") : NamedObject<Matrix>(s) {
		//std::cout << " ctor str " << s << std::endl;
		if (s != "") {
			released = true;
		}  
		Ctx().addCmd(TypeStr<Matrix>::str() + " " + getName(*this) + "; ");
	}

	//Matrix(const Matrix & other, const std::string & s) : NamedObject<Matrix>(s) {
	//	//std::cout << " same ctor " << name << " " << other.name << std::endl;
	//	release(other);
	//
	//	Ctx().addCmd(TypeStr<Matrix>::str() + " " + name + " = " + other.name + ";");
	//	
	//}

	//Matrix(const Matrix & other) : NamedObject<Matrix>("p"+other.name) {
	//	std::cout << " same ctor " << name << " " << other.name << std::endl;
	//}
	//Matrix(Matrix && other) = default;

	//template<typename U, typename = std::enable_if_t<EqualMat<Matrix,U> > >
	//Matrix & operator=(const U & other) {
	//	release(*this, other);
	//	Ctx().addCmd(getName(*this) + " = " + getName(other) + ";");
	//	return *this;
	//}

	Matrix & operator=(const Matrix & other) {
		//std::cout << " = operator " << name << " " << other.name << std::endl;
		release(*this, other);
	
		Ctx().addCmd(getName(*this) + " = " + getName(other) + ";");
		return *this;
	}

	template<bool b = scalar && ( isFP<type>::value || isInt<type>::value), typename = std::enable_if_t<b> >
	Matrix & operator=(const double & other) {
		release(*this, other);
		Ctx().addCmd(getName(*this) + " = " + getName(other) + ";");
		return *this;
	}

	template<bool b = scalar && isInt<type>::value, typename = std::enable_if_t<b> >
	Matrix & operator=(const int & other) {
		release(*this, other);
		Ctx().addCmd(getName(*this) + " = " + getName(other) + ";");
		return *this;
	}

	//template<bool b = scalar && isFP<type>::value, typename = std::enable_if_t<b> >
	//Matrix & operator=(const int & other) = delete;

	//template<bool b = scalar && isInt<type>::value, typename = std::enable_if_t<b> >
	//Matrix & operator=(const double & other) = delete;


	template<bool b = integral, typename = std::enable_if_t<b> >
	Matrix(const int& i, const std::string & s = ""
	) : NamedObject<Matrix>(s) {
		//static_assert(std::is_convertible_v<T, std::string>, "error ctor");
		//static_assert(Ncols == 1 && Nrows == 1, "contructor from integer only for Int");
		//std::cout << " ctor i str " << s << std::endl;
		Ctx().addCmd(TypeStr<Matrix>::str() + " " + getName(*this) + " = " + std::to_string(i) + ";");
		//ctx.flow.ctor(this);
		released = true;
	}

	template<bool b = isFP<type>::value, typename = std::enable_if_t<b> >
	Matrix(const double & d, const std::string & s ) : NamedObject<Matrix>(s) {
		//static_assert(std::is_convertible_v<T, std::string>, "error ctor");
		//static_assert(Ncols == 1 && Nrows == 1, "contructor from integer only for Int");
		//std::cout << " ctor i str " << s << std::endl;
		Ctx().addCmd(TypeStr<Matrix>::str() + " " + getName(*this) + " = " + std::to_string(d) + ";");
		//ctx.flow.ctor(this);
		released = true;
	}

	//template<bool b = isFP<type>::value, typename = std::enable_if_t<b> >
	//Matrix(const double & d) {

	//	NamedObject<Matrix>(std::to_string(d))
	//	//static_assert(std::is_convertible_v<T, std::string>, "error ctor");
	//	//static_assert(Ncols == 1 && Nrows == 1, "contructor from integer only for Int");
	//	//std::cout << " ctor i str " << s << std::endl;
	//	//Ctx().addCmd(TypeStr<Matrix>::str() + " " + getName(*this) + " = " + std::to_string(d) + ";");
	//	//ctx.flow.ctor(this);
	//	//released = true;
	//}

	template<typename ... Ts, typename = std::enable_if_t< AreValidVecCtor<Ts...> && getVecSize<Ts...> == Nrows * Ncols > >
	Matrix(const Ts & ... vs) {
		//std::cout << " = ctor " << strOf(vs...) << std::endl;
		name = TypeStr<Matrix>::str() + "(" + strFromObj(vs...) + ")";
		release(vs...);
	}

	template<typename T, typename = std::enable_if_t< NotBool<T> && ( IsScalar<T>  || EqualType<Matrix,T> ) > >
	/* explicit */ Matrix(const T & t)  {
		release(t);
		name = TypeStr<Matrix>::str() + "(" + getName(t) + ")";
		//Ctx().addCmd(TypeStr<Matrix>::str() + "(" + getName(t) + ")");
	}


	template<bool b = isbool, typename = std::enable_if_t<b> > operator bool() const {
		return false;
	}

	//// unary + operators

	template<bool b = notBool<type>, typename = std::enable_if_t<b> > const Matrix operator++(int) const {
		return createDummy<Matrix>("++" + getName(*this));
	}

	// unary ++ operators
	template<bool b = notBool<type>, typename = std::enable_if_t<b> > const Matrix operator++(int) {
		return createDummy<Matrix>(getName(*this) + "++");
	}
	template<bool b = notBool<type>, typename = std::enable_if_t<b> > const Matrix operator++() {
		return createDummy<Matrix>("++" + getName(*this));
	}

	// unary - operator
	template<bool b = notBool<type>, typename = std::enable_if_t<b> > const Matrix operator-() {
		return createDummy<Matrix>("-" + getName(*this));
	}

	// unary -- operators
	template<bool b = notBool<type>, typename = std::enable_if_t<b> > const Matrix operator--(int) {
		return createDummy<Matrix>(getName(*this) + "--");
	}
	template<bool b = notBool<type>, typename = std::enable_if_t<b> > const Matrix operator--() {
		return createDummy<Matrix>("--" + getName(*this));
	}

	// unary ! operator
	template<bool b = isbool, typename = std::enable_if_t<b> >
	const Bool operator!() const {
		release(*this);
		return createDummy<Bool>("!" + getName(*this));
	}

	// unary ~ operator
	template<bool b = isInt<type>::value && Ncols == 1 , typename = std::enable_if_t<b> >
	const Matrix operator~() const {
		release(*this);
		return createDummy<Matrix>("~" + getName(*this));
	}

	//operators X=

	template<typename A, typename = std::enable_if_t< NotBool<A> && (EqualMat<Matrix, A> || IsScalar<A>) > >
	void operator+=(const A& a) {
		release(a);
		Ctx().addCmd(getName(*this) + " += " + getName(a) + ";");
	}

	template<typename A, typename = std::enable_if_t< NotBool<A> && (EqualMat<Matrix, A> || IsScalar<A>) > >
	void operator-=(const A& a) {
		release(a);
		Ctx().addCmd(getName(*this) + " -= " + getName(a) + ";");
	}

	template<typename A, typename = std::enable_if_t< NotBool<A> && (EqualMat<Matrix, A> || IsScalar<A>) > >
	void operator*=(const A& a) {
		release(a);
		Ctx().addCmd(getName(*this) + " *= " + getName(a) + ";");
	}

	template<typename A, typename = std::enable_if_t< NotBool<A> && (EqualMat<Matrix, A> || IsScalar<A>) > >
	void operator/=(const A& a) {
		release(a);
		Ctx().addCmd(getName(*this) + " /= " + getName(a) + ";");
	}

	// swizzles accessors
	template<unsigned int Dim, unsigned int Size, SwizzleSet Set, unsigned int Bytes,
		typename = std::enable_if_t<Ncols == 1 && Nrows != 1 && Set != SwizzleSet::MIXED_SET && Dim <= Nrows && Size <= Nrows > >
		const Vec<type, Size> operator[](const SwizzlePack<Dim, Size, Set, Bytes, true> & a) const {
		release(*this);
		return createDummy<Vec<type, Size>>(getName(*this) + "." + a.s, true);
	}

	template<unsigned int Dim, unsigned int Size, SwizzleSet Set, unsigned int Bytes,
		typename = std::enable_if_t<Ncols == 1 && Nrows != 1 && Set != SwizzleSet::MIXED_SET && Dim <= Nrows && Size <= Nrows > >
		Vec<type, Size> operator[](const SwizzlePack<Dim, Size, Set, Bytes, false> & a) {
		release(*this);
		return createDummy<Vec<type, Size> >(getName(*this) + "." + a.s, true);
	}

	template<unsigned int Dim, unsigned int Size, SwizzleSet Set, unsigned int Bytes, bool Repeated,
		typename = std::enable_if_t<Ncols == 1 /* && Nrows != 1 */ && Set != SwizzleSet::MIXED_SET && Dim <= Nrows /* && Size <= Nrows */ > >
		const Vec<type, Size> operator[](const SwizzlePack<Dim, Size, Set, Bytes, Repeated> & a) const {
		release(*this);
		return createDummy<Vec<type, Size> >(getName(*this) + "." + a.s, true);
	}

	// array subscript accessors
	template<bool b = !scalar, typename = std::enable_if_t<b> >
	const typename std::conditional_t< Ncols == 1, Vec<type, 1>, Vec<type, Nrows> > operator[](unsigned int i) const {
		release(*this);
		return createDummy<const std::conditional_t< Ncols >= 2, Vec<type, Nrows>, Vec<type, 1> > >(getName(*this) + "[" + std::to_string(i) + "]");
	}

	template<bool b = !scalar, typename = std::enable_if_t<b> >
	typename std::conditional_t< Ncols == 1, Vec<type, 1>, Vec<type, Nrows> > operator[](unsigned int i) {
		release(*this);
		return createDummy<std::conditional_t< Ncols >= 2, Vec<type, Nrows>, Vec<type, 1> > >(getName(*this) + "[" + std::to_string(i) + "]");
	}
};

//special init naming operator 

template<numberType type, unsigned int Nrows, unsigned int Ncols>
const InitMatrix<type, Nrows, Ncols> operator<<
(
	const Matrix<type, Nrows, Ncols> & m,
	const std::string & s)
{
	release(m);
	return InitMatrix<type, Nrows, Ncols>(s, getName(m));
}

	


////// glsl matrix operators

// multiplication operators

// M x M
template<numberType type, unsigned int NrowsA, unsigned int NcolsA, unsigned int NcolsB,
	typename = std::enable_if_t< notBool<type> && CT_XOR<NrowsA == 1 && NcolsA == 1, NcolsA == 1 && NcolsB == 1> > >
const Matrix<type, NrowsA, NcolsB> operator* (const Matrix<type, NrowsA, NcolsA> & matA, const Matrix<type, NcolsA, NcolsB> & matB) {
	release(matA, matB);
	return createDummy<Matrix<type, NrowsA, NcolsB>>(getName(matA) + "*" + getName(matB));
}

template<typename A, typename B, typename = std::enable_if_t<
	NoBools<A, B> && (EqualMat<A, B> || IsScalar<A> || IsScalar<B>) > >
ArithmeticBinaryReturnType<A, B> operator*(const A& a, const B& b) {
	release(a, b);
	return createDummy<ArithmeticBinaryReturnType<A, B>>(getName(a) + "*" + getName(b));
}

template<typename A, typename B, typename = std::enable_if_t<
	NoBools<A, B> && (EqualMat<A, B> || IsScalar<A> || IsScalar<B>) > >
ArithmeticBinaryReturnType<A, B> operator+(const A& a, const B& b) {
	release(a, b);
	return createDummy<ArithmeticBinaryReturnType<A, B>>("(" + getName(a) + "+" + getName(b) + ")");
}

template<typename A, typename B, typename = std::enable_if_t<
	NoBools<A, B> && (EqualMat<A, B> || IsScalar<A> || IsScalar<B>) > >
ArithmeticBinaryReturnType<A, B> operator-(const A& a, const B& b) {
	release(a, b);
	return createDummy<ArithmeticBinaryReturnType<A, B>>("(" + getName(a) + "-" + getName(b) + ")");
}

template<typename A, typename B, typename = std::enable_if_t<
	NoBools<A, B> && (EqualMat<A, B> || IsScalar<A> || IsScalar<B>) > >
ArithmeticBinaryReturnType<A, B> operator/(const A& a, const B& b) {
	release(a, b);
	return createDummy<ArithmeticBinaryReturnType<A, B>>("( " + getName(a) + " )/( " + getName(b) + " )");
}

// Bool operators
const Bool operator&&(const Bool & b1, const Bool & b2) {
	release(b1, b2);
	return createDummy<Bool>("(" + getName(b1) + " && " + getName(b2) + ")");
}

const Bool operator||(const Bool & b1, const Bool & b2) {
	release(b1, b2);
	return createDummy<Bool>("(" + getName(b1) + " || " + getName(b2) + ")");
}

const Bool XOR(const Bool & b1, const Bool & b2) {
	release(b1, b2);
	return createDummy<Bool>("(" + getName(b1) + " ^^ " + getName(b2) + ")");
}

// bitwise operator
template<typename A, typename B, typename = std::enable_if_t<
	AreIntLoose<A, B> && AreVector<A, B> && (EqualDim<A, B> || IsScalar<A> || IsScalar<B>)
> >
const ArithmeticBinaryReturnType<A, B> operator&(const A& a, const B& b) {
	release(a, b);
	return createDummy<ArithmeticBinaryReturnType<A, B>>(getName(a) + "&" + getName(b));
}

template<typename A, typename B, typename = std::enable_if_t<
	AreIntLoose<A, B> && AreVector<A, B> && (EqualDim<A, B> || IsScalar<A> || IsScalar<B>)
> >
const ArithmeticBinaryReturnType<A, B> operator^(const A& a, const B& b) {
	release(a, b);
	return createDummy<ArithmeticBinaryReturnType<A, B>>(getName(a) + "^" + getName(b));
}

template<typename A, typename B, typename = std::enable_if_t<
	AreIntLoose<A, B> && AreVector<A, B> && (EqualDim<A, B> || IsScalar<A> || IsScalar<B>)
> >
const ArithmeticBinaryReturnType<A, B> operator|(const A& a, const B& b) {
	release(a, b);
	return createDummy<ArithmeticBinaryReturnType<A, B>>(getName(a) + "|" + getName(b));
}


//// comparison operators

template<typename A, typename B, typename = std::enable_if_t< NoBools<A,B> && IsScalar<A> && IsScalar<B> > >
const Bool operator<(const A& a, const B& b) {
	release(a,b);
	return createDummy<Bool>(getName(a) + "<" + getName(b));
}

template<typename A, typename B, typename = std::enable_if_t< NoBools<A, B> && IsScalar<A> && IsScalar<B> > >
const Bool operator>(const A& a, const B& b) {
	release(a, b);
	return createDummy<Bool>(getName(a) + ">" + getName(b));
}

template<typename A, typename B, typename = std::enable_if_t< NoBools<A, B> && EqualDim<A,B> > >
const Bool operator==(const A& a, const B& b) {
	release(a, b);
	return createDummy<Bool>(getName(a) + "==" + getName(b));
}


//// bit shifts operators
template<typename A, typename B, typename = std::enable_if_t< 
	AreIntLoose<A,B> && IsVector<A> && (EqualDim<A,B> || IsScalar<B> )
> >
const MatrixType<A> operator>>(const A& a, const B& b) {
	release(a, b);
	return createDummy<MatrixType<A>>(getName(a) + ">>" + getName(b));
}

template<typename A, typename B, typename = std::enable_if_t<
	AreIntLoose<A, B> && IsVector<A> && (EqualDim<A, B> || IsScalar<B>)
> >
const MatrixType<A> operator<<(const A& a, const B& b) {
	release(a, b);
	return createDummy<MatrixType<A>>(getName(a) + "<<" + getName(b));
}

//// modulo operator
template<typename A, typename B, typename = std::enable_if_t<
	AreIntLoose<A, B> && AreVector<A,B> && (EqualDim<A, B> || IsScalar<A> || IsScalar<B>)
> >
const ArithmeticBinaryReturnType<A,B> operator%(const A& a, const B& b) {
	release(a, b);
	return createDummy<ArithmeticBinaryReturnType<A, B>>(getName(a) + "%" + getName(b));
}


////

template<numberType nType> struct nTypeStr;

template<> struct nTypeStr<numberType::BOOL> {
	static const std::string str() { return "b"; }
};
template<> struct nTypeStr<numberType::INT> {
	static const std::string str() { return "i"; }
};
template<> struct nTypeStr<numberType::UINT> {
	static const std::string str() { return "u"; }
};
template<> struct nTypeStr<numberType::FLOAT> {
	static const std::string str() { return ""; }
};
template<> struct nTypeStr<numberType::DOUBLE> {
	static const std::string str() { return "d"; }
};

template<> struct TypeStr<Bool> {
	static const std::string str() { return "bool"; }
};

template<> struct TypeStr<Int> {
	static const std::string str() { return "int"; }
};

template<> struct TypeStr<Uint> {
	static const std::string str() { return "uint"; }
};

template<> struct TypeStr<Float> {
	static const std::string str() { return "float"; }
};

template<> struct TypeStr<Double> {
	static const std::string str() { return "double"; }
};


template<numberType type, unsigned int N> struct TypeStr< Vec<type, N> > {
	static const std::string str() { return nTypeStr<type>::str() + "vec" + std::to_string(N); }
};

template<numberType type, unsigned int N, unsigned int M> struct TypeStr< Matrix<type, N, M> > {
	static const std::string str() {
		return N == M ?
			 nTypeStr<type>::str() + "mat" + std::to_string(N)
		:
			nTypeStr<type>::str() + "mat" + std::to_string(N) + "x" + std::to_string(M);	
	}
};




#define INT(a) Int a( #a )
#define BOOL(a) Bool a( #a )
#define FLOAT(a) Float a( #a )




