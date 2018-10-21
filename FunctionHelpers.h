#pragma once

#include <functional>

template<typename T> struct FunctionReturnType {
	struct DummyType {};
	static_assert(std::is_same<DummyType, T>::value, "should not be here");
	//using type = void;
};

template<typename ReturnType, typename Fun, typename... Args>
struct FunctionReturnType< ReturnType(Fun::*)(Args...) const> {
	using type = std::function<ReturnType(Args...)>;
	using RType = ReturnType;
};

template<typename ReturnType, typename Fun>
struct FunctionReturnType< ReturnType(Fun::*)() const> {
	using type = std::function<ReturnType()>;
	using RType = ReturnType;
};

template<typename Lambda> typename FunctionReturnType<decltype(&Lambda::operator())>::type
functionFromLambda(const Lambda &func) {
	return func;
}

template<typename R, template<typename...> class Params, typename... Args, std::size_t... I>
void call_from_tuple_helper(std::function<R(Args...)> const&func, Params<Args...> const&params, std::index_sequence<I...>)
{
	func(std::get<I>(params)...);
}

template<typename R, template<typename...> class Params, typename... Args>
void call_from_tuple(const std::function<R(Args...)> &func, const Params<Args...> &params)
{
	call_from_tuple_helper(func, params, std::index_sequence_for<Args...>{});
}
