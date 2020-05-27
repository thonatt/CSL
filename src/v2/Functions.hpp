#pragma once

#include "NamedObjects.hpp"
#include "Types.hpp"

#include <utility>

namespace v2 {

	/////////////////////////////////////////////////////////////////////////////////////////////
	// general helpers

	template<typename T> struct FuncPtrInfo;

	template<typename ReturnType, typename Fun, typename... Args>
	struct FuncPtrInfo<ReturnType(Fun::*)(Args...) const> {
		using ArgTup = std::tuple<Args...>;
		using ArgTList = TList<Args...>;
	};

	template<typename F>
	using LambdaInfos = FuncPtrInfo<decltype(&F::operator())>;

	template<typename F>
	using GetArgTList = typename LambdaInfos<F>::ArgTList;

	/////////////////////////////////////////////////////////////////////////////////////////////
	// Overlad resolution helpers

	template<typename A, typename B>
	struct TypeMatchingPred {
		static constexpr bool Value = SameSize<A, B> && SameScalarType<A, B>;
	};

	template<typename F, typename ArgList>
	struct FunCallMatchingPred {
		static constexpr bool Value = EqualLists<TypeMatchingPred, GetArgTList<F>, ArgList>;
	};

	template<typename ReturnTypeList, typename FList, typename ArgList>
	struct OverloadResolution {

		template<typename F>
		using FunMatchingPred = FunCallMatchingPred<F, ArgList>;

		using Candidates = typename Matching<FunMatchingPred, FList>::Ids;

		static_assert(Candidates::Size >= 1, "No overload candidate found");
		static_assert(Candidates::Size <= 1, "Multiple overload candidates found");
		
		using ReturnType = typename ReturnTypeList::GetType<Candidates::Front>;
	};

	//using A = Matrix<float, 1, 1>;
	//using B = Matrix<double, 1, 1>;
	//constexpr bool v = TypeMatchingPred<A, B>::Value;


	//constexpr bool value = EqualLists<TypeMatchingPred, TList<B, B>, TList<A, B>>;

	//constexpr auto f = [](int a, double b) {};
	//constexpr auto g = [](int a, int b) {};

	//using F = decltype(f);
	//using G = decltype(g);
	//using ArgList = TList<int, int>;

	//template<typename F>
	//struct FPred {
	//	static constexpr bool Value = EqualLists<TypeMatchingPred, GetArgTList<F>, ArgList>;
	//};

	//Matching<FPred, TList<F>>;
	//Matching<FPred, TList<F, G>>;

	//OverloadResolution<TList<bool, char>, TList<F, G>, TList<int, int>>::ReturnType;


	/////////////////////////////////////////////////////////////////////////////////////////////
	//get minimal number of argument helpers from https://stackoverflow.com/a/57254989/4953963

	template <typename F, typename T, std::size_t ... Is>
	constexpr auto lambda_min_number_of_args_helper(std::index_sequence<Is...> is, int)
		-> decltype((void)std::declval<F>() (std::declval<std::tuple_element_t<Is, T>>()...), std::size_t{})
	{
		return sizeof...(Is);
	}

	template <typename F, typename T, std::size_t ... Is>
	constexpr auto lambda_min_number_of_args_helper(std::index_sequence<Is...>, long)
	{
		return lambda_min_number_of_args_helper<F, T>(std::make_index_sequence<sizeof...(Is) + 1u>{}, 0);
	}

	template <typename F, typename ArgList>
	constexpr std::size_t get_min_number_of_args()
	{
		return lambda_min_number_of_args_helper<F, ArgList>(std::index_sequence<>{}, 0);
	}

	template<typename F>
	constexpr size_t min_number_of_args() {
		return get_min_number_of_args<F, typename LambdaInfos<F>::ArgTup>();
	}

	template<typename F, typename ...Args>
	void call_with_first_args_empty(const F& f, TList<Args...>) {
		f(Args()...);
	}

	template<typename F>
	void call_with_only_non_default_args(const F& f) {
		call_with_first_args_empty(f, Subset<GetArgTList<F>, 0, min_number_of_args<F>() >{});
	}

	///////////////////////////


}
