#pragma once

#include "NamedObjects.hpp"

namespace csl {

	namespace core {

		////////////////////////////////////////////////////////////////
		// Merge sort helpers for LayoutQualifiers

		template<typename A, typename B> struct ConcatImpl;
		template<typename A, typename B>
		using Concat = typename ConcatImpl<A, B>::Type;

		template<typename A, typename ...Bs> 
		struct ConcatImpl<TList<A>, TList<Bs...>> {
			using Type = TList<A, Bs...>;
		};

		template<typename A, typename B, template<typename, typename> class Comp> 
		struct MergeImpl;
		
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
		struct MergeImpl<TList<A, As...>, TList<B, Bs...>, Comp> {
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

		template<typename T, template<typename, typename> class Comp> 
		struct SortImpl;
		
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
			static constexpr int sort_value = static_cast<int>(lq) / maxQualifierValues;
		};

		template<LayoutQualifier lq, uint N>
		struct LayoutQArgValue {
			static constexpr int sort_value = static_cast<int>(lq) / maxQualifierValues;
		};

		template<typename A, typename B> struct LayoutQualifierSort {
			constexpr static int value = A::sort_value - B::sort_value;
		};

		template<QualifierType q, typename T, typename L>
		struct Qualifier : T
		{
			using T::exp;

			Qualifier(const std::string& s = "", ObjFlags flags = ObjFlags::NONE)
				: T(s, ObjFlags::IS_BASE)
			{
				exp = createDeclaration<Qualifier>(NamedObjectBase::strPtr(), flags);
			}

			Qualifier(const std::string& s, OpFlags flags)
				: T(s, ObjFlags::IS_BASE)
			{
				exp = createInit<Qualifier>(NamedObjectBase::strPtr(), DECLARATION, flags, ObjFlags::IS_BASE);
			}

			template<size_t N>
			Qualifier(const char(&s)[N], ObjFlags flags = ObjFlags::NONE)
				: T(s, ObjFlags::IS_BASE)
			{
				exp = createDeclaration<Qualifier>(NamedObjectBase::strPtr(), flags);
			}

			Qualifier(const NamedObjectInit<T>& obj) 
				: T(obj.name, ObjFlags::IS_BASE)
			{
				exp = createInit<Qualifier>(NamedObjectBase::strPtr(), INITIALISATION, OpFlags::NONE, ObjFlags::IS_BASE, obj.exp);
			}

			Qualifier(const Ex& _ex, OpFlags ctor_flags = OpFlags::NONE, ObjFlags obj_flags = ObjFlags::IS_TRACKED, const std::string& s = "")
				: T(_ex, ctor_flags, obj_flags | ObjFlags::IS_BASE, s)
			{
			}
		};

		template<QualifierType q, ScalarType type, uint NR, uint NC, typename L>
		struct Qualifier<q, Matrix<type, NR, NC>, L> 
			: Matrix<type, NR, NC> 
		{
			using Base = Matrix<type, NR, NC>;
			using Base::operator[];
			using Base::operator=; 
			using Base::exp;
			
			Qualifier(const std::string& s = "", ObjFlags flags = ObjFlags::NONE)
				: Base(s, ObjFlags::IS_BASE)
			{
				exp = createDeclaration<Qualifier>(NamedObjectBase::strPtr(), flags);
			}

			Qualifier(const std::string& s, OpFlags flags)
				: Base(s, ObjFlags::IS_BASE)
			{
				exp = createInit<Qualifier>(NamedObjectBase::strPtr(), DECLARATION, flags, ObjFlags::IS_BASE);
			}

			template<size_t N>
			Qualifier(const char(&s)[N], ObjFlags flags = ObjFlags::NONE)
				: Base(s, ObjFlags::IS_BASE) {
				exp = createDeclaration<Qualifier>(NamedObjectBase::strPtr(), flags);
			}

			Qualifier(const NamedObjectInit<Base>& obj)
				: Base(obj.name, ObjFlags::IS_BASE)
			{
				exp = createInit<Qualifier>(NamedObjectBase::strPtr(), INITIALISATION, OpFlags::NONE, ObjFlags::IS_BASE, obj.exp);
			}

			Qualifier(const Ex& _ex, OpFlags ctor_flags = OpFlags::NONE, ObjFlags obj_flags = ObjFlags::IS_TRACKED, const std::string& s = "")
				: Base(_ex, ctor_flags, obj_flags | ObjFlags::IS_BASE, s)
			{
			}
		};

