#pragma once

#include "MatrixTypesTest.h"
#include <tuple>

namespace csl {

	////////////////////////////////////////////////////////////////
	// Merge sort helpers for LayoutQualifiers

	template<typename A, typename B> struct Concat;
	template<typename A, typename ...Bs> struct Concat<TList<A>, TList<Bs...>> {
		using Type = TList<A, Bs...>;
	};

	template<typename A, typename B, template<typename,typename> class Comp> struct MergeImpl;
	template<typename A, typename B, template<typename, typename> class Comp> 
	using Merge = typename MergeImpl<A, B, Comp>::Type;

	template<template<typename, typename> class Comp, typename ...As>
	struct MergeImpl<TList<As...>, TList<>, Comp> {
		using Type = TList<As...>;
	};
	template<typename ...Bs, template<typename, typename> class Comp>
	struct MergeImpl<TList<>, TList<Bs...>, Comp> {
		using Type = TList<Bs...>;
	};
	template<typename A, typename B, template<typename, typename> class Comp, typename ... As, typename ... Bs>
	struct MergeImpl<TList<A, As...>, TList<B, Bs...>, Comp> {
		using Type = std::conditional_t <
			(Comp<A, B>::value == 0),
			Merge<TList<As...>, TList<B, Bs...>, Comp>,
			std::conditional_t <
				(Comp<A, B>::value < 0),
				typename Concat<TList<A>, Merge<TList<As...>, TList<B, Bs...>, Comp>>::Type,
				typename Concat<TList<B>, Merge<TList<A, As...>, TList<Bs...>, Comp>>::Type
			>
		>;
	};

	template<typename A, size_t first, typename Range>
	struct SubsetImpl;

	template<typename A, size_t first, size_t last>
	using Subset = typename SubsetImpl<A, first, std::make_index_sequence<last - first>>::Type;

	template<size_t first, size_t ... Is, typename ... Ts>
	struct SubsetImpl< TList<Ts...>, first, std::index_sequence<Is...> > {
		using Type = TList<std::tuple_element_t<first + Is, std::tuple<Ts...>> ...>;
	};

	template<typename T, template<typename, typename> class Comp> struct SortImpl;
	template<typename T, template<typename, typename> class Comp>
	using Sort = typename SortImpl<T, Comp>::Type;

	template<template<typename, typename> class Comp>
	struct SortImpl<TList<>, Comp> {
		using Type = TList<>;
	};
	template<typename T, template<typename, typename> class Comp>
	struct SortImpl<TList<T>, Comp> {
		using Type = TList<T>;
	};

	template<template<typename, typename> class Comp, typename T, typename U, typename ... Ts>
	struct SortImpl<TList<T, U, Ts...>, Comp> {
		constexpr static size_t N = 2 + sizeof...(Ts);
		using M = TList<T, U, Ts...>;
		using Type = Merge<
			Sort<Subset<M, 0, N / 2>, Comp>,
			Sort<Subset<M, N / 2, N>, Comp>,
			Comp
		>;
	};

	//template<typename A, typename B> struct SizeOfComparator {
	//	constexpr static int value = sizeof(A) - sizeof(B);
	//};

	//template<typename A, typename B> struct SizeOfComparatorReverse {
	//	constexpr static int value = -SizeOfComparator<A, B>::value;
	//};

	//using LL = TList<double, char, double, double, double, float, short, double>;

	//using S = Sort<LL, SizeOfComparator>;
	//using RS = Sort<LL, SizeOfComparatorReverse>;
	//
	//static constexpr bool bbbb = std::is_same_v<S, TList<char, short, float, double>>;
	//static constexpr bool bbb = std::is_same_v<RS, TList<double, float, short, char>>;

	template<LayoutQualifier lq> 
	struct LayoutQArg {
		static constexpr size_t sort_value = static_cast<size_t>(lq) / 8;
	};

	template<LayoutQualifier lq, uint N>
	struct LayoutQArgValue {
		static constexpr size_t sort_value = static_cast<size_t>(lq) / 8;
	};

	template<typename A, typename B> struct LayoutQualifierSort {
		constexpr static int value = (A::sort_value - B::sort_value);
	};

	using Shared = LayoutQArg<SHARED>;
	using Packed = LayoutQArg<PACKED>;
	using Std140 = LayoutQArg<STD140>;
	using Std430 = LayoutQArg<STD430>;

	template<int N> using Offset = LayoutQArgValue<OFFSET, N>;
	template<int N> using Binding = LayoutQArgValue<BINDING, N>;
	template<int N> using Location = LayoutQArgValue<LOCATION, N>;

	namespace geom_common {
		using Points = LayoutQArg<POINTS>;
		using Lines = LayoutQArg<LINES>;
		using Triangles = LayoutQArg<TRIANGLES>;

		using Line_strip = LayoutQArg<LINE_STRIP>;
		using Triangle_strip = LayoutQArg<TRIANGLE_STRIP>;

		template<int N> using Max_vertices = LayoutQArgValue<MAX_VERTICES, N>;
	}

	
	template<typename ... LayoutArgs>
	struct Layout {
		using CleanupArgs = Sort<TList<LayoutArgs...>, LayoutQualifierSort>;
	};

	template<QualifierType q, typename T, typename L>
	struct Qualifier : T
	{
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

		Qualifier(const Ex & _ex, uint ctor_flags = 0, uint obj_flags = IS_TRACKED, const std::string & s = "")
			: T(_ex, ctor_flags, obj_flags, s)
		{
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

} //namespace csl