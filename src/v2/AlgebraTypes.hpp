#pragma once


#include "NamedObjects.hpp"
#include "Swizzles.hpp"
#include "Types.hpp"

#include <typeinfo>
#include <vector>
#include <array>

#define EXPR(type, var) get_expr(std::forward<type>(var))

namespace v2 {

	template< typename T, std::size_t R, std::size_t C, typename Qs>
	struct MatrixIndirection;

	template< typename T, typename Ds, std::size_t R, std::size_t C, typename Qs>
	struct MatrixArrayIndirection;

	template< typename T, std::size_t R, std::size_t C, typename ... Qs>
	using MatrixInterface = std::conditional_t<
		ArrayInfos<Qs...>::Value,
		typename MatrixArrayIndirection<T, typename ArrayInfos<Qs...>::Dimensions, R, C, RemoveArrayFromQualifiers<Qs...> >::Type,
		typename MatrixIndirection<T, R, C, RemoveArrayFromQualifiers<Qs...> >::Type
	>;

	template<typename T, std::size_t R, std::size_t C, typename ...Qs>
	class Matrix : virtual public NamedObject<Matrix<T, R, C, Qs...>> {
	public:

		virtual ~Matrix() = default;

		using Qualifiers = TList<Qs...>;

		using This = Matrix;
		using QualifierFree = Matrix<T, R, C>;
		using Row = Matrix<T, 1, C, Qs...>;
		using Col = Matrix<T, R, 1, Qs...>;
		using Scalar = Matrix<T, 1, 1, Qs...>;

		template<std::size_t RowCount>
		using SubCol = Vector<T, RowCount, Qs...>;

		static constexpr std::size_t RowCount = R;
		static constexpr std::size_t ColCount = C;
		static constexpr bool IsScalar = (R == 1 && C == 1);
		static constexpr bool IsBool = std::is_same_v<T, bool>;

		static constexpr bool IsArray = false;
		using ArrayDimensions = SizeList<>;

		using Base = NamedObject<Matrix<T, R, C, Qs...>>;

	public:
		Matrix() : Base() {}

		template<std::size_t N>
		explicit Matrix(const char(&name)[N]) : Base(name) {}

		//Matrix(const Expr& expr, const ObjFlags obj_flags = ObjFlags::Default) : Base(expr, obj_flags) { }
		//Matrix(Expr && expr, const ObjFlags obj_flags = ObjFlags::Default) : Base(expr, obj_flags) { }
		//Matrix(Expr expr) : Base(expr, ObjFlags::Default) { }
		Matrix(const Expr& expr, const ObjFlags obj_flags = ObjFlags::Default) : NamedObjectBase("", obj_flags), Base(expr, obj_flags) { }

		Matrix(Matrix&& other) : Base(other) {}

		template<typename M, typename = std::enable_if_t<SameSize<Matrix, M> && SameScalarType<Matrix, M>>>
		Matrix operator=(M&& other)& {
			return { make_expr<BinaryOperator>(Op::Assignment, NamedObjectBase::get_expr_as_ref(), EXPR(M,other)) };
		}

		template<typename M, typename = std::enable_if_t<SameSize<Matrix, M> && SameScalarType<Matrix, M>>>
		Matrix operator=(M&& other)&& {
			return { make_expr<BinaryOperator>(Op::Assignment, NamedObjectBase::get_expr_as_temp(), EXPR(M,other)) };
		}

		template<typename U, typename T, typename ...Ts,
			typename = std::enable_if_t< (NumElements<U, T, Ts...> == R * C) && SameScalarType<Matrix, U, T, Ts...> > >
			explicit Matrix(U&& u, T&& t, Ts&&...ts) : Base("", ObjFlags::Default, CtorFlags::Initialisation, EXPR(U, u), EXPR(T, t), EXPR(Ts, ts)...) 
		{
		}

		// swizzling
		template<typename Swizzle, typename = std::enable_if_t<C == 1 && R != 1 && (Swizzle::Size <= R)> >
		std::conditional_t<Swizzle::Unique, SubCol<Swizzle::Size>, const SubCol<Swizzle::Size>> operator[](Swizzle swizzle) const& {
			return { make_expr<SwizzlingWrapper>(SwizzlingWrapper::create<Swizzle>(NamedObjectBase::get_expr_as_ref())) };
		}

