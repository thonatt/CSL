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

template<numberType type, unsigned int Nrows, unsigned int Ncols, Side side> class MatrixExpr;

template<numberType type, unsigned int Nrows, unsigned int Ncols> using Matrix2 = MatrixExpr<type,Nrows,Ncols,Side::LEFT>;

template<numberType type, unsigned int N> using Vec2 = Matrix2<type, N, 1>;

template<numberType type> using Scalar2 = Vec2<type, 1>;

using Bool2 = Scalar2<numberType::BOOL>;

template<numberType type, unsigned int Nrows, unsigned int Ncols, Side side> class MatrixExpr :
	NamedObject<MatrixExpr<type,Nrows,Ncols,side>>
{

};

template<numberType type, unsigned int Nrows, unsigned int Ncols, Side side> class MatrixExpr; 

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

template<unsigned int N> using VecF = Vec<numberType::FLOAT, N>;

template<typename T> struct isScalarT {
	static const bool value = false;
};

template<numberType type> struct isScalarT<Matrix<type,1,1>> {
	static const bool value = true;
};

template<typename T> constexpr bool isScalar = isScalarT<T>::value;

template<typename T> struct isSquareT {
	static const bool value = false;
};

template<numberType type, unsigned int N> struct isSquareT<Matrix<type, N, N>> {
static const bool value = true;
};

template<typename T> constexpr bool isSquare = isSquareT<T>::value;

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
	static constexpr bool scalar = isScalar<Matrix>;
	static constexpr bool integral = scalar && (type == numberType::INT || type == numberType::UINT);
	static constexpr bool isbool = scalar && !notBool<type>;

public:
	using NamedObject<Matrix>::name;
	using NamedObject<Matrix>::released;

