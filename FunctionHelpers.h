#pragma once

#include <functional>

namespace csl {

	template<typename T> struct FunctionReturnType;

	template<typename ReturnType, typename Fun, typename... Args>
	struct FunctionReturnType< ReturnType(Fun::*)(Args...) const> {
		using type = const std::function<ReturnType(Args...)>;
		using RType = ReturnType;
	};

	template<typename Lambda>
	typename FunctionReturnType<decltype(&Lambda::operator())>::type
		functionFromLambda(const Lambda &func) {
		return func;
	}

	template<typename R, template<typename...> class Params, typename... Args, std::size_t... I>
	void call_from_tuple_helper(const std::function<R(Args...)> & func, const Params<Args...> & params, std::index_sequence<I...>)
	{
		func(std::get<I>(params)...);
	}

	template<typename R, template<typename...> class Params, typename... Args>
	void call_from_tuple(const std::function<R(Args...)> &func, const Params<Args...> & params)
	{
		call_from_tuple_helper(func, params, std::index_sequence_for<Args...>{});
	}

} //namespace csl