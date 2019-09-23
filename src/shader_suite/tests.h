#pragma once

#include <string>
#include <iostream>

void testSwitch();

void testStructsMacros();

void testArgCleaning();

void testInArgs();

//struct Arg {
//	Arg(int && t) : val(t) { std::cout << val << std::endl; }
//	~Arg() { std::cout << val << std::endl; }
//	operator int() const { return val; }
//	int val;
//};
//
//template<typename ...Args> struct Print;
//template<> struct Print<> {
//	static void run() {}
//};
//
//template<typename T, typename ... Ts> struct Print<T,Ts...> {
//	static void run(T && t, Ts && ... ts) {
//		std::cout << t << std::endl;
//		Print<Ts...>::run(ts...);
//	}
//};
//
//template<typename ...Args>
//void printArgs(Args && ... args) {
//	Print<Args...>::run(args...);
//}
//
//template< class, class = std::void_t<> > struct
//needs_unapply : std::true_type { };
//
//template< class T > struct
//needs_unapply<T, std::void_t<decltype(std::declval<T>()())>> : std::false_type { };
//
//template <typename F> auto
//curry(F&& f);
//
//template <bool> struct
//curry_on;
//
//template <> struct
//curry_on<false> {
//	template <typename F> static auto
//		apply(F&& f) {
//		return f();
//	}
//};
//
//template <> struct
//curry_on<true> {
//	template <typename F> 
//	static auto apply(F&& f) {
//		return [=](auto&& x) {
//			return curry( [=](auto&&...xs) -> decltype(f(x, xs...)) {
//				return f(x, xs...);
//			});
//		};
//	}
//};
//
//template <typename F> auto
//curry(F&& f) {
//	return curry_on<needs_unapply<decltype(f)>::value>::template apply(f);
//}

void testArgsOrder();