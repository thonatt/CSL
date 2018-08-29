#pragma once

#include "Algebra.h"
//#include <type_traits>

struct shared;
struct packed;
struct std140;

struct BaseLayoutQualifier {};

template<unsigned int N> struct Component;
template<unsigned int N> struct Index;

template<int First, int Second> struct LastIfValid {
	static const int value = (Second >= 0 ? Second : First);
};
template<int First, int Second> constexpr int LastIfValidV = LastIfValid<First, Second>::value;

template<typename... Ts> struct AreLayoutQualifiers;
template<typename... Ts> constexpr bool AreLayoutQualifiersV = AreLayoutQualifiers<Ts...>::value;

template<> struct AreLayoutQualifiers<> {
	static const bool value = true;

};
template<typename T, typename... Ts> struct AreLayoutQualifiers<T, Ts...> {
	static const bool value = AreLayoutQualifiersV<T> && ( SizeOf<Ts...> == 0 ? true : AreLayoutQualifiersV<Ts...>);
};



#define GLSL_LAYOUT_QUALIFIER(Q)													\
																					\
template<int N> struct Q : BaseLayoutQualifier {									\
	static const int value = N;														\
};																					\
																					\
template<typename... Ts> struct Get##Q;												\
template<typename... Ts> constexpr int Get##Q##V = Get##Q<Ts...>::value;			\
																					\
template<typename T> struct Get##Q<T> {												\
	static const int value = -1;													\
};																					\
																					\
template<> struct Get##Q<> {														\
	static const int value = -1;													\
};																					\
																					\
template<int N> struct Get##Q<Q<N>> {												\
	static const int value = N;														\
};																					\
																					\
template<typename T, typename... Ts> struct Get##Q<T, Ts...> {						\
	static const int value = LastIfValidV<Get##Q##V<T>, Get##Q##V<Ts...> >;			\
};																					\
																					\
template<int N> struct AreLayoutQualifiers<Q<N>> {									\
	static const bool value = true;													\
};																					\

GLSL_LAYOUT_QUALIFIER(Offset);
GLSL_LAYOUT_QUALIFIER(Binding);
GLSL_LAYOUT_QUALIFIER(Location);

template<typename... LayoutQualifiers> struct Layout {
	static_assert(AreLayoutQualifiersV<LayoutQualifiers...>, "cannot use Layout with non LayoutQualifier types");

	//static const int binding = GetBindingV<LayoutQualifiers...>;
	//static const int location = GetLocationV<LayoutQualifiers...>;
	//static const int location = GetLocationV<LayoutQualifiers...>;
};

template<typename... LayoutQualifiers> struct LayoutQualifiersStr { 

	static void addToLayoutStr(const std::string & qualifierStr, int value, std::string & s, bool & first) {
		if (value >= 0) {
			if (!first) {
				s += ", ";
			}
			s += qualifierStr + " = " + std::to_string(value);
			first = false;
		}
	}

	static const std::string str() {
		std::string s = "";
		bool first = true;
		addToLayoutStr("binding", GetBindingV<LayoutQualifiers...>, s, first);
		addToLayoutStr("location", GetLocationV<LayoutQualifiers...>, s, first);
		addToLayoutStr("offset", GetOffsetV<LayoutQualifiers...>, s, first);

		return s;
	}
};


template<typename T> struct LayoutStrT {
	static const std::string str() { return ""; }
};

template<typename... LayoutQualifiers> struct LayoutStrT<Layout<LayoutQualifiers...> > {
	static const std::string str() {
		return SizeOf<LayoutQualifiers...> == 0 ? "" : ("layout(" + LayoutQualifiersStr<LayoutQualifiers...>::str() + ") ");
	}
};

template<typename T> struct IsLayout {
	static const bool value = false;
};
template<typename T> constexpr bool IsLayoutV = IsLayout<T>::value;

template<typename... LayoutQualifiers> struct IsLayout<Layout<LayoutQualifiers...> > {
	static const bool value = true;
};

template<typename Param, typename T, typename L = Layout<> > struct Qualifier : public T {
	static_assert(IsLayoutV<L>, "L must be a Layout");
	//using T::T;		

	Qualifier(const std::string & s = "") : T(s) {
		Ctx().removeCmd();
		Ctx().addCmd(strFromDecl(*this) + ";");
	}
	Qualifier(const T & t) : T(strFromObj(t)) {
		Ctx().removeCmd();
		release(t);
	}

	static const std::string typeStr() { return TypeStr<Qualifier>::str(); }
};

struct InParam {};
struct OutParam {};
struct UniformParam {};
template<> struct TypeStr<InParam> { static std::string str() { return "in"; } };
template<> struct TypeStr<OutParam> { static std::string str() { return "out"; } };
template<> struct TypeStr<UniformParam> { static std::string str() { return "uniform"; } };

template<typename Param, typename T, typename L> struct TypeStr<Qualifier<Param, T, L>> {
	static const std::string str() { return LayoutStrT<L>::str() + TypeStr<Param>::str() + " " + T::typeStr(); }
};
template<typename Param, typename L, numberType type, unsigned int N> struct AreValidVecCtorT<Qualifier<Param, Vec<type,N> , L>> {
	static const bool value = true;
};
template<typename Param, typename L, numberType type, unsigned int N> struct getVecSizeT<Qualifier<Param, Vec<type, N>, L>> {
	static const unsigned int value = N;
};

template<typename T, typename L = Layout<> > using In = Qualifier<InParam, T, L>;
template<typename T, typename L = Layout<> > using Out = Qualifier<OutParam, T, L>;
template<typename T, typename L = Layout<> > using Uniform = Qualifier<UniformParam, T, L>;