		template<QualifierType q, typename T, typename L>
		struct Qualifier<q, Array<T>, L> 
			: Array<T>
		{
			using Base = Array<T>;
			using Base::Base;
			using Base::operator[];
		};


		template<QualifierType _qType, typename ... T>
		struct QualiPH;

		//placeholder
		template<QualifierType _qType>
		struct QualiPH<_qType> {
			static const QualifierType qType = _qType;
			using LayoutType = LayoutImpl<>;
			using Type = QualiPH;
		};

		//placeholder
		template<QualifierType _qType, typename ... Q>
		struct QualiPH<_qType, LayoutImpl<Q...>> {
			static const QualifierType qType = _qType;
			using LayoutType = LayoutImpl<Q...>;
			using Type = QualiPH;
		};

		//actual types
		template<QualifierType qType, typename T>
		struct QualiPH<qType, T> {
			using Type = Qualifier<qType, T, LayoutImpl<>>;
		};

		template<QualifierType qType, typename T, typename L>
		struct QualiPH<qType, T, L> {
			using Type = Qualifier<qType, T, L>;
		};

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

		namespace geom_common {
			using Points = LayoutQArg<POINTS>;
			using Lines = LayoutQArg<LINES>;
			using Triangles = LayoutQArg<TRIANGLES>;

			using Line_strip = LayoutQArg<LINE_STRIP>;
			using Triangle_strip = LayoutQArg<TRIANGLE_STRIP>;

			template<uint N>
			using Max_vertices = LayoutQArgValue<MAX_VERTICES, N>;
		}

		namespace tesc_common {

			template<uint N>
			using Vertices = LayoutQArgValue<VERTICES, N>;
		}

		namespace tese_common {

			using Triangles = LayoutQArg<TRIANGLES>;
			using Quads = LayoutQArg<QUADS>;
			using Isolines = LayoutQArg<ISOLINES>;

			using Equal_spacing = LayoutQArg<EQUAL_SPACING>;
			using Fractional_even_spacing = LayoutQArg<FRACTIONAL_EVEN_SPACING>;
			using Fractional_odd_spacing = LayoutQArg<FRACTIONAL_ODD_SPACING>;

			using Cw = LayoutQArg<CW>;
			using Ccw = LayoutQArg<CCW>;

		}

		template<typename ... LayoutArgs>
		struct LayoutImpl {
			using CleanupArgs = Sort<TList<LayoutArgs...>, LayoutQualifierSort>;
		};
	}

	template<typename ... LayoutArgs>
	using Layout = core::LayoutImpl<LayoutArgs ...>;

	using Shared = core::LayoutQArg<core::SHARED>;
	using Packed = core::LayoutQArg<core::PACKED>;
	using Std140 = core::LayoutQArg<core::STD140>;
	using Std430 = core::LayoutQArg<core::STD430>;
	using Row_major = core::LayoutQArg<core::ROW_MAJOR>;
	using Column_major = core::LayoutQArg<core::COLUMN_MAJOR>;
	using Early_fragment_tests = core::LayoutQArg<core::EARLY_FRAGMENT_TEST>;

	template<core::uint N> using Offset = core::LayoutQArgValue<core::OFFSET, N>;
	template<core::uint N> using Binding = core::LayoutQArgValue<core::BINDING, N>;
	template<core::uint N> using Location = core::LayoutQArgValue<core::LOCATION, N>;

	template<typename ...T>
	using Uniform = typename core::QualiPH<core::UNIFORM, T...>::Type;

	template<typename ...T>
	using Out = typename core::QualiPH<core::OUT, T...>::Type;

	template<typename ...T>
	using In = typename core::QualiPH<core::IN, T...>::Type;

	template<typename ...T>
	using Inout = typename core::QualiPH<core::INOUT, T...>::Type;

} //namespace csl
