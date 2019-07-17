#pragma once

#include "MatrixTypesTest.h"

template<LayoutArgBoolType layoutArg, bool b>
struct LayoutArgBool { };

using Shared = LayoutArgBool<SHARED, true>;
using Packed = LayoutArgBool<PACKED, true>;
using std140 = LayoutArgBool<STD140, true>;
using std430 = LayoutArgBool<STD430, true>;

template<LayoutArgIntType layoutArg, int N>
struct LayoutArgInt { };

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
	
	static constexpr int offset = GetLayoutArg<OFFSET, LayoutArgs...>::value;
	static constexpr int binding = GetLayoutArg<BINDING, LayoutArgs...>::value;
	static constexpr int location = GetLayoutArg<LOCATION, LayoutArgs...>::value;

	static constexpr bool is_std140 = ContainsType<std140, LayoutArgs...>;
	static constexpr bool is_std430 = ContainsType<std430, LayoutArgs...>;
	static constexpr bool is_packed = ContainsType<Packed, LayoutArgs...>; 
	static constexpr bool is_shared = ContainsType<Shared, LayoutArgs...>;

	using CleanedArgs = LayoutCleanedArg<
		Offset<offset>,
		Binding<binding>,
		Location<location>,
		LayoutArgBool<STD140, is_std140>,
		LayoutArgBool<STD430, is_std430>,
		LayoutArgBool<PACKED, is_packed>,
		LayoutArgBool<SHARED, is_shared>
	>;

	static constexpr bool empty = (offset < 0) && (binding < 0) && (location < 0) && !( is_std140 || is_std430 || is_shared || is_packed);
};

template<QualifierType q, typename T, typename L> 
struct Qualifier : T
{
	using UnderlyingType = T;
	using T::exp;
	using T::operator=;

	Qualifier(const std::string &s = "", uint flags = 0) : T(s, 0)
	{
		exp = createDeclaration<Qualifier>(NamedObjectBase::strPtr(), flags);
	}

	Qualifier(const NamedObjectInit<T> & obj) : T(obj.name, 0)
	{
		exp = createInit<Qualifier>(NamedObjectBase::strPtr(), INITIALISATION, 0, obj.exp);
	}
};


template<QualifierType _qType, typename ... T> 
struct QualiPH;

//placeholder
template<QualifierType _qType>
struct QualiPH<_qType> {
	static const QualifierType qType = _qType;
	using LayoutType = Layout<>;
	using Type = QualiPH;
};

//placeholder
template<QualifierType _qType, typename ... Q>
struct QualiPH<_qType, Layout<Q...>> {
	static const QualifierType qType = _qType;
	using LayoutType = Layout<Q...>;
	using Type = QualiPH;
};

//actual types
template<QualifierType qType, typename T>
struct QualiPH<qType, T> {
	using Type = Qualifier<qType, T, Layout<>>;
};

template<QualifierType qType, typename T, typename L>
struct QualiPH<qType, T, L> {
	using Type = Qualifier<qType, T, L>;
};

template<typename ...T>
using Uniform = typename QualiPH<UNIFORM, T...>::Type;

template<typename ...T>
using Out = typename QualiPH<OUT, T...>::Type;

template<typename ...T>
using In = typename QualiPH<IN, T...>::Type;

template<typename ... T>
struct GetQualifier;

template<typename T>
struct GetQualifier<T> {
	using Type = T;
};

template<typename QualifierPH, typename T>
struct GetQualifier<QualifierPH, T> {
	using Type = Qualifier<QualifierPH::qType, T, typename QualifierPH::LayoutType>;
};