public:
	static const std::string typeStr() { return TypeStr<Matrix>::str(); }
	static const int numMembers() { return 0; } 

	~Matrix() {
		//ctx.flow.dtor(this);
		if (name != "" /* &&!released */) {
			//ctx.addCmd("dtor : " + name);
		}
		if (!released) {
			//std::cout << "not released " << name << std::endl;
			Ctx().addCmd(name + ";");
		}
	}

	Matrix(const InitMatrix < type, Nrows, Ncols> & other) : NamedObject<Matrix>(other.name) {
		Ctx().addCmd(TypeStr<Matrix>::str() + " " + name + " = " + other.expr + ";");
	}

	Matrix & operator=(const InitMatrix < type, Nrows, Ncols> & other) = delete;

	Matrix(const std::string & s = "") : NamedObject<Matrix>(s) {
		//std::cout << " ctor str " << s << std::endl;
		if (s != "") {
			released = true;
		}  
		Ctx().addCmd(TypeStr<Matrix>::str() + " " + name + "; ");
	}

	//Matrix(const Matrix & other, const std::string & s) : NamedObject<Matrix>(s) {
	//	//std::cout << " same ctor " << name << " " << other.name << std::endl;
	//	release(other);
	//
	//	Ctx().addCmd(TypeStr<Matrix>::str() + " " + name + " = " + other.name + ";");
	//	
	//}

	//Matrix(const Matrix & other) = delete;
	//Matrix(Matrix && other) = default;

	Matrix & operator=(const Matrix & other) {
		//std::cout << " = operator " << name << " " << other.name << std::endl;
		release(*this, other);
	
		Ctx().addCmd(name + " = " + other.name + ";");
		return *this;
	}

	//template<typename T, typename = std::enable_if_t<!integral && !std::is_base_of_v<NamedObject,T> > > Matrix(const T & t) : Matrix(std::string(t)) {
	//	static_assert(!integral && !std::is_base_of_v<NamedObject, T>, "err ctor");
	//	static_assert(!std::is_same_v<T, int>, "cannot use 0 as std::string ctor");
	//	std::cout << "forbidden ctor " << std::string(t) << std::endl;
	//}

	template<typename T, typename = std::enable_if_t< integral > >
	Matrix(const int& i, const T & s = ""
	) : NamedObject<Matrix>(s) {
		static_assert(std::is_convertible_v<T, std::string>, "error ctor");
		//static_assert(Ncols == 1 && Nrows == 1, "contructor from integer only for Int");
		//std::cout << " ctor i str " << s << std::endl;
		Ctx().addCmd(TypeStr<Matrix>::str() + " " + name + " = " + std::to_string(i) + ";");
		//ctx.flow.ctor(this);
		released = true;
	}

	template<typename T, typename = std::enable_if_t< isFP<type>::value > >
	Matrix(const double & d, const std::string & s = ""
	) : NamedObject<Matrix>(s) {
		static_assert(std::is_convertible_v<T, std::string>, "error ctor");
		//static_assert(Ncols == 1 && Nrows == 1, "contructor from integer only for Int");
		//std::cout << " ctor i str " << s << std::endl;
		Ctx().addCmd(TypeStr<Matrix>::str() + " " + name + " = " + std::to_string(d) + ";");
		//ctx.flow.ctor(this);
		released = true;
	}

	//Matrix(Matrix && other) {
	//	//std::cout << " && ctor " << name << " " << other.name << std::endl;
	//	released = false;
	//	other.released = true;
	//	ctx.addCmd(name + " = " + other.name + ";");

	//}

	//template<unsigned int... Ns, typename = std::enable_if_t<SumOfV<Ns...> == Nrows * Ncols > >
	//Matrix(const Vec<type,Ns> & ... vs) {
	//	//std::cout << " = ctor " << strOf(vs...) << std::endl;
	//	name = TypeStr<Matrix>::str() + "(" + strOf(vs...) + ")";
	//	release(vs...);
	//}

	template<typename ... Ts, typename = std::enable_if_t< AreValidVecCtor<Ts...> && getVecSize<Ts...> == Nrows * Ncols > >
	Matrix(const Ts & ... vs) {
		//std::cout << " = ctor " << strOf(vs...) << std::endl;
		name = TypeStr<Matrix>::str() + "(" + strFromObj(vs...) + ")";
		release(vs...);
	}

	template<typename = std::enable_if_t< isbool > > operator bool() const { 
		return false;
	}

	//template<typename = std::enable_if_t< integral > > operator unsigned int() const {
	//	return 0;
	//}

	//// operators +
	template<typename = std::enable_if_t<notBool<type> /* && !scalar */ > >
	const Matrix operator+(const Scalar & f) const {
		release(*this,f);
		return createDummy<Matrix>(name + " + " + f.name);
	}

	template<typename = std::enable_if_t<notBool<type> > >
	Matrix & operator+=(const Scalar & f) { 
		release(f);
		Ctx().addCmd(name + " += " + f.name + ";");
		return *this; 
	}

	template<unsigned int otherNrows, unsigned int otherNcols, typename =
		std::enable_if_t<notBool<type> && otherNrows == Nrows && otherNcols == Ncols && !scalar > >
	const Matrix operator+(const Matrix<type, otherNrows, otherNcols> & other) const { 
		release(*this,other);
		return createDummy<Matrix>(name + " + " + other.name); 
	}

	template<unsigned int otherNrows, unsigned int otherNcols, typename =
		std::enable_if_t<notBool<type> && otherNrows == Nrows && otherNcols == Ncols && !scalar > >
	Matrix & operator+=(const Matrix<type, otherNrows, otherNcols> & other) {
		Ctx().addCmd(name + " += " + other.name + ";");
		return *this; 
	}

	//MxS
	//template<typename = std::enable_if_t<notBool<type> > >
	//const Matrix operator*(const Scalar & f) const {
	//	release(*this, f);
	//	return createDummy<Matrix>(name + "*" + f.name);
	//}

	//template<unsigned int otherNrows, unsigned int otherNcols, typename =
	//	std::enable_if_t< notBool<type> && scalar && !isScalar<Matrix<type, otherNrows, otherNcols> > >
	//>
	//const Matrix<type, otherNrows, otherNcols> operator*(const Matrix<type, otherNrows, otherNcols> & other) const {
	//	release(*this,other);
	//	return createDummy<Matrix<type, otherNrows, otherNcols>>(name + "*" + other.name);
	//}


	////MxM
	//template<unsigned int otherNrows, unsigned int otherNcols, typename =
	//	std::enable_if_t< notBool<type>::value && otherNrows == Ncols && !scalar && !isScalar<Matrix<type, otherNrows, otherNcols> > > >
	//const Matrix<type, Nrows, otherNcols> operator*(const Matrix<type, otherNrows, otherNcols> & other) const { return Matrix<type, Nrows, otherNcols>(); }

	//template<unsigned int otherNrows, unsigned int otherNcols, typename =
	//	std::enable_if_t< notBool<type>::value && otherNrows == Ncols && otherNcols == Ncols && !scalar > >
	//Matrix & operator*=(const Matrix<type, otherNrows, otherNcols> & other) { return *this; }

	////VxM
	//template<unsigned int N, typename = std::enable_if_t<notBool<type>::value && Ncols == 1 && Nrows == N && !scalar > >
	//const Vec<type,N> operator*(const Matrix<type, N, N> & other) const { return Vec<type, N>(); }

	//template<unsigned int N, typename = std::enable_if_t<notBool<type>::value && Ncols == 1 && Nrows == N && !scalar > >
	//Vec<type, N> & operator*=(const Matrix<type, N, N> & other) { return *this; }


	//M/S
	//template<typename = std::enable_if_t< notBool<type>::value> >
	//const Matrix operator/(const Scalar & f) const { return Matrix(); }

	//template<typename = std::enable_if_t< notBool<type>::value> >
	//Matrix & operator/=(const Scalar & f) { return *this; }

	//M/M
	//template<unsigned int otherNrows, unsigned int otherNcols, typename =
	//	std::enable_if_t< notBool<type>::value && otherNrows == Nrows && otherNcols == Ncols && !scalar > >
	//const Matrix operator/(const Matrix<type, otherNrows, otherNcols> & f) const { return Matrix(); }

	//template<unsigned int otherNrows, unsigned int otherNcols, typename =
	//	std::enable_if_t< notBool<type>::value && otherNrows == Nrows && otherNcols == Ncols && !scalar > >
	//Matrix & operator/=(const Matrix<type, otherNrows, otherNcols> & f) { return *this; }

	/*

	//// operators -
	template< typename = std::enable_if_t< notBool<type>::value > >
	const Matrix operator-() const { return Matrix(); }

	template<typename = std::enable_if_t<notBool<type>::value && !scalar > >
	const Matrix operator-(const Scalar & f) const { return Matrix(); }

	template<typename = std::enable_if_t<notBool<type>::value> >
	Matrix & operator-=(const Scalar & f) { return *this; }

	template<unsigned int otherNrows, unsigned int otherNcols, typename =
		std::enable_if_t<notBool<type>::value && otherNrows == Nrows && otherNcols == Ncols && !scalar > >
	const Matrix operator-(const Matrix<type, otherNrows, otherNcols> & other) const { return Matrix(); }
	
	template<unsigned int otherNrows, unsigned int otherNcols, typename =
		std::enable_if_t<notBool<type>::value && otherNrows == Nrows && otherNcols == Ncols && !scalar > >
	Matrix & operator-=(const Matrix<type, otherNrows, otherNcols> & other) { return *this; }
	
	*/
	//// operators ++ and --
	template<typename = std::enable_if_t< notBool<type> > > const Matrix operator++(int) const { 
		Ctx().ignoreNextCmds(1);
		Matrix m(name + "++");
		m.released = false;
		return m;
	}
	template<typename = std::enable_if_t< notBool<type> > > Matrix & operator++() {
		Ctx().addCmd("++" + name + ";");
		return *this; 
	}

	//template<typename = std::enable_if_t<!isBool<type>::value > > const Matrix operator--(int) const { return Matrix(); }
	//template<typename = std::enable_if_t<!isBool<type>::value > > Matrix & operator--() { return *this; }

	




	//template<typename = std::enable_if_t<!isBool<type>::value && scalar > > const Bool operator<=(const Scalar & s) const { return Bool(); }
	//template<typename = std::enable_if_t<!isBool<type>::value && scalar > > const Bool operator>(const Scalar & s) const { return Bool(); }
	//template<typename = std::enable_if_t<!isBool<type>::value && scalar > > const Bool operator>=(const Scalar & s) const { return Bool(); }

	/*
	//// operators %
	template<unsigned int otherNrows, unsigned int otherNcols, typename =
		std::enable_if_t<isInt<type>::value && Ncols == 1 && otherNrows == Nrows && otherNcols == Ncols && !scalar > >
	const Matrix operator%(const Matrix<type, otherNrows, otherNcols> & other) const { return Matrix(); }

	template<unsigned int otherNrows, unsigned int otherNcols, typename =
		std::enable_if_t<isInt<type>::value && Ncols == 1 && otherNrows == Nrows && otherNcols == Ncols && !scalar > >
		Matrix & operator%=(const Matrix<type, otherNrows, otherNcols> & other) { return *this; }

	template< typename = std::enable_if_t<isInt<type>::value && Ncols == 1 && !scalar > >
	const Matrix operator%(const Scalar & other) const { return Matrix(); }

	template< typename = std::enable_if_t<isInt<type>::value && Ncols == 1 && !scalar > >
	Matrix & operator%=(const Scalar & other) { return *this; }

	template<unsigned int N, typename = std::enable_if_t<isInt<type>::value && scalar && !isScalar<Vec<type, N> > > >
	const Vec<type,N> operator%(const Vec<type,N> & other) const { return Vec<type, N>(); }

	//// operators == and !=
	const Bool operator==(const Matrix & other) const { return Bool(); }
	const Bool operator!=(const Matrix & other) const { return Bool(); }

	//// operators !, &&, ||
	//template<typename = std::enable_if_t<isBool<type>::value && scalar::value > > const Bool operator!() const { return Bool(); }
	
	template<typename = std::enable_if_t<isBool<type>::value && Ncols == 1 > >
	const Vec<numberType::BOOL, Nrows> operator!() const { return Vec<numberType::BOOL, Nrows>(); }

	template<typename = std::enable_if_t<isBool<type>::value && scalar > > const Bool operator&&(const Bool & b) const { return Bool(); }
	template<typename = std::enable_if_t<isBool<type>::value && scalar > > const Bool operator||(const Bool & b) const { return Bool(); }

	//// operator ~
	template<unsigned int N, typename = std::enable_if_t<isInt<type>::value && Ncols == 1> > 
	const Vec<type, N> operator~() const { return Vec<type, N>(); }

	//// operators << and >>
	//S<<V
	template<unsigned int N, typename = std::enable_if_t<isInt<type>::value && Nrows == 1 && Ncols == 1 > >
	const Vec<type, N> operator<<(const Vec<type, N> & s) const { return Vec<type, N>(); }

	//V<<S
	template<typename = std::enable_if_t<isInt<type>::value && Nrows >= 2 && Ncols == 1 > >
	const Matrix operator<<(const Scalar & s) const { return Matrix(); }

	template<typename = std::enable_if_t<isInt<type>::value && Nrows >= 2 && Ncols == 1 > >
	Matrix & operator<<=(const Scalar & s) { return *this; }

	//V<<V
	template<unsigned int N, typename = std::enable_if_t<isInt<type>::value && !scalar && N == Nrows && Ncols == 1 > >
	const Matrix operator<<(const Vec<type, N> & v) const { return Matrix(); }

	template<unsigned int N, typename = std::enable_if_t<isInt<type>::value && !scalar && N == Nrows && Ncols == 1 > >
	Matrix & operator<<=(const Vec<type, N> & v) { return *this; }

	//S>>V
	template<unsigned int N, typename = std::enable_if_t<isInt<type>::value && Nrows == 1 && Ncols == 1 > >
	const Vec<type, N> operator>>(const Vec<type, N> & s) const { return Vec<type, N>(); }

	//V>>S
	template<typename = std::enable_if_t<isInt<type>::value && Nrows >= 2 && Ncols == 1 > >
	const Matrix operator>>(const Scalar & s) const { return Matrix(); }

	template<typename = std::enable_if_t<isInt<type>::value && Nrows >= 2 && Ncols == 1 > >
	Matrix & operator>>(const Scalar & s) { return *this; }

	//V>>V
	template<unsigned int N, typename = std::enable_if_t<isInt<type>::value && !scalar && N == Nrows && Ncols == 1 > >
	const Matrix operator>>(const Vec<type, N> & v) const { return Matrix(); }

	template<unsigned int N, typename = std::enable_if_t<isInt<type>::value && !scalar && N == Nrows && Ncols == 1 > >
	Matrix & operator>>=(const Vec<type, N> & v) { return *this; }

	//// operators &, ^ and |
	// &
	template<unsigned int N, typename = std::enable_if_t<isInt<type>::value && Nrows == 1 && Ncols == 1 > >
	const Vec<type, N> operator&(const Vec<type, N> & s) const { return Vec<type, N>(); }

	template<typename = std::enable_if_t<isInt<type>::value && Nrows >= 2 && Ncols == 1 > >
	const Matrix operator&(const Scalar & s) const { return Matrix(); }

	template<typename = std::enable_if_t<isInt<type>::value && Nrows >= 2 && Ncols == 1 > >
	Matrix & operator&=(const Scalar & s) { return *this; }

	template<unsigned int N, typename = std::enable_if_t<isInt<type>::value && !scalar && N == Nrows && Ncols == 1 > >
	const Matrix operator&(const Vec<type, N> & v) const { return Matrix(); }

	template<unsigned int N, typename = std::enable_if_t<isInt<type>::value && !scalar && N == Nrows && Ncols == 1 > >
	const Matrix & operator&=(const Vec<type, N> & v) { return *this; }

	// ^
	template<unsigned int N, typename = std::enable_if_t<isInt<type>::value && Nrows == 1 && Ncols == 1 > >
	const Vec<type, N> operator^(const Vec<type, N> & s) const { return Vec<type, N>(); }

	template<typename = std::enable_if_t<isInt<type>::value && Nrows >= 2 && Ncols == 1 > >
	const Matrix operator^(const Scalar & s) const { return Matrix(); }

	template<typename = std::enable_if_t<isInt<type>::value && Nrows >= 2 && Ncols == 1 > >
	Matrix & operator^=(const Scalar & s) { return *this; }

	template<unsigned int N, typename = std::enable_if_t<isInt<type>::value && !scalar::value && N == Nrows && Ncols == 1 > >
	const Matrix operator^(const Vec<type, N> & v) const { return Matrix(); }

	template<unsigned int N, typename = std::enable_if_t<isInt<type>::value && !scalar && N == Nrows && Ncols == 1 > >
	Matrix & operator^=(const Vec<type, N> & v) { return *this; }

	// |
	template<unsigned int N, typename = std::enable_if_t<isInt<type>::value && scalar > >
	const Vec<type, N> operator|(const Vec<type, N> & s) const { return Vec<type, N>(); }

	template<typename = std::enable_if_t<isInt<type>::value && Nrows >= 2 && Ncols == 1 > >
	const Matrix operator|(const Scalar & s) const { return Matrix(); }

	template<typename = std::enable_if_t<isInt<type>::value && Nrows >= 2 && Ncols == 1 > >
	Matrix & operator|=(const Scalar & s) { return *this; }

	template<unsigned int N, typename = std::enable_if_t<isInt<type>::value && !scalar && N == Nrows && Ncols == 1 > >
	const Matrix operator|(const Vec<type, N> & v) const { return Matrix(); }

	template<unsigned int N, typename = std::enable_if_t<isInt<type>::value && !scalar && N == Nrows && Ncols == 1 > >
	Matrix & operator|=(const Vec<type, N> & v) { return *this; }

	*/

	//// swizzles
	template<unsigned int Dim, unsigned int Size, SwizzleSet Set, unsigned int Bytes, 
		typename = std::enable_if_t<Ncols == 1 && Nrows != 1 && Set != SwizzleSet::MIXED_SET && Dim <= Nrows && Size <= Nrows > >
	const Vec<type, Size> operator[](const SwizzlePack<Dim, Size, Set, Bytes, true> & a) const { 
		return createDummy<Vec<type, Size>>(name + "." + a.s,true);
	}
	
	template<unsigned int Dim, unsigned int Size, SwizzleSet Set, unsigned int Bytes,
		typename = std::enable_if_t<Ncols == 1 && Nrows != 1 && Set != SwizzleSet::MIXED_SET && Dim <= Nrows && Size <= Nrows > >
	 Vec<type, Size> operator[](const SwizzlePack<Dim, Size, Set, Bytes, false> & a) { 
		return createDummy<Vec<type, Size> >(name + "." + a.s,true);
	}

	template<unsigned int Dim, unsigned int Size, SwizzleSet Set, unsigned int Bytes, bool Repeated,
		typename = std::enable_if_t<Ncols == 1 /* && Nrows != 1 */ && Set != SwizzleSet::MIXED_SET && Dim <= Nrows /* && Size <= Nrows */ > >
	const Vec<type, Size> operator[](const SwizzlePack<Dim, Size, Set, Bytes, Repeated> & a) const {
		release(*this);
		return createDummy<Vec<type, Size> >(name + "." + a.s, true); 
	}

	//// array subscript
	template<typename = std::enable_if_t<!scalar > >
	typename std::conditional_t< Ncols == 1, Vec<type, 1>, Vec<type, Nrows> > operator[](unsigned int i) const {
		return std::conditional_t< Ncols >= 2, Vec<type, Nrows>, Vec<type, 1> >();
	}
};


