#pragma once

#include "MatrixTypesTest.h"

template<LayoutArgBoolType layoutArg, bool b>
struct LayoutArgBool {
};


using Shared = LayoutArgBool<SHARED, true>;
using Packed = LayoutArgBool<PACKED, true>;
using std140 = LayoutArgBool<STD140, true>;
using std430 = LayoutArgBool<STD430, true>;


template<LayoutArgIntType layoutArg, int N>
struct LayoutArgInt {
};

template<int N> using Offset = LayoutArgInt<OFFSET, N>;
template<int N> using Binding = LayoutArgInt<BINDING, N>;
template<int N> using Location = LayoutArgInt<LOCATION, N>;

template<LayoutArgIntType type, typename T> struct ExtractInt {
	static const int value = -1;
};

template<LayoutArgIntType type, template <LayoutArgIntType, int> typename T, int N>
struct ExtractInt<type,T<type, N>> {
	static const int value = N;
};

template<LayoutArgIntType type, typename ...Ts> struct GetLayoutArg;


template<LayoutArgIntType type>
struct GetLayoutArg<type> {
	static const int value = -1;
};

template<LayoutArgIntType type, typename T>
struct GetLayoutArg<type, T> {
	static const int value = ExtractInt<type, T>::value;
};

template<LayoutArgIntType type, typename T, typename ... Ts>
struct GetLayoutArg<type, T, Ts...> {
	static const int value = Last<GetLayoutArg<type, T>::value, GetLayoutArg<type, Ts...>::value>;
};

template<typename ... LayoutCleanedArgs> struct LayoutCleanedArg {};

template<typename ... LayoutArgs>
struct Layout {
	
	static const int offset = GetLayoutArg<OFFSET, LayoutArgs...>::value;
	static const int binding = GetLayoutArg<BINDING, LayoutArgs...>::value;
	static const int location = GetLayoutArg<LOCATION, LayoutArgs...>::value;

	static const bool is_std140 = AnyTrue<std::is_same<LayoutArgs, std140>::value...>;
	static const bool is_std430 = AnyTrue<std::is_same<LayoutArgs, std430>::value...>;
	static const bool is_shared = AnyTrue<std::is_same<LayoutArgs, Shared>::value...>;
	static const bool is_packed = AnyTrue<std::is_same<LayoutArgs, Packed>::value...>;

	using CleanedArgs = LayoutCleanedArg<
		Offset<offset>,
		Binding<binding>,
		Location<location>,
		LayoutArgBool<STD140,is_std140>,
		LayoutArgBool<STD430,is_std430>,
		LayoutArgBool<PACKED,is_packed>,
		LayoutArgBool<SHARED,is_shared>
	>;

	static const bool empty = (offset < 0) && (binding < 0) && (location < 0) && !( is_std140 || is_std430 || is_shared || is_packed);
};

template<QualifierType qType, typename T, typename ... LayoutArgs>
struct Qualifier< qType, T, Layout<LayoutArgs...>> : public T {
	
	using UnderlyingType = T;

	Qualifier(const std::string & s = "") : T(s,NOT_TRACKED) {
		NamedObjectBase::exp = createDeclaration<Qualifier>(NamedObjectBase::myNamePtr());
	}

	template<typename R_A, typename A = CleanType<R_A>, typename = std::enable_if_t<EqualMat<A,T> > >
	void operator=(R_A && other) const & {
		listen().addEvent(createExp(std::make_shared<SingleCharBinaryOp<'=', NO_PARENTHESIS>>(),
			getExp<Qualifier, false>(*this),
			getExp<R_A>(other))
		);
	}

	//static const int offset = Layout<LayoutArgs...>::offset;
	//static const int binding = Layout<LayoutArgs...>::binding;
	//static const int location = Layout<LayoutArgs...>::location;
};

template<typename T, typename L = Layout<> > using Uniform = Qualifier<UNIFORM, T, L>;
template<typename T, typename L = Layout<>> using In = Qualifier<IN, T, L>;
template<typename T, typename L =  Layout<>> using Out = Qualifier<OUT, T, L>;

