#pragma once

#include "NamedObjects.hpp"
#include "Swizzles.hpp"
#include "Types.hpp"

#include <typeinfo>
#include <vector>
#include <array>

#define EXPR(type, var) (get_expr(std::forward<type>(var)))

namespace v2 {

	template< typename T, std::size_t R, std::size_t C, typename ... Qs>
	class Matrix : public NamedObject<Matrix<T, R, C, Qs...>> {
	public:

		using This = Matrix;
		using Row = Matrix<T, 1, C, Qs...>;
		using Col = Matrix<T, R, 1, Qs...>;
		using Scalar = Matrix<T, 1, 1, Qs...>;

		template<std::size_t RowCount>
		using SubCol = Vector<T, RowCount, Qs...>;

		static constexpr std::size_t RowCount = R;
		static constexpr std::size_t ColCount = C;
		static constexpr bool IsScalar = (R == 1 && C == 1);

		template<std::size_t OtherC>
		using MatrixMul = Matrix<T, R, OtherC>;

		using Base = NamedObject<Matrix<T, R, C, Qs...>>;

	public:
		Matrix() : Base() {}

		template<std::size_t N>
		explicit Matrix(const char(&name)[N]) : Base(name) {}

		template<typename U, typename V, typename ...Vs,
			typename = std::enable_if_t< (NumElements<U, V, Vs...> == R * C) && SameScalarType<Matrix, U, V, Vs...> > >
			explicit Matrix(U&& u, V&& v, Vs&&...vs) : Base("", ObjFlags::Default, CtorFlags::Initialisation, EXPR(U, u), EXPR(V, v), EXPR(Vs, vs)...) {

		}

		template<typename Other>
		Matrix operator*(Other&& other) {
			return {};
		}

		template<typename Swizzle, typename = std::enable_if_t<C == 1 && R != 1 && (Swizzle::Size <= R)> >
		std::conditional_t<Swizzle::Unique, SubCol<Swizzle::Size>, const SubCol<Swizzle::Size>> operator[](Swizzle swizzle) const& {
			return {};
		}

		template<typename Index, typename = std::enable_if_t<!IsScalar && Infos<Index>::IsInteger > >
		std::conditional_t<C == 1, Scalar, Col> operator[](Index&& index) const&
		{
			//get_expr(std::forward<Index>(index));
			return { };
		}


	};

	template< typename T, std::size_t N, std::size_t R, std::size_t C, typename ... Qs>
	class MatrixArray {
	public:
		using ArrayComponent = Matrix<T, R, C, Qs...>;

		template<typename Index>
		ArrayComponent operator [](Index&& index) {
			return {};
		}
	};

	template< typename T, std::size_t R, std::size_t C, typename ... Qs>
	class MatrixInterface : public std::conditional_t<IsArray<Qs...>, MatrixArray<T, 0, R, C, Qs...>, Matrix<T, R, C, Qs...>>
	{
	public:
		using Derived = std::conditional_t<IsArray<Qs...>, MatrixArray<T, 0, R, C, Qs...>, Matrix<T, R, C, Qs...>>;
		using Derived::Derived;
	};

	template<typename T, std::size_t R, typename ... Qs>
	using VectorInterface = MatrixInterface<T, R, 1, Qs...>;

	template<typename T, typename ... Qs>
	using ScalarInterface = VectorInterface<T, 1, Qs...>;


	template<typename ... Qs>
	class mat3 : public MatrixInterface<float, 3, 3, Qs...> {
	public:
		using Base = MatrixInterface<float, 3, 3, Qs...>;
		using Base::Base;
	};

	mat3()->mat3<>;

	template<typename U, typename V, typename ...Vs>
	mat3(U&&, V&&, Vs&&...)->mat3<>;

	template<typename ... Qs> struct Infos<mat3<Qs...>> : Infos<Matrix<float, 3, 3, Qs...>> {};

	template<typename ... Qs>
	class vec3 : public VectorInterface<float, 3, Qs...> {
	public:
		using Base = VectorInterface<float, 3, Qs...>;
		using Base::Base;
	};
	vec3()->vec3<>;

	template<typename ... Qs> struct Infos<vec3<Qs...>> : Infos<Vector<float, 3, Qs...>> {};

	template<std::size_t N>
	vec3(const char(&)[N])->vec3<>;

	struct Q {};

	void deduction_test() {

		vec3 a;
		//a[0];
		a* a;

		vec3 aa("aa");
		vec3<Q> b;
		vec3<Q> bb = vec3<Q>("bb");
		vec3<Q, Array<3>> c;
		c[0] * c[1];
		//c* c;

		float f = 1.0f;
		const vec3 d;

		static_assert(Infos<decltype(f)>::IsConstant);
		static_assert(Infos<decltype(d)>::IsConst);
	}

} //namespace csl

#undef EXPR