//special init naming operator 

template<numberType type, unsigned int Nrows, unsigned int Ncols>
const InitMatrix<type, Nrows, Ncols> operator<<(
	const Matrix<type, Nrows, Ncols> & m,
	const std::string & s) {
	release(m);
	return InitMatrix<type, Nrows, Ncols>(s, m.name);
}

// glsl matrix operators


// multiplication operators

// M x M
template<numberType type, unsigned int NrowsA, unsigned int NcolsA, unsigned int NcolsB, 
	typename = std::enable_if_t< notBool<type> && CT_XOR<NrowsA == 1 && NcolsA == 1, NcolsA == 1 && NcolsB == 1> > >
const Matrix<type, NrowsA, NcolsB> operator* (const Matrix<type, NrowsA, NcolsA> & matA, const Matrix<type, NcolsA, NcolsB> & matB) {
	release(matA, matB);
	return createDummy<Matrix<type, NrowsA, NcolsB>>(matA.name + "*" + matB.name);
}

// M x S
template<numberType type, unsigned int Nrows, unsigned int Ncols, 
	typename = std::enable_if_t< notBool<type> && !(Nrows == 1 && Ncols == 1) > >
const Matrix<type, Nrows, Ncols> operator* (const Matrix<type, Nrows, Ncols> & mat, const Scalar<type> & s) {
	release(mat, s);
	return createDummy<Matrix<type, Nrows, Ncols>>(mat.name + "*" + s.name);
}

