#pragma once

#include "NamedObjects.hpp"
#include "Types.hpp"

#include <stdexcept>
#include <type_traits>
#include <utility>

namespace csl {

	/////////////////////////////////////////////////////////////////////////////////////////////
	// general helpers

	template<typename T> struct FuncPtrInfo;

	template<typename ReturnType, typename Fun, typename... Args>
	struct FuncPtrInfo<ReturnType(Fun::*)(Args...) const> {
		using ArgTup = std::tuple<Args...>;
		using ArgTList = TList<Args...>;
		using RType = ReturnType;
	};

	template<typename F>
	using LambdaInfos = FuncPtrInfo<decltype(&F::operator())>;

	template<typename F>
	using GetArgTList = typename LambdaInfos<F>::ArgTList;

	template<typename F>
	using GetReturnType = typename LambdaInfos<F>::RType;

	/////////////////////////////////////////////////////////////////////////////////////////////
	// Overlad resolution helpers

	template<typename A, typename B>
	struct TypeMatchingPred {
		static constexpr bool Value = std::is_same_v<A, B> || (SameSize<A, B> && SameScalarType<A, B>);
	};

	template<typename ReturnTypeList, typename FList, typename ArgList>
	struct OverloadResolution {

		template<typename F>
		struct FunMatchingPred {
			static constexpr bool Value = EqualLists<TypeMatchingPred, GetArgTList<F>, ArgList>;
		};

		using Candidates = typename Matching<FunMatchingPred, FList>::Indexes;

		static_assert(Candidates::Size >= 1, "Invalid call, no overload candidate found");
		static_assert(Candidates::Size <= 1, "Ambiguous call, multiple overload candidates found");

		using ReturnType = typename ReturnTypeList::template At<Candidates::Front>;
	};

	/////////////////////////////////////////////////////////////////////////////////////////////
	// functor minimal number of argument, helpers from https://stackoverflow.com/a/57254989/4953963

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

	/////////////////////////////////////////////////////////////////////////////////////////////
	// functions argument evaluation order

	enum class ArgEvaluationOrder {
		LeftToRight, RightToLeft, NotSupported
	};

	struct ArgOrdering 
	{
		ArgOrdering(const std::size_t i) : value(i) {
			static size_t counter = 0;
			counter_value = counter;
			++counter;
		}

		static ArgEvaluationOrder call(ArgOrdering a, ArgOrdering b, ArgOrdering c) {
			ArgEvaluationOrder out;
			if (a.check(a) && b.check(b) && c.check(c)) {
				out = ArgEvaluationOrder::LeftToRight;
			} else if (a.check(c) && b.check(b) && c.check(a)) {
				out = ArgEvaluationOrder::RightToLeft;
			} else {
				out = ArgEvaluationOrder::NotSupported;
				throw std::runtime_error("ArgEvaluationOrder is not supported");
			}
			return out;
		}

		bool check(const ArgOrdering& other) const { return value == other.counter_value; }

		std::size_t value, counter_value;

	};

	inline ArgEvaluationOrder get_arg_evaluation_order() {
		static const ArgEvaluationOrder order = ArgOrdering::call(0, 1, 2);
		return order;
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	// function objects

	struct FuncBase : NamedObjectBase
	{
		FuncBase() : NamedObjectBase(ObjFlags::None) {}
	};

	template<typename ReturnTList, typename ... Fs>
	struct Function : FuncBase
	{
		using FuncTList = TList<Fs...>;

		static_assert(ReturnTList::Size == FuncTList::Size, "Number of overloads and return types dont match");
		static_assert(FuncTList::Size > 0, "Functions must have at least one overload");
		static_assert(((std::is_same_v<GetReturnType<std::remove_reference_t<Fs>>, void>)&& ...), "C++ return detected in CSL function, use CSL_RETURN instead");

		Function(const std::string& name, Fs&& ... fs);

		template<typename ... Args>
		using ReturnType = typename OverloadResolution<ReturnTList, FuncTList, TList<std::remove_reference_t<Args>...>>::ReturnType;

		template<typename ... Args>
		ReturnType<Args...> operator()(Args&& ...args);
	};

	template<typename ... ReturnTypes, typename ... Fs >
	Function<TList<ReturnTypes...>, Fs... > define_function(const std::string& name, Fs&& ... fs)
	{
		return { name, std::forward<Fs>(fs)... };
	}

	template<typename ... ReturnTypes, typename F, typename ... Fs, typename = std::enable_if_t<!std::is_convertible_v<F, std::string> > >
	Function<TList<ReturnTypes...>, F, Fs... > define_function(F&& f, Fs&& ... fs)
	{
		return { "", std::forward<F>(f), std::forward<Fs>(fs)... };
	}

}