//
//#include "Algebra.h"
////#include <type_traits>
//
//struct shared;
//struct packed;
//struct std140;
//
//struct BaseLayoutQualifier {};
//
//template<unsigned int N> struct Component;
//template<unsigned int N> struct Index;
//
//template<int First, int Second> struct LastIfValid {
//	static const int value = (Second >= 0 ? Second : First);
//};
//template<int First, int Second> constexpr int LastIfValidV = LastIfValid<First, Second>::value;
//
//template<typename... Ts> struct AreLayoutQualifiers;
//template<typename... Ts> constexpr bool AreLayoutQualifiersV = AreLayoutQualifiers<Ts...>::value;
//
//template<> struct AreLayoutQualifiers<> {
//	static const bool value = true;
//
//};
//template<typename T, typename... Ts> struct AreLayoutQualifiers<T, Ts...> {
//	static const bool value = AreLayoutQualifiersV<T> && ( SizeOf<Ts...> == 0 ? true : AreLayoutQualifiersV<Ts...>);
//};
//
//
//
//#define GLSL_LAYOUT_QUALIFIER(Q)													\
//																					\
//template<int N> struct Q : BaseLayoutQualifier {									\
//	static const int value = N;														\
//};																					\
//																					\
//template<typename... Ts> struct Get##Q;												\
//template<typename... Ts> constexpr int Get##Q##V = Get##Q<Ts...>::value;			\
//																					\
//template<typename T> struct Get##Q<T> {												\
//	static const int value = -1;													\
//};																					\
//																					\
//template<> struct Get##Q<> {														\
//	static const int value = -1;													\
//};																					\
//																					\
//template<int N> struct Get##Q<Q<N>> {												\
//	static const int value = N;														\
//};																					\
//																					\
//template<typename T, typename... Ts> struct Get##Q<T, Ts...> {						\
//	static const int value = LastIfValidV<Get##Q##V<T>, Get##Q##V<Ts...> >;			\
//};																					\
//																					\
//template<int N> struct AreLayoutQualifiers<Q<N>> {									\
//	static const bool value = true;													\
//};																					\
//
//GLSL_LAYOUT_QUALIFIER(Offset);
//GLSL_LAYOUT_QUALIFIER(Binding);
//GLSL_LAYOUT_QUALIFIER(Location);
//
//template<typename... LayoutQualifiers> struct Layout {
//	static_assert(AreLayoutQualifiersV<LayoutQualifiers...>, "cannot use Layout with non LayoutQualifier types");
//
//	//static const int binding = GetBindingV<LayoutQualifiers...>;
//	//static const int location = GetLocationV<LayoutQualifiers...>;
//	//static const int location = GetLocationV<LayoutQualifiers...>;
//};
//
//template<typename... LayoutQualifiers> struct LayoutQualifiersStr { 
//
//	static void addToLayoutStr(const std::string & qualifierStr, int value, std::string & s, bool & first) {
//		if (value >= 0) {
//			if (!first) {
//				s += ", ";
//			}
//			s += qualifierStr + " = " + std::to_string(value);
//			first = false;
//		}
//	}
//
//	static const std::string str() {
//		std::string s = "";
//		bool first = true;
//		addToLayoutStr("binding", GetBindingV<LayoutQualifiers...>, s, first);
//		addToLayoutStr("location", GetLocationV<LayoutQualifiers...>, s, first);
//		addToLayoutStr("offset", GetOffsetV<LayoutQualifiers...>, s, first);
//
//		return s;
//	}
//};
//
//
//template<typename T> struct LayoutStrT {
//	static const std::string str() { return ""; }
//};
//
//template<typename... LayoutQualifiers> struct LayoutStrT<Layout<LayoutQualifiers...> > {
//	static const std::string str() {
//		return SizeOf<LayoutQualifiers...> == 0 ? "" : ("layout(" + LayoutQualifiersStr<LayoutQualifiers...>::str() + ") ");
//	}
//};
//
//template<typename T> struct IsLayout {
//	static const bool value = false;
//};
//template<typename T> constexpr bool IsLayoutV = IsLayout<T>::value;
//
//template<typename... LayoutQualifiers> struct IsLayout<Layout<LayoutQualifiers...> > {
//	static const bool value = true;
//};
//
//template<typename Param, typename T, typename L = Layout<> > struct Qualifier : public T {
//	static_assert(IsLayoutV<L>, "L must be a Layout");
//	//using T::T;		
//
//	Qualifier(const std::string & s = "") : T(s) {
//		Ctx().removeCmd();
//		Ctx().addCmd(strFromDecl(*this) + ";");
//	}
//	Qualifier(const T & t) : T(strFromObj(t)) {
//		Ctx().removeCmd();
//		release(t);
//	}
//
//	static const std::string typeStr() { return TypeStr<Qualifier>::str(); }
//};
//
//struct InParam {};
//struct OutParam {};
//struct UniformParam {};
//template<> struct TypeStr<InParam> { static std::string str() { return "in"; } };
//template<> struct TypeStr<OutParam> { static std::string str() { return "out"; } };
//template<> struct TypeStr<UniformParam> { static std::string str() { return "uniform"; } };
//
//template<typename Param, typename T, typename L> struct TypeStr<Qualifier<Param, T, L>> {
//	static const std::string str() { return LayoutStrT<L>::str() + TypeStr<Param>::str() + " " + T::typeStr(); }
//};
//template<typename Param, typename L, numberType type, unsigned int N> struct AreValidVecCtorT<Qualifier<Param, Vec<type,N> , L>> {
//	static const bool value = true;
//};
//template<typename Param, typename L, numberType type, unsigned int N> struct getVecSizeT<Qualifier<Param, Vec<type, N>, L>> {
//	static const unsigned int value = N;
//};
//
//template<typename Param, typename T, typename L> struct Infos<Qualifier<Param,T,L>> {
//	static const bool numeric_type = Infos<T>::numeric_type;
//	static const unsigned int rows = Infos<T>::rows;
//	static const unsigned int cols = Infos<T>::cols;
//	static const numberType scalar_type = Infos<T>::scalar_type;
//};
//
//
//template<typename T, typename L = Layout<> > using In = Qualifier<InParam, T, L>;
//template<typename T, typename L = Layout<> > using Out = Qualifier<OutParam, T, L>;
//template<typename T, typename L = Layout<> > using Uniform = Qualifier<UniformParam, T, L>;
//
