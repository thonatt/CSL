#pragma once

#include <typeinfo>
#include <vector>
#include <array>

namespace v2 {

	template<std::size_t N>
	class Array {};

	template<typename T>
	struct Infos {
		static constexpr bool IsArray = false;
		static constexpr bool IsConst = false;
		static constexpr bool IsConstant = false;
	};

	template<typename T>
	struct Infos<const T> : Infos<T> {
		static constexpr bool IsConst = true;
	};

	template<>
	struct Infos<float> {
		static constexpr bool IsArray = false;
		static constexpr bool IsConst = false;
		static constexpr bool IsConstant = true;
	};

	template<std::size_t N>
	struct Infos<Array<N>> {
		static constexpr bool IsArray = true;
	};

	template<typename T, std::size_t R, std::size_t C, typename ... Qs>
	class Matrix;

	template<typename T, std::size_t R, typename ... Qs>
	using Vector = Matrix<T, R, 1, Qs...>;

	template<typename T, typename ... Qs>
	using Scalar = Vector<T, 1, Qs...>;


	template< typename T, std::size_t R, std::size_t C, typename ... Qs>
	class Matrix
	{
	public:

		static constexpr bool IsArray = (Infos<Qs>::IsArray || ...);
		using ArrayComponent = Matrix<T, R, C>;
		using Row = Matrix<T, 1, C>;
		using Col = Matrix<T, R, 1>;
		using Scalar = Matrix<T, 1, 1>;

		template<std::size_t OtherC>
		using MatrixMul = Matrix<T, R, OtherC>;

	public:
		Matrix() {}

		template<std::size_t N>
		Matrix(const char(&name)[N]) {}

		template<typename Other>
		Matrix operator*(Other&& other) {
			static_assert(!IsArray, "No operator* for arrays");
			return {};
		}

		template<typename Index>
		ArrayComponent operator [](Index&& index) {
			static_assert(IsArray, "T is not an array type");
			return {};
		}
	};

	//template<typename T, std::size_t R, std::size_t C>
	//Matrix()->Matrix<T, R, C>;

	//template<std::size_t N, typename T, std::size_t R, std::size_t C>
	//Matrix(const char(&)[N])->Matrix<T, R, C>;

	template<typename ... Qs>
	class vec3 : public Vector<float, 1, Qs...> {
	public:
		using Base = Vector<float, 1, Qs...>;
		using Base::Base;
	};

	vec3()->vec3<>;

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
