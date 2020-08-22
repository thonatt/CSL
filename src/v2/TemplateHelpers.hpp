#pragma once

#include <cstdint>
#include <tuple>
#include <utility>

namespace v2 {

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
		using PushBack = TList<T, Ts..., U>;

		static constexpr std::size_t Size = 1 + sizeof...(Ts);
	};

	template<typename TList, template<typename, std::size_t> typename F>
	struct IterateOverListImpl {
		template<std::size_t ...Ns, typename ...Args>
		static void call(std::index_sequence<Ns...>, Args&& ... args) {
			(F<typename TList::template GetType<Ns>, Ns>::call(std::forward<Args>(args)...), ...);
		}
	};

	// Iterate over type list using functor taking type and index as template argument
	template<typename TList, template<typename, std::size_t> typename F, typename ...Args>
	void iterate_over_typelist(Args&& ...args) {
		IterateOverListImpl<TList, F>::call(std::make_index_sequence<TList::Size>{}, std::forward<Args>(args)...);
	}

	template<typename List, std::size_t first, typename Range>
	struct SubsetImpl;

	// Extract type list subset given first (inclusive) and last (exclusive) indexesù
	template<typename List, size_t first, size_t last>
	using Subset = typename SubsetImpl<List, first, ::std::make_index_sequence<last - first>>::Type;

	template<size_t first, size_t ... Is, typename ... Ts>
	struct SubsetImpl<TList<Ts...>, first, ::std::index_sequence<Is...>> {
		using Type = TList<::std::tuple_element_t<first + Is, ::std::tuple<Ts...>> ...>;
	};

	// Convert tuple to type list
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

	// Match a type over a type list using a predicates, defines list of matched types (::Values) and their indexes (::Ids) from the input type list
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

	// Remove type from type list given index
	template<typename IdList, typename List>
	using RemoveAt = typename RemoveAtImpl<IdList, List, 0>::Type;

	template<std::size_t Id, std::size_t ...Ids, typename T, typename ...Ts, std::size_t CurrentId>
	struct RemoveAtImpl<SizeList<Id, Ids...>, TList<T, Ts...>, CurrentId> {
		static constexpr bool RemoveCurrent = (Id == CurrentId);
		using NextIds = std::conditional_t<RemoveCurrent, SizeList<Ids...>, SizeList<Id, Ids...>>;
		using Next = typename RemoveAtImpl<NextIds, TList<Ts...>, CurrentId + 1>::Type;
		using Type = std::conditional_t<RemoveCurrent, Next, typename Next::template PushFront<T>>;
	};

}
