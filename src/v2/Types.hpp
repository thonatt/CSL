#pragma once

#include "Qualifiers.hpp"
#include  <cstdint>

namespace v2 {

	using uint = std::uint32_t;

	template<typename T, std::size_t R, std::size_t C, typename ... Qs>
	class Matrix;

	template<typename T, std::size_t R, typename ... Qs>
	using Vector = Matrix<T, R, 1, Qs...>;

	template<typename T, typename ... Qs>
	using Scalar = Vector<T, 1, Qs...>;

	template<typename T>
	struct Infos {
		static constexpr bool IsArray = false;
	};

	template<std::size_t N>
	struct Infos<Array<N>> {
		static constexpr bool IsArray = true;
	};

	template<typename ...Qs>
	constexpr bool IsArray = (Infos<Qs>::IsArray || ...);

	template<typename T>
	struct Infos<const T> : Infos<T> {
		static constexpr bool IsConst = true;
	};

	template<typename T>
	struct Infos<T&> : Infos<T> { };

	template<typename T, std::size_t R, std::size_t C, typename ... Qs>
	struct Infos<Matrix<T, R, C, Qs...>> {
		static constexpr bool IsArray = IsArray<Qs...>;
		static constexpr bool IsConst = false;
		static constexpr bool IsConstant = false;
		static constexpr std::size_t NumElements = R * C;
		static constexpr bool IsInteger = std::is_same_v<T, int> || std::is_same_v<T, uint>;
		using ScalarType = T;
	};

	template<typename ...Ts>
	constexpr std::size_t NumElements = (Infos<Ts>::NumElements + ...);

	template<typename ...Ts>
	constexpr bool SameScalarType = true;

	template<typename T, typename U, typename ...Ts>
	constexpr bool SameScalarType<T, U, Ts...> = std::is_same_v<typename Infos<T>::ScalarType, typename Infos<U>::ScalarType>  && SameScalarType<U, Ts...>;

	template<typename T>
	constexpr bool IsInteger = std::is_same_v<Infos<T>::ScalarType, int> || std::is_same_v<Infos<T>::ScalarType, uint>;

	template<>
	struct Infos<float> : Infos<Matrix<float, 1, 1>> {
		static constexpr bool IsConstant = true;
	};

	template<>
	struct Infos<int> : Infos<Matrix<int, 1, 1>> {
		static constexpr bool IsConstant = true;
	};

}
