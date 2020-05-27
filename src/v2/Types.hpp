#pragma once

#include "Qualifiers.hpp"

#include <cstdint>
#include <tuple>
#include <utility>

namespace v2 {

	using uint = std::uint32_t;


	//
	template<std::size_t ...Ns>
	struct SizeList;

	template<>
	struct SizeList<>
	{
		using Front = SizeList<>;
		using Tail = SizeList<>;
		static constexpr std::size_t Size = 0;
	};

	template<std::size_t N, std::size_t ...Ns>
	struct SizeList<N, Ns...>
	{
		static constexpr std::size_t Size = 1 + sizeof...(Ns);
		using Front = SizeList<N>;
		using Tail = SizeList<Ns...>;
	};

	template<typename List>
	struct GetArrayFromList;

	template<std::size_t ... Ns>
	struct GetArrayFromList<SizeList<Ns...>> {
		using Type = Array<Ns...>;
	};

	// Type list utils

	template<typename ... Args>
	struct TList {
		using Tuple = std::tuple<Args...>;
		template<std::size_t id> using GetType = typename std::tuple_element<id, Tuple>::type;

		static constexpr std::size_t Size = sizeof...(Args);
	};

	template<typename T>
	struct GetTList;

	template<typename ...Args>
	struct GetTList<std::tuple<Args...>> {
		using Type = TList<Args...>;
	};

	template<std::size_t Id, typename List, std::size_t... Ids>
	auto remove_at_impl(std::index_sequence<Ids...>) -> decltype(std::tuple_cat(
		std::declval<
		std::conditional_t<(Id == Ids),
		std::tuple<>,
		std::tuple<typename List::GetType<Ids>>
		>
		>()...
	));

	template <std::size_t Id, typename List>
	using RemoveAt = typename GetTList<decltype(remove_at_impl<Id, List>(std::make_index_sequence<List::Size>{})) > ::Type;

	template<template <typename> typename Pred, typename List, std::size_t CurrentId>
	struct LastOfImpl {
		static constexpr bool Exists = false;
		static constexpr std::size_t Id = 0;
	};

	template<template <typename> typename Pred, typename List>
	struct LastOf {
		static constexpr bool Exists = LastOfImpl<Pred, List, 0>::Exists;
		static constexpr std::size_t Id = LastOfImpl<Pred, List, 0>::Id;
	};

	template<template <typename> typename Pred, typename T, std::size_t CurrentId, typename ...Ts>
	struct LastOfImpl<Pred, TList<T, Ts...>, CurrentId> {
		static constexpr bool Exists = Pred<T>::Value || LastOfImpl<Pred, TList<Ts...>, CurrentId + 1>::Exists;
		static constexpr std::size_t Id = std::max((Pred<T>::Value ? CurrentId : 0), LastOfImpl<Pred, TList<Ts...>, CurrentId + 1>::Id);
	};

	///////////////////////////////////

	template<typename T, std::size_t R, std::size_t C, typename ... Qs>
	class Matrix;

	template<typename T, std::size_t R, typename ... Qs>
	using Vector = Matrix<T, R, 1, Qs...>;

	template<typename T, typename ...Qs>
	using Scalar = Vector<T, 1, Qs...>;

	template< typename T, typename Ds, std::size_t R, std::size_t C, typename ... Qs>
	class MatrixArray;

	template<typename T>
	struct IsArray {
		static constexpr bool Value = false;
		using Dimensions = SizeList<>;
	};

	template<std::size_t ...Ns>
	struct IsArray<Array<Ns...>> {
		static constexpr bool Value = true;
		using Dimensions = SizeList<Ns...>;
	};

	template<typename ...Qs>
	struct ArrayInfos {
		using List = TList<Qs...>;
		static constexpr bool HasArray = LastOf<IsArray, List>::Exists;
		static constexpr std::size_t Id = LastOf<IsArray, List>::Id;
		using Dimensions = std::conditional_t<HasArray, typename IsArray<typename List::GetType<Id>>::Dimensions, SizeList<>>;
		static constexpr bool Value = (Dimensions::Size > 0);
	};

