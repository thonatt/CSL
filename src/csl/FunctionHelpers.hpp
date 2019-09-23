#pragma once

#include <functional>
#include "TypesHelpers.hpp"

namespace csl {

	/////////////////////////////////////////////////////////////////////////////////////////////
	//get minimal number of argument helpers from https://stackoverflow.com/a/57254989/4953963

	template <typename F, typename T, std::size_t ... Is>
	constexpr auto lambda_minumnarg_helper(std::index_sequence<Is...> is, int)
		-> decltype((void)std::declval<F>() (std::declval<std::tuple_element_t<Is, T>>()...), std::size_t{})
	{
		return sizeof...(Is);
	}

	template <typename F, typename T, std::size_t ... Is>
	constexpr auto lambda_minumnarg_helper(std::index_sequence<Is...>, long)
	{
		return lambda_minumnarg_helper<F, T>(std::make_index_sequence<sizeof...(Is) + 1u>{}, 0);
	}

	template <typename F, typename ArgList>
	constexpr std::size_t getMinNumArgs()
	{
		return lambda_minumnarg_helper<F, ArgList>(std::index_sequence<>{}, 0);
	}


	//template<typename T> struct FunctionReturnType;

	//template<typename ReturnType, typename Fun, typename... Args>
	//struct FunctionReturnType< ReturnType(Fun::*)(Args...) const> {
	//	using type = const std::function<ReturnType(Args...)>;
	//	using RType = ReturnType;
	//	using Tup = std::tuple<Args...>;
	//	using ArgTList = TList<Args...>;
	//};

	//template<typename Lambda>
	//typename FunctionReturnType<decltype(&Lambda::operator())>::type
	//	functionFromLambda(const Lambda &func) {
	//	return func;
	//}

	//template<typename R, template<typename...> class Params, typename... Args, std::size_t... I>
	//void call_from_tuple_helper(const std::function<R(Args...)> & func, const Params<Args...> & params, std::index_sequence<I...>)
	//{
	//	func(std::get<I>(params)...);
	//}

	//template<typename R, template<typename...> class Params, typename... Args>
	//void call_from_tuple(const std::function<R(Args...)> &func, const Params<Args...> & params)
	//{
	//	call_from_tuple_helper(func, params, std::index_sequence_for<Args...>{});
	//}
	template<typename F>
	constexpr size_t min_num_args() {
		return getMinNumArgs<F, typename LambdaInfos<F>::ArgTup>();
	}

	template<typename F, typename ...Args>
	void call_with_first_args_empty(const F & f, TList<Args...>) {
		f(Args()...);
	}

	template<typename F>
	void call_with_only_non_default_args(const F & f) {
		//std::cout << "num args : " << min_num_args<F>() << "/" << GetArgTList<F>::size << std::endl;
		call_with_first_args_empty(f, Subset<GetArgTList<F>, 0, min_num_args<F>() >{});
	}

	

} //namespace csl
