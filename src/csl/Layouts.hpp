#pragma once

#include "Matrix.hpp"

namespace csl {

	////////////////////////////////////////////////////////////////
	// Merge sort helpers for LayoutQualifiers

	template<typename A, typename B> struct ConcatImpl;
	template<typename A, typename B> 
	using Concat = typename ConcatImpl<A, B>::Type;

	template<typename A, typename ...Bs> struct ConcatImpl<TList<A>, TList<Bs...>> {
		using Type = TList<A, Bs...>;
	};

	template<typename A, typename B, template<typename,typename> class Comp> struct MergeImpl;
	template<typename A, typename B, template<typename, typename> class Comp> 
	using Merge = typename MergeImpl<A, B, Comp>::Type;

	template<template<typename, typename> class Comp, typename ...As>
	struct MergeImpl<TList<As...>, TList<>, Comp> {
		using Type = TList<As...>;
	};
	template<template<typename, typename> class Comp, typename ...Bs>
	struct MergeImpl<TList<>, TList<Bs...>, Comp> {
		using Type = TList<Bs...>;
	};
	template<template<typename, typename> class Comp, typename A, typename B, typename ... As, typename ... Bs>
	struct MergeImpl<TList<A,As...>, TList<B,Bs...>, Comp> {
		using Type = std::conditional_t <
			(Comp<A, B>::value == 0),
			Merge<TList<As...>, TList<B, Bs...>, Comp>,
			std::conditional_t <
				(Comp<A, B>::value < 0),
				Concat<TList<A>, Merge<TList<As...>, TList<B, Bs...>, Comp>>,
				Concat<TList<B>, Merge<TList<A, As...>, TList<Bs...>, Comp>>
			>
		>;
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
		static constexpr int sort_value = static_cast<int>(lq) / 8;
	};

	template<LayoutQualifier lq, uint N>
	struct LayoutQArgValue {
		static constexpr int sort_value = static_cast<int>(lq) / 8;
	};

	template<typename A, typename B> struct LayoutQualifierSort {
		constexpr static int value = A::sort_value - B::sort_value;
	};

	using Shared = LayoutQArg<SHARED>;
	using Packed = LayoutQArg<PACKED>;
	using Std140 = LayoutQArg<STD140>;
	using Std430 = LayoutQArg<STD430>;
	using Row_major = LayoutQArg<ROW_MAJOR>;
	using Column_major = LayoutQArg<COLUMN_MAJOR>;
	using Early_fragment_tests = LayoutQArg<EARLY_FRAGMENT_TEST>;

	template<uint N> using Offset = LayoutQArgValue<OFFSET, N>;
	template<uint N> using Binding = LayoutQArgValue<BINDING, N>;
	template<uint N> using Location = LayoutQArgValue<LOCATION, N>;

	namespace geom_common {
		using Points = LayoutQArg<POINTS>;
		using Lines = LayoutQArg<LINES>;
		using Triangles = LayoutQArg<TRIANGLES>;

		using Line_strip = LayoutQArg<LINE_STRIP>;
		using Triangle_strip = LayoutQArg<TRIANGLE_STRIP>;

		template<uint N> using Max_vertices = LayoutQArgValue<MAX_VERTICES, N>;
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

		Qualifier(const std::string &s = "", uint flags = 0) : T(s, IS_BASE)
		{
			exp = createDeclaration<Qualifier>(NamedObjectBase::strPtr(), flags);
		}

		template<size_t N>
		Qualifier(const char(&s)[N], uint flags = 0) : T(s, IS_BASE) {
			exp = createDeclaration<Qualifier>(NamedObjectBase::strPtr(), flags);
		}

		Qualifier(const NamedObjectInit<T> & obj) : T(obj.name, IS_BASE)
		{
			exp = createInit<Qualifier>(NamedObjectBase::strPtr(), INITIALISATION, 0, obj.exp);
		}

		Qualifier(const Ex & _ex, uint ctor_flags = 0, uint obj_flags = IS_TRACKED, const std::string & s = "")
			: T(_ex, ctor_flags | IS_BASE, obj_flags, s)
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