	template<>
	struct ArrayInfos<> {
		using List = TList<>;
		static constexpr bool HasArray = false;
		static constexpr std::size_t Id = 0;
		using Dimensions = SizeList<>;
		static constexpr bool Value = false;
	};

	template<typename T>
	struct Infos {
		static constexpr bool IsArray = false;
	};

	template<typename T>
	struct Infos<const T> : Infos<T> {
		static constexpr bool IsConst = true;
	};

	template<typename T>
	struct Infos<T&> : Infos<T> { };

	template<typename T, std::size_t R, std::size_t C, typename ...Qs>
	struct Infos<Matrix<T, R, C, Qs...>> {
		static constexpr bool IsConst = false;
		static constexpr bool IsConstant = false;
		static constexpr bool IsScalar = (R == 1 && C == 1);
		static constexpr bool IsVec = (C == 1);
		static constexpr bool IsSquare = (R == C);

		static constexpr std::size_t NumElements = R * C;
		static constexpr std::size_t RowCount = R;
		static constexpr std::size_t ColCount = C;

		static constexpr bool IsFloating = std::is_same_v<T, float> || std::is_same_v<T, double>;
		static constexpr bool IsInteger = std::is_same_v<T, int> || std::is_same_v<T, uint>;
		static constexpr bool IsBool = std::is_same_v<T, bool>;
		using ScalarType = T;
		using QualifierFree = Matrix<T, R, C>;
	};

	template<typename T, typename Ds, std::size_t R, std::size_t C, typename ...Qs>
	struct Infos<MatrixArray<T, Ds, R, C, Qs...>> {
	};

	template<typename ...Ts>
	constexpr std::size_t NumElements = (Infos<Ts>::NumElements + ...);

	template<typename ...Ts>
	constexpr bool SameSize = (Infos<Ts>::RowCount == ...) && (Infos<Ts>::ColCount == ...);

	template<typename A, typename B>
	constexpr bool HasMoreElementsThan = Infos<A>::NumElements >= Infos<B>::NumElements;

	template<typename ...Ts>
	constexpr bool SameScalarType = true;

	template<typename T, typename U, typename ...Ts>
	constexpr bool SameScalarType<T, U, Ts...> = std::is_same_v<typename Infos<T>::ScalarType, typename Infos<U>::ScalarType> && SameScalarType<U, Ts...>;

	template<typename T>
	constexpr bool IsInteger = std::is_same_v<Infos<T>::ScalarType, int> || std::is_same_v<Infos<T>::ScalarType, uint>;

	template<>
	struct Infos<double> : Infos<Matrix<float, 1, 1>> {
		static constexpr bool IsConstant = true;
	};

	template<>
	struct Infos<float> : Infos<Matrix<float, 1, 1>> {
		static constexpr bool IsConstant = true;
	};

	template<>
	struct Infos<int> : Infos<Matrix<int, 1, 1, TList<>>> {
		static constexpr bool IsConstant = true;
	};

	template<typename A, typename B>
	struct AlgebraMulInfos {
		static_assert(SameScalarType<A,B>);

		// Scalar*Matrix get transformed into Matrix*Scalar
		static_assert(!Infos<A>::IsScalar || Infos<B>::IsScalar);

		using ScalarType = typename Infos<A>::ScalarType;
		static constexpr std::size_t OutRowCount = Infos<A>::RowCount;
		static constexpr std::size_t OutColCount = Infos<std::conditional_t<Infos<B>::IsScalar, A, B>>::ColCount;
		using ReturnType = Matrix<ScalarType, OutRowCount, OutColCount>;


		static constexpr bool SameSize = SameSize<A, B>;
		static constexpr Op Operator = (SameSize && Infos<A>::IsVec) ? Op::CWiseMul : (Infos<B>::IsScalar ? Op::MatrixTimesScalar : Op::MatrixTimesMatrix);
	};

	template<typename A>
	constexpr Op ScalarMulType = IsInteger<A> ? Op::OpIMul : Op::OpFMul;
}
