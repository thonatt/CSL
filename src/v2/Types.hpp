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
		using Tail = SizeList<>;
		static constexpr std::size_t Size = 0;

		static constexpr std::size_t Front = 0;
		static constexpr std::size_t Back = 0;

		template<std::size_t M>
		using PushFront = SizeList<M>;

		template<std::size_t M>
		using PushBack = SizeList<M>;
	};

	template<std::size_t N, std::size_t ...Ns>
	struct SizeList<N, Ns...>
	{
		using Tail = SizeList<Ns...>;

		static constexpr std::size_t Size = 1 + sizeof...(Ns);
		static constexpr std::size_t Front = N;
		static constexpr std::size_t Back = (Size == 1 ? N : Tail::Back);
		
		template<std::size_t M>
		using PushFront = SizeList<M, N, Ns...>;

		template<std::size_t M>
		using PushBack = SizeList<N, Ns..., M>;
	};

	template<typename List>
	struct GetArrayFromList;

	template<std::size_t ... Ns>
	struct GetArrayFromList<SizeList<Ns...>> {
		using Type = Array<Ns...>;
	};

	//////////////////////////////////////////////////////////////////////////
	// Type list utils

	template<typename ... Ts>
	struct TList {
		using Tuple = std::tuple<Ts...>;

		template<std::size_t Id> 
		using GetType = typename std::tuple_element<Id, Tuple>::type;
		
		template<typename T>
		using PushFront = TList<T, Ts...>;

		template<typename T>
		using PushBack = TList<Ts..., T>;

		static constexpr std::size_t Size = sizeof...(Ts);
	};

	template<template<typename, typename> typename Pred, typename A, typename B>
	constexpr bool EqualLists = false;

	template<template<typename, typename> typename Pred, typename ...As, typename ...Bs>
	constexpr bool EqualLists<Pred, TList<As...>, TList<Bs...>> = (sizeof...(As) == sizeof...(Bs)) && (Pred<As, Bs>::Value && ... && true);

	template<typename T>
	struct GetTList;

	template<typename ...Args>
	struct GetTList<std::tuple<Args...>> {
		using Type = TList<Args...>;
	};

	template<template <typename> typename Pred, typename List, std::size_t Id>
	class MatchingImpl {
	public:
		using Values = TList<>;
		using Ids = SizeList<>;
	};

	template<template <typename> typename Pred, typename List>
	using Matching = MatchingImpl<Pred, List, 0>;

	template<template <typename> typename Pred, typename T, std::size_t Id, typename ...Ts>
	class MatchingImpl<Pred, TList<T, Ts...>, Id> {
	protected:
		using Next = MatchingImpl<Pred, TList<Ts...>, Id + 1>;
		using NextValues = typename Next::Values;
		using NextIds = typename Next::Ids;
	public:
		using Values = std::conditional_t<Pred<T>::Value, typename NextValues::template PushFront<T>, NextValues>;
		using Ids = std::conditional_t<Pred<T>::Value, typename NextIds::template PushFront<Id>, NextIds>;
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

		template<typename T>
		struct ArrayPred {
			static bool constexpr Value = IsArray<T>::Value;
		};

		using Matches = Matching<ArrayPred, List>;
		static constexpr bool HasArray = (Matches::Values::Size > 0);
		static constexpr std::size_t Id = HasArray ? Matches::Ids::Back : 0;
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

	template<typename A, typename B>
	struct AlgebraAddInfos {
		static_assert(SameScalarType<A, B>);
		static_assert(!Infos<A>::IsScalar || Infos<B>::IsScalar);

		using ReturnType = typename Infos<A>::QualifierFree;
		static constexpr Op OperatorAdd = SameSize<A, B> ? Op::CWiseAdd : Op::MatrixAddScalar;
		static constexpr Op OperatorSub = SameSize<A, B> ? Op::CWiseSub : Op::MatrixSubScalar;
	};
}
