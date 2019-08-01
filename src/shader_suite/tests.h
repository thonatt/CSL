#pragma once

#include <string>
#include <iostream>

void testSwitch();

void testStructsMacros();

void testArgCleaning();

enum ArgOrderEvaluation { LEFT_TO_RIGHT, RIGHT_TO_LEFT, ORDER_NOT_SUPPORTED };

struct ArgOrdering {
	ArgOrdering(size_t i) : value(i) {
		static size_t counter = 0;
		counter_value = counter;
		++counter;
	}

	static ArgOrderEvaluation call_args(ArgOrdering a, ArgOrdering b, ArgOrdering c) {
		ArgOrderEvaluation out;
		if (a.check(a) && b.check(b) && c.check(c)) {
			out = LEFT_TO_RIGHT;
		} else  if (a.check(c) && b.check(b) && c.check(a)) {
			out = RIGHT_TO_LEFT;
		} else {
			out = ORDER_NOT_SUPPORTED;
		}
		return out;
	}
	
	bool check(const ArgOrdering & other) const { return value == other.counter_value; }

	size_t value, counter_value;
	
};

inline ArgOrderEvaluation getArgOrder() {
	static bool first = true;
	static ArgOrderEvaluation order;
	if (first) {
		order = ArgOrdering::call_args(ArgOrdering(0), ArgOrdering(1), ArgOrdering(2));
	} 
	return order;
}

inline void getArgOrderStr() {
	ArgOrderEvaluation order = getArgOrder();
	std::string out;
	if (order == LEFT_TO_RIGHT) {
		out = "left to right";
	} else if (order == RIGHT_TO_LEFT) {
		out = "right to left";
	} else {
		out = "order no not supported";
	}
	std::cout << out << std::endl;
}

struct Arg {
	Arg(int && t) : val(t) { std::cout << val << std::endl; }
	~Arg() { std::cout << val << std::endl; }
	operator int() const { return val; }
	int val;
};

template<typename ...Args> struct Print;
template<> struct Print<> {
	static void run() {}
};

template<typename T, typename ... Ts> struct Print<T,Ts...> {
	static void run(T && t, Ts && ... ts) {
		std::cout << t << std::endl;
		Print<Ts...>::run(ts...);
	}
};

template<typename ...Args>
void printArgs(Args && ... args) {
	Print<Args...>::run(args...);
}

template< class, class = std::void_t<> > struct
needs_unapply : std::true_type { };

template< class T > struct
needs_unapply<T, std::void_t<decltype(std::declval<T>()())>> : std::false_type { };

template <typename F> auto
curry(F&& f);

template <bool> struct
curry_on;

template <> struct
curry_on<false> {
	template <typename F> static auto
		apply(F&& f) {
		return f();
	}
};

template <> struct
curry_on<true> {
	template <typename F> 
	static auto apply(F&& f) {
		return [=](auto&& x) {
			return curry( [=](auto&&...xs) -> decltype(f(x, xs...)) {
				return f(x, xs...);
			});
		};
	}
};

template <typename F> auto
curry(F&& f) {
	return curry_on<needs_unapply<decltype(f)>::value>::template apply(f);
}

void testArgsOrder();