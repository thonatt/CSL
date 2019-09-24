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

	template<typename FList> struct CallFuncs;

	template<> struct CallFuncs<TList<>> {
		static void run() {}
	};	

	template<typename ReturnTList, typename ... Fs>
	void init_function_declaration(const std::string & fname, const Fs & ...fs);

	struct FuncBase : NamedObject<FuncBase> {
		FuncBase(const std::string & s = "") : NamedObject<FuncBase>(s, 0) {}
		static std::string typeStr(int trailing = 0) { return "function"; }
		static std::string typeNamingStr(int trailing = 0) { return typeStr(trailing); }
	};
	template<> struct AutoNaming<FuncBase> {
		using Type = NamingCounter<NamingCaterogy::FUNCTION>;
	};

	template<typename ReturnTList, typename FuncTList>
	struct Function : FuncBase {
		static_assert(ReturnTList::size == FuncTList::size, "numbers of overload and return type dont match");

		template<typename ... Fs>
		Function(const std::string & _name, const Fs & ... _fs) : FuncBase(_name) {
			init_function_declaration<ReturnTList>(str(), _fs ...);
		}

		template<typename ... Args>
		using ReturnType = OverloadResolutionType<ReturnTList, FuncTList, TList<std::remove_reference_t<Args>...>>;

		template<typename ... Args>
		ReturnType<Args...> operator()(Args && ...args);
	};

	template<typename ... ReturnTypes, typename F, typename ... Fs >
	Function<TList<ReturnTypes...>, TList<F, Fs...> > declareFunc(const std::string & name, const F & f, const Fs & ... fs) {
		return Function<TList<ReturnTypes...>, TList<F, Fs...> >(name, f, fs...);
	}

	template<typename ... ReturnTypes, typename F, typename ... Fs, typename = std::enable_if_t<!std::is_convertible<F, std::string>::value > >
	Function<TList<ReturnTypes...>, TList<F, Fs...> > declareFunc(const F & f, const Fs & ... fs) {
		return Function<TList<ReturnTypes...>, TList<F, Fs...> >("", f, fs...);
	}

} //namespace csl