		// col access
		template<typename Index, typename = std::enable_if_t<!IsScalar && Infos<Index>::IsInteger > >
		std::conditional_t<C == 1, Scalar, Col> operator[](Index&& index) const&
		{
			return { make_expr<ArraySubscript>(NamedObjectBase::get_expr_as_ref(), EXPR(Index, index)) };
		}

		template<bool b = !IsBool, typename = std::enable_if_t<b>>
		QualifierFree operator-() const& {
			return { make_expr<UnaryOperator>(Op::UnaryNegation, NamedObjectBase::get_expr_as_ref()) };
		}

		template<bool b = !IsBool, typename = std::enable_if_t<b>>
		QualifierFree operator-() const&& {
			return { make_expr<UnaryOperator>(Op::UnaryNegation, NamedObjectBase::get_expr_as_temp()) };
		}
	};

	// operator*

	template<typename A, typename B, typename = std::enable_if_t<
		!Infos<A>::IsBool && SameScalarType<A, B> && (Infos<B>::IsScalar || SameSize<A, B> || Infos<A>::ColCount == Infos<B>::RowCount)>>
		typename AlgebraMulInfos<A, B>::ReturnType operator*(A&& a, B&& b)
	{
		return { make_expr<BinaryOperator>(AlgebraMulInfos<A, B>::Operator, EXPR(A,a), EXPR(B,b)) };
	}

	template<typename A, typename B, typename = std::enable_if_t<
		!Infos<A>::IsBool && SameScalarType<A, B> && !Infos<B>::IsScalar && (Infos<A>::IsScalar || (!Infos<A>::IsSquare && Infos<A>::RowCount == Infos<B>::ColCount)) >>
		auto operator*(A&& a, B&& b)
	{
		return std::forward<B>(b) * std::forward<A>(a);
	}

	// operator +

	template<typename A, typename B, typename = std::enable_if_t<
		!Infos<A>::IsBool && SameScalarType<A, B> && (Infos<B>::IsScalar || SameSize<A, B>)>>
		typename AlgebraAddInfos<A, B>::ReturnType operator+(A&& a, B&& b)
	{
		return { make_expr<BinaryOperator>(AlgebraAddInfos<A, B>::OperatorAdd, EXPR(A,a), EXPR(B,b)) };
	}

	template<typename A, typename B, typename = std::enable_if_t<
		!Infos<A>::IsBool && SameScalarType<A, B> && !Infos<B>::IsScalar && Infos<A>::IsScalar >>
		auto operator+(A&& a, B&& b)
	{
		return std::forward<B>(b) + std::forward<A>(a);
	}

	// operator -

	template<typename A, typename B, typename = std::enable_if_t<
		!Infos<A>::IsBool && SameScalarType<A, B> && (Infos<B>::IsScalar || SameSize<A, B>)>>
		typename AlgebraAddInfos<A, B>::ReturnType operator-(A&& a, B&& b)
	{
		return { make_expr<BinaryOperator>(AlgebraAddInfos<A, B>::OperatorSub, EXPR(A,a), EXPR(B,b)) };
	}

	template<typename A, typename B, typename = std::enable_if_t<
		!Infos<A>::IsBool && SameScalarType<A, B> && !Infos<B>::IsScalar && Infos<A>::IsScalar >>
		auto operator-(A&& a, B&& b)
	{
		return std::forward<B>(b) - std::forward<A>(a);
	}

	template< typename T, typename Ds, std::size_t R, std::size_t C, typename ... Qs>
	class MatrixArray : public NamedObject<MatrixArray<T, Ds, R, C, Qs...>> {
	public:

		using Qualifiers = TList<Qs...>;
		using Base = NamedObject<MatrixArray<T, Ds, R, C, Qs...>>;

		using ArrayComponent = MatrixInterface<T, R, C, typename GetArrayFromList<typename Ds::Tail>::Type, Qs...>;
		static constexpr bool IsArray = true;

		using ArrayDimensions = Ds;
		static constexpr std::size_t ComponentCount = Ds::Front;

		MatrixArray() : Base() {}

		template<std::size_t N>
		explicit MatrixArray(const char(&name)[N]) : Base(name) {}

		MatrixArray(const Expr& expr) : Base(expr) { }

