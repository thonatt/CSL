#pragma once

#include <cstdint>
#include <utility>

namespace csl
{
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

		template<std::size_t Index>
		static constexpr std::size_t At = (Index == 0 ? N : Tail::template At<Index - 1>);
	};

	//////////////////////////////////////////////////////////////////////////
	// Type list utils

	template<typename ... Ts>
	struct TList;

	template<>
	struct TList<>
	{
		using Front = void;
		using Tail = TList<>;

		template<std::size_t Index>
		using At = void;

		template<typename T>
		using PushFront = TList<T>;

		template<typename T>
		using PushBack = TList<T>;

		template<typename T>
		static constexpr bool Contains = false;

		static constexpr std::size_t Size = 0;
	};

	template<typename T, typename ... Ts>
	struct TList<T, Ts...>
	{
		using Front = T;
		using Tail = TList<Ts...>;

		template<std::size_t Index>
		using At = std::conditional_t<Index == 0, T, typename Tail::template At<Index - 1>>;

		template<typename U>
		using PushFront = TList<U, T, Ts...>;

		template<typename U>
		using PushBack = TList<T, Ts..., U>;

		template<typename U>
		static constexpr bool Contains = std::is_same_v<U, T> || ((std::is_same_v<U, Ts>) || ...);

		static constexpr std::size_t Size = 1 + sizeof...(Ts);
	};

	template<typename TListUnique, typename TListRemaining>
	struct RemoveDuplicatesImpl;

	template<typename InputTList>
	using RemoveDuplicates = typename RemoveDuplicatesImpl<TList<>, InputTList>::Type;

	template<typename ...Ts>
	struct RemoveDuplicatesImpl<TList<Ts...>, TList<>>
	{
		using Type = TList<Ts...>;
	};

	template<typename ...Ts, typename T, typename ...Us>
	struct RemoveDuplicatesImpl<TList<Ts...>, TList<T, Us...>>
	{
		using Current = TList<Ts...>;
		using Next = std::conditional_t<Current::template Contains<T>, Current, typename Current::template PushBack<T>>;
		using Type = typename RemoveDuplicatesImpl<Next, TList<Us...>>::Type;
	};
	
	template<typename TList, template<typename, std::size_t> typename F>
	struct IterateOverListImpl {
		template<std::size_t ...Ns, typename ...Args>
		static void call(std::index_sequence<Ns...>, Args&& ... args) {
			(F<typename TList::template At<Ns>, Ns>::call(std::forward<Args>(args)...), ...);
		}
	};

	// Iterate over type list using functor taking type and index as template argument.
	template<typename TList, template<typename, std::size_t> typename F, typename ...Args>
	void iterate_over_typelist(Args&& ...args) {
		IterateOverListImpl<TList, F>::call(std::make_index_sequence<TList::Size>{}, std::forward<Args>(args)...);
	}

	template<typename List, std::size_t First, typename Range>
	struct SubsetImpl;

	// Extract type list subset given first (inclusive) and last (exclusive) indexes.
	template<typename List, size_t First, size_t Last>
	using Subset = typename SubsetImpl<List, First, std::make_index_sequence<Last - First>>::Type;

	template<std::size_t First, std::size_t ... Is, typename ... Ts>
	struct SubsetImpl<TList<Ts...>, First, std::index_sequence<Is...>> {
		using Type = TList<typename TList<Ts...>::template At<First + Is>...>;
	};

	template<template <typename> typename Pred, typename List, std::size_t Index>
	class MatchingImpl {
	public:
		using Values = TList<>;
		using Indexes = SizeList<>;
	};

	// Match a type over a type list using a predicate (must implement a constexpr bool ::Value).
	// Return a list of matched types (::Values) and their indexes (::Indexes) from the input type list.
	template<template <typename> typename Pred, typename List>
	using Matching = MatchingImpl<Pred, List, 0>;

	template<template <typename> typename Pred, typename T, std::size_t Index, typename ...Ts>
	class MatchingImpl<Pred, TList<T, Ts...>, Index>
	{
	protected:
		using Next = MatchingImpl<Pred, TList<Ts...>, Index + 1>;
		using NextValues = typename Next::Values;
		using NextIndexes = typename Next::Indexes;
	public:
		using Values = std::conditional_t<Pred<T>::Value, typename NextValues::template PushFront<T>, NextValues>;
		using Indexes = std::conditional_t<Pred<T>::Value, typename NextIndexes::template PushFront<Index>, NextIndexes>;
	};

	template<typename IndexList, typename TypeList, std::size_t Index>
	struct RemoveAtImpl {
		using Type = TypeList;
	};

	// Remove type from type list given index.
	template<typename IndexList, typename TypeList>
	using RemoveAt = typename RemoveAtImpl<IndexList, TypeList, 0>::Type;

	template<std::size_t Index, std::size_t ...Indexes, typename T, typename ...Ts, std::size_t CurrentIndex>
	struct RemoveAtImpl<SizeList<Index, Indexes...>, TList<T, Ts...>, CurrentIndex> {
		static constexpr bool RemoveCurrent = (Index == CurrentIndex);
		using NextIds = std::conditional_t<RemoveCurrent, SizeList<Indexes...>, SizeList<Index, Indexes...>>;
		using Next = typename RemoveAtImpl<NextIds, TList<Ts...>, CurrentIndex + 1>::Type;
		using Type = std::conditional_t<RemoveCurrent, Next, typename Next::template PushFront<T>>;
	};

}