// S x M
template<numberType type, unsigned int Nrows, unsigned int Ncols,
	typename = std::enable_if_t< notBool<type> && !(Nrows == 1 && Ncols == 1) > >
const Matrix<type, Nrows, Ncols> operator* (const Scalar<type> & s, const Matrix<type, Nrows, Ncols> & mat) {
	release(mat, s);
	return createDummy<Matrix<type, Nrows, Ncols>>(s.name + "*" + mat.name);
}

// M + M
template<numberType type, unsigned int Nrows, unsigned int Ncols, typename = std::enable_if_t< notBool<type> > >
const Matrix<type, Nrows, Ncols> operator+ (
	Matrix<type, Nrows, Ncols> & m1, const Matrix<type, Nrows, Ncols> & m2) {
	return createDummy<Matrix<type, Nrows, Ncols>>(m1.name + " + " + m2.name);
}

// M - M
template<numberType type, unsigned int Nrows, unsigned int Ncols, typename = std::enable_if_t< notBool<type> > >
const Matrix<type, Nrows, Ncols> operator- (
	Matrix<type, Nrows, Ncols> & m1, const Matrix<type, Nrows, Ncols> & m2  ) {
	return createDummy<Matrix<type, Nrows, Ncols>>(m1.name + " - " + m2.name);
}

