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

	template<typename T, std::size_t Id>
	struct GetValueAtImpl;

	template<typename T, std::size_t Id>
	constexpr std::size_t GetValueAt = GetValueAtImpl<T, Id>::Value;

	template<std::size_t N, std::size_t ...Ns>
	struct GetValueAtImpl<SizeList<N, Ns...>, 0> {
		static constexpr std::size_t Value = N;
	};

	template<std::size_t Id, std::size_t N, std::size_t ...Ns>
	struct GetValueAtImpl<SizeList<N, Ns...>, Id> {
		static constexpr std::size_t Value = GetValueAt<SizeList<Ns...>, Id - 1>;
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
	struct TList;

	template<>
	struct TList<> {
		using Tuple = std::tuple<>;

		using Front = void;
		using Tail = TList<>;

		template<std::size_t Id>
		using GetType = void;

		template<typename T>
		using PushFront = TList<T>;

		template<typename T>
		using PushBack = TList<T>;

		static constexpr std::size_t Size = 0;
	};


	template<typename T, typename ... Ts>
	struct TList<T, Ts...> {
		using Tuple = std::tuple<T, Ts...>;

		using Front = T;
		using Tail = TList<Ts...>;

		template<std::size_t Id>
		using GetType = typename std::tuple_element<Id, Tuple>::type;

		template<typename U>
		using PushFront = TList<U, T, Ts...>;

		template<typename U>
		using PushBack = TList<Ts..., T, U>;

		static constexpr std::size_t Size = 1 + sizeof...(Ts);
	};

	template<typename TList, template<typename, std::size_t> typename F>
	struct IterateOverListImpl {
		template<std::size_t ...Ns, typename ...Args>
		static void call(std::index_sequence<Ns...>, Args&& ... args) {
			(F<typename TList::template GetType<Ns>, Ns>::call(std::forward<Args>(args)...), ...);
		}
	};

	template<typename TList, template<typename, std::size_t> typename F, typename ...Args>
	void iterate_over_typelist(Args&& ...args) {
		IterateOverListImpl<TList, F>::call(std::make_index_sequence<TList::Size>{}, std::forward<Args>(args)...);
	}

	template<typename List, std::size_t first, typename Range>
	struct SubsetImpl;

	template<typename List, size_t first, size_t last>
	using Subset = typename SubsetImpl<List, first, ::std::make_index_sequence<last - first>>::Type;

	template<size_t first, size_t ... Is, typename ... Ts>
	struct SubsetImpl<TList<Ts...>, first, ::std::index_sequence<Is...>> {
		using Type = TList<::std::tuple_element_t<first + Is, ::std::tuple<Ts...>> ...>;
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

	template<typename IdList, typename List, std::size_t Id>
	struct RemoveAtImpl {
		using Type = List;
	};

	template<typename IdList, typename List>
	using RemoveAt = typename RemoveAtImpl<IdList, List, 0>::Type;

	template<std::size_t Id, std::size_t ...Ids, typename T, typename ...Ts, std::size_t CurrentId>
	struct RemoveAtImpl<SizeList<Id, Ids...>, TList<T, Ts...>, CurrentId> {
		static constexpr bool RemoveCurrent = (Id == CurrentId);
		using NextIds = std::conditional_t<RemoveCurrent, SizeList<Ids...>, SizeList<Id, Ids...>>;
		using Next = typename RemoveAtImpl<NextIds, TList<Ts...>, CurrentId + 1>::Type;
		using Type = std::conditional_t<RemoveCurrent, Next, typename Next::template PushFront<T>>;
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

	template<typename T, typename Ds, typename ... Qs>
	struct ArrayInterface;

	template<typename T, typename ...Qs>
	struct TypeInterface;

	enum class ObjFlags : std::size_t;

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
		static constexpr std::size_t Id = (HasArray ? Matches::Ids::Back : 0);
		using Dimensions = std::conditional_t<HasArray, typename IsArray<typename List::template GetType<Id>>::Dimensions, SizeList<>>;
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
		static constexpr bool IsValid = false;
		static constexpr bool IsArray = false;
	};

	template<typename T>
	struct Infos<const T> : Infos<T> {
		static constexpr bool IsConst = true;
	};

	template<typename T>
	struct Infos<T&> : Infos<T> { };

	template<typename T>
	struct Infos<const T&> : Infos<T> { };

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

		static constexpr bool IsBool = std::is_same_v<T, bool>;
		static constexpr bool IsFloating = std::is_same_v<T, float> || std::is_same_v<T, double>;
		static constexpr bool IsInteger = std::is_same_v<T, int> || std::is_same_v<T, uint>;

		static constexpr bool IsValid = true;

		using ArrayDimensions = typename ArrayInfos<Qs...>::Dimensions;

		using Type = Matrix<T, R, C, Qs...>;
		using ScalarType = T;
		using QualifierFree = Matrix<T, R, C>;
	};

	template<typename T, typename Ds, typename ... Qs>
	struct Infos<ArrayInterface<T, Ds, Qs...>> : Infos<T> {
		using ArrayDimensions = Ds;
	};

	template<typename T, typename ...Qs>
	struct Infos<TypeInterface<T, Qs...>> : Infos<T> {
	};


	template<typename T, typename Ds, std::size_t R, std::size_t C, typename ...Qs>
	struct Infos<MatrixArray<T, Ds, R, C, Qs...>> : Infos<Matrix<T, R, C, Qs...>> {
		//TODO fix me
	};

	//////////////////////////////////////////////////////////////

	template<>
	struct Infos<Expr> {
		static constexpr std::size_t NumElements = 0;
		using ScalarType = void;
	};

	template<>
	struct Infos<ObjFlags> {
		static constexpr std::size_t NumElements = 0;
		using ScalarType = void;
	};

	template<std::size_t N>
	struct Infos<const char(&)[N]> {
		static constexpr std::size_t NumElements = 0;
		using ScalarType = void;
		static constexpr std::size_t RowCount = 0;
		static constexpr std::size_t ColCount = 0;
	};

	template<>
	struct Infos<std::string> {
		static constexpr std::size_t NumElements = 0;
		using ScalarType = void;
		static constexpr std::size_t RowCount = 0;
		static constexpr std::size_t ColCount = 0;
	};

	////////////////////////////////////////////////////////////////////////

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

	template<typename A, typename B>
	constexpr bool SameDimensions = std::is_same_v<typename Infos<A>::ArrayDimensions, typename Infos<B>::ArrayDimensions>;

	template<typename A, typename B>
	constexpr bool SameType = std::is_same_v<A, B> || (SameSize<A, B> && SameScalarType<A, B> && SameDimensions<A, B>);

	template<typename T>
	constexpr bool IsInteger = Infos<T>::IsInteger;

	template<>
	struct Infos<double> : Infos<Matrix<float, 1, 1>> {
		static constexpr bool IsConstant = true;
		static constexpr bool IsValid = true;
	};

	template<>
	struct Infos<float> : Infos<Matrix<float, 1, 1>> {
		static constexpr bool IsConstant = true;
		static constexpr bool IsValid = true;
	};

	template<>
	struct Infos<int> : Infos<Matrix<int, 1, 1, TList<>>> {
		static constexpr bool IsConstant = true;
		static constexpr bool IsValid = true;
	};

	template<>
	struct Infos<bool> : Infos<Matrix<bool, 1, 1, TList<>>> {
		static constexpr bool IsConstant = true;
		static constexpr bool IsValid = true;
	};

	//template<>
	//struct Infos<void> : Infos<Matrix<void, 0, 0>> {
	//};

	template<typename A, typename B>
	struct AlgebraMulInfos {
		static_assert(SameScalarType<A, B>);
		static_assert(!Infos<A>::IsScalar || Infos<B>::IsScalar);

		using ScalarType = typename Infos<A>::ScalarType;
		static constexpr std::size_t OutRowCount = Infos<A>::RowCount;
		static constexpr std::size_t OutColCount = Infos<std::conditional_t<Infos<B>::IsScalar, A, B>>::ColCount;
		using ReturnType = Matrix<ScalarType, OutRowCount, OutColCount>;

		static constexpr Op Operator = (SameSize<A, B> && Infos<A>::IsVec) ? Op::CWiseMul : (Infos<B>::IsScalar ? Op::MatrixTimesScalar : Op::MatrixTimesMatrix);
	};

	template<typename A, typename B>
	struct AlgebraAddInfos {
		static_assert(SameScalarType<A, B>);
		static_assert(!Infos<A>::IsScalar || Infos<B>::IsScalar);

		using ReturnType = typename Infos<A>::QualifierFree;
		static constexpr Op OperatorAdd = SameSize<A, B> ? Op::CWiseAdd : Op::MatrixAddScalar;
		static constexpr Op OperatorSub = SameSize<A, B> ? Op::CWiseSub : Op::MatrixSubScalar;
	};

	template<typename ...Qs>
	using RemoveArrayFromQualifiers = RemoveAt<typename Matching<IsArray, TList<Qs...>>::Ids, TList<Qs...>>;

	template<typename T, typename ... Qs>
	struct QualifiedIndirection;

	template<typename T, typename ... Qs>
	using Qualify = std::conditional_t<
		ArrayInfos<Qs...>::Value,
		ArrayInterface<T, typename ArrayInfos<Qs...>::Dimensions, RemoveArrayFromQualifiers<Qs...> >,
		TypeInterface<T, RemoveArrayFromQualifiers<Qs...> >
	>;

	//template<typename T, typename ... Qs>
	//using Qualify = typename QualifiedIndirection<typename Infos<T>::Type, Qs...>::Type;

}