		template<typename U, typename V, typename ... Us, typename = std::enable_if_t<
			!(std::is_same_v<Expr, Us> || ...) && (SameType<Us, ArrayComponent> && ...) && (ComponentCount == 0 || 2 + sizeof...(Us) == ComponentCount)
			>>
			explicit MatrixArray(U&& u, V&& v, Us&& ... us) : Base("", ObjFlags::Default, CtorFlags::Initialisation, EXPR(Us, us)...)
		{
		}

		template<typename Index>
		ArrayComponent operator [](Index&& index) const& {
			return { make_expr<ArraySubscript>(NamedObjectBase::get_expr_as_ref(), EXPR(Index, index)) };
		}

		template<typename Index>
		ArrayComponent operator [](Index&& index) const&& {
			return { make_expr<ArraySubscript>(NamedObjectBase::get_expr_as_temp(), EXPR(Index, index)) };
		}

	};

	template< typename T, std::size_t R, std::size_t C, typename ...Qs>
	struct MatrixIndirection<T, R, C, TList<Qs...>> {
		using Type = Matrix<T, R, C, Qs...>;
	};

	template< typename T, typename Ds, std::size_t R, std::size_t C, typename ...Qs>
	struct MatrixArrayIndirection<T, Ds, R, C, TList<Qs...>> {
		using Type = MatrixArray<T, Ds, R, C, Qs...>;
	};

	using vec3 = Vector<float, 3>;

	using mat4 = Matrix<float, 4, 4>;
	using mat3 = Matrix<float, 3, 3>;

	using Float = Scalar<float>;

	using Int = Scalar<int>;

	using Bool = Scalar<bool>;

	template<typename T, std::size_t R, std::size_t C, typename ... Qs>
	struct QualifiedIndirection<Matrix<T, R, C>, Qs... > {
		using Type = MatrixInterface<T, R, C, Qs...>;
	};

	//template<typename T, std::size_t R, typename ... Qs>
	//using VectorInterface = MatrixInterface<T, R, 1, Qs...>;

	//template<typename T, typename ... Qs>
	//using ScalarInterface = VectorInterface<T, 1, Qs...>;

	//template<typename ... Qs>
	//class mat3 : public MatrixInterface<float, 3, 3, Qs...> {
	//public:
	//	using Base = MatrixInterface<float, 3, 3, Qs...>;
	//	using Base::Base;
	//};

	//mat3()->mat3<>;

	//template<typename U, typename V, typename ...Vs>
	//mat3(U&&, V&&, Vs&&...)->mat3<>;

	//template<typename ... Qs> struct Infos<mat3<Qs...>> : Infos<MatrixInterface<float, 3, 3, Qs...>> {};

	//template<typename ... Qs>
	//class vec3 : public VectorInterface<float, 3, Qs...> {
	//public:
	//	using Base = VectorInterface<float, 3, Qs...>;
	//	using Base::Base;
	//	using Base::operator=;
	//};
	//vec3()->vec3<>;

	//template<typename U, typename V, typename ...Vs>
	//vec3(U&&, V&&, Vs&&...)->vec3<>;

	//template<typename ... Qs> struct Infos<vec3<Qs...>> : Infos<MatrixInterface<float, 3, 1, Qs...>> {};

	//template<std::size_t N>
	//vec3(const char(&)[N])->vec3<>;


	//template<typename ... Qs>
	//class Float : public ScalarInterface<float, Qs...> {
	//public:
	//	using Base = ScalarInterface<float, Qs...>;
	//	using Base::Base;
	//};

	//Float()->Float<>;

	//template<std::size_t N>
	//Float(const char(&)[N])->Float<>;

	//template<typename ... Qs> struct Infos<Float<Qs...>> : Infos<MatrixInterface<float, 1, 1, Qs...>> {};

	//template<typename ... Qs>
	//class Int : public ScalarInterface<int, Qs...> {
	//public:
	//	using Base = ScalarInterface<int, Qs...>;
	//	using Base::Base;
	//};

	//Int()->Int<>;

	//template<std::size_t N>
	//Int(const char(&)[N])->Int<>;

	//template<typename ... Qs> struct Infos<Int<Qs...>> : Infos<MatrixInterface<int, 1, 1, Qs...>> {};

} //namespace csl

#undef EXPR