// comparison oeprators
template<numberType type, typename = std::enable_if_t< notBool<type> > >
const Bool operator<(const Scalar<type> & s1, const Scalar<type> & s2) {
	return createDummy<Bool>(s1.name + "<" + s2.name);
}

template<numberType type, typename = std::enable_if_t< notBool<type> > >
const Bool operator<(const Scalar<type> & s, int i) {
	return createDummy<Bool>(s.name + "<" + std::to_string(i));
}

template<numberType type, typename = std::enable_if_t< notBool<type> > >
const Bool operator>(const Scalar<type> & s1, const Scalar<type> & s2) {
	return createDummy<Bool>(s1.name + ">" + s2.name);
}

template<numberType type, typename = std::enable_if_t< notBool<type> > >
const Bool operator>(int i, const Scalar<type> & s ) {
	return createDummy<Bool>( std::to_string(i) + "<" + s.name);
}

//template<numberType type, unsigned int Nrows, unsigned int Ncols, typename = std::enable_if_t< notBool<type>::value > >
//const Matrix<type, Nrows, Ncols> operator*(const Scalar<type> & f, const Matrix<type, Nrows, Ncols> & mat) {
//	return createDummy<Matrix<type, Nrows, Ncols>>(f.name + "" + mat.name ); 
//}


//template<unsigned int N> const Bool any(const Vec<numberType::BOOL, N> & v) { return Bool(); }
//template<unsigned int N> const Bool all(const Vec<numberType::BOOL, N> & v) { return Bool(); }
//
//template<typename T> struct GetStrTmp {
//	static const std::string str() { return ""; }
//	static const std::string strArg(const T & t) { return ""; }
//};


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


#define INT(a) Int a( #a )
#define BOOL(a) Bool a( #a )
#define FLOAT(a) Float a( #a )




