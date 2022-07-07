#pragma once


#include "NamedObjects.hpp"
#include "Swizzles.hpp"
#include "Types.hpp"

#include <typeinfo>
#include <vector>
#include <array>

#define EXPR(type, var) get_expr(std::forward<type>(var))

namespace csl
{
	template<typename T, std::size_t R, std::size_t C>
	class Matrix : public NamedObject<Matrix<T, R, C>>
	{
	public:
		using This = Matrix;
		using Base = NamedObject<Matrix<T, R, C>>;

		using Row = Matrix<T, 1, C>;
		using Col = Matrix<T, R, 1>;
		using Scalar = Matrix<T, 1, 1>;

		template<std::size_t RowCount>
		using SubCol = Vector<T, RowCount>;

		static constexpr std::size_t RowCount = R;
		static constexpr std::size_t ColCount = C;
		static constexpr bool IsScalar = (R == 1 && C == 1);
		static constexpr bool IsBool = std::is_same_v<T, bool>;
		static constexpr bool IsVec = (C == 1);
	public:

		//TODO move me
		operator bool() {
			static_assert(IsBool, "possible wrong conversion");
			return false;
		}

		Matrix(csl::Dummy) : Base() {}

		Matrix() : Base("", ObjFlags::Default, {}, {}) {}

		template<std::size_t N>
		Matrix(const char(&name)[N], const ObjFlags obj_flags = ObjFlags::Default) : Base(name, obj_flags, {}, {}) {}

		//Matrix(const Expr& expr, const ObjFlags obj_flags = ObjFlags::Default) : Base(expr, obj_flags) { }
		//Matrix(Expr && expr, const ObjFlags obj_flags = ObjFlags::Default) : Base(expr, obj_flags) { }
		//Matrix(Expr expr) : Base(expr, ObjFlags::Default) { }
		Matrix(const Expr& expr, const ObjFlags obj_flags = ObjFlags::Default)
			: /*NamedObjectBase(obj_flags),*/ Base(expr, obj_flags) { }

		//TODO add is_convertible_to
		template<typename U, typename V, typename ...Vs,
			typename = std::enable_if_t< (NumElements<U, V, Vs...> == R * C) && SameScalarType<U, V, Vs...> > >
			explicit Matrix(U&& u, V&& v, Vs&&...vs)
			: Base("", ObjFlags::Default, CtorFlags::Initialisation, SizeList<>{}, TList<>{}, EXPR(U, u), EXPR(V, v), EXPR(Vs, vs)...)
		{
		}

		Matrix(Matrix&& other) : Base(other.get_expr_as_temp())
		{
		}

		template<typename U, typename = std::enable_if_t<SameSize<Matrix, U>&& SameScalarType<Matrix, U>>>
		Matrix(const NamedObjectInit<U>& init) : Base(init) {}


		//TODO add IsConvertibleTo<Infos<M>::ScalarType, This>
		template<typename M, typename = std::enable_if_t< (SameSize<This, M> || Infos<M>::IsScalar)>>
		Matrix(M&& other) : Matrix(make_expr<ConvertorOperator< M, This>>(EXPR(M, other))) { }

		template<typename M, typename = std::enable_if_t<SameSize<Matrix, M>&& SameScalarType<Matrix, M>>>
		Matrix operator=(M&& other)& {
			return { make_expr<BinaryOperator>(Op::Assignment, NamedObjectBase::get_expr_as_ref(), EXPR(M,other)) };
		}

		template<typename M, typename = std::enable_if_t<SameSize<Matrix, M>&& SameScalarType<Matrix, M>>>
		Matrix operator=(M&& other)&& {
			return { make_expr<BinaryOperator>(Op::Assignment, NamedObjectBase::get_expr_as_temp(), EXPR(M,other)) };
		}

		// swizzling
		// TODO
		template<char ...cs, typename = std::enable_if_t<C == 1 && R != 1 && (swizzling::highest<cs...> <= R)> >
		std::conditional_t<swizzling::unique<cs...>, SubCol<sizeof...(cs)>, const SubCol<sizeof...(cs)>> operator()(Swizzle<cs> ... swizzles)& {
			return { make_expr<Swizzling<Swizzle>>(NamedObjectBase::get_expr_as_ref()) };
		}

		template<typename Swizzle, typename = std::enable_if_t<C == 1 && R != 1 && (Swizzle::HighestComponent <= R)> >
		std::conditional_t<Swizzle::NoDuplicates, SubCol<Swizzle::Size>, const SubCol<Swizzle::Size>> operator[](Swizzle swizzle)& {
			return { make_expr<Swizzling<Swizzle>>(NamedObjectBase::get_expr_as_ref()) };
		}

		template<typename Swizzle, typename = std::enable_if_t<C == 1 && R != 1 && (Swizzle::HighestComponent <= R)> >
		std::conditional_t<Swizzle::NoDuplicates, SubCol<Swizzle::Size>, const SubCol<Swizzle::Size>> operator[](Swizzle swizzle)&& {
			return { make_expr<Swizzling<Swizzle>>(NamedObjectBase::get_expr_as_temp()) };
		}

		template<typename Swizzle, typename = std::enable_if_t<C == 1 && R != 1 && (Swizzle::HighestComponent <= R)> >
		std::conditional_t<Swizzle::NoDuplicates, SubCol<Swizzle::Size>, const SubCol<Swizzle::Size>> operator[](Swizzle swizzle) const& {
			return { make_expr<Swizzling<Swizzle>>(NamedObjectBase::get_expr_as_ref()) };
		}

		template<typename Swizzle, typename = std::enable_if_t<C == 1 && R != 1 && (Swizzle::HighestComponent <= R)> >
		std::conditional_t<Swizzle::NoDuplicates, SubCol<Swizzle::Size>, const SubCol<Swizzle::Size>> operator[](Swizzle swizzle) const&& {
			return { make_expr<Swizzling<Swizzle>>(NamedObjectBase::get_expr_as_temp()) };
		}

		// col access
		template<typename Index, typename = std::enable_if_t<!IsScalar && Infos<Index>::IsInteger > >
		std::conditional_t<C == 1, Scalar, Col> operator[](Index&& index) const&
		{
			return { make_expr<ArraySubscript>(NamedObjectBase::get_expr_as_ref(), EXPR(Index, index)) };
		}

		// unaries
		template<bool b = !IsBool, typename = std::enable_if_t<b>>
		Matrix operator-() const& {
			return { make_expr<UnaryOperator>(Op::UnarySub, NamedObjectBase::get_expr_as_ref()) };
		}

		template<bool b = !IsBool, typename = std::enable_if_t<b>>
		Matrix operator-() const&& {
			return { make_expr<UnaryOperator>(Op::UnarySub, NamedObjectBase::get_expr_as_temp()) };
		}

		// X=
		template<typename A, typename = std::enable_if_t<!IsBool && SameScalarType<This, A> && (SameSize<This, A> || Infos<A>::IsScalar)>>
		void operator+=(A&& a)& {
			(void)Matrix(make_expr<BinaryOperator>(Op::AddAssignment, NamedObjectBase::get_expr_as_ref(), EXPR(A, a)));
		}

		template<typename A, typename = std::enable_if_t<!IsBool && SameScalarType<This, A> && (SameSize<This, A> || Infos<A>::IsScalar)>>
		void operator+=(A&& a)&& {
			(void)Matrix(make_expr<BinaryOperator>(Op::AddAssignment, NamedObjectBase::get_expr_as_temp(), EXPR(A, a)));
		}

		template<typename A, typename = std::enable_if_t<!IsBool && SameScalarType<This, A> && (SameSize<This, A> || Infos<A>::IsScalar)>>
		void operator-=(A&& a)& {
			(void)Matrix(make_expr<BinaryOperator>(Op::SubAssignment, NamedObjectBase::get_expr_as_ref(), EXPR(A, a)));
		}

		template<typename A, typename = std::enable_if_t<!IsBool && SameScalarType<This, A> && (SameSize<This, A> || Infos<A>::IsScalar)>>
		void operator-=(A&& a)&& {
			(void)Matrix(make_expr<BinaryOperator>(Op::SubAssignment, NamedObjectBase::get_expr_as_temp(), EXPR(A, a)));
		}

		template<typename A, typename = std::enable_if_t<!IsBool && SameScalarType<This, A> && (SameSize<This, A> || Infos<A>::IsScalar)>>
		void operator*=(A&& a)& {
			(void)Matrix(make_expr<BinaryOperator>(Op::MulAssignment, NamedObjectBase::get_expr_as_ref(), EXPR(A, a)));
		}

		template<typename A, typename = std::enable_if_t<!IsBool && SameScalarType<This, A> && (SameSize<This, A> || Infos<A>::IsScalar)>>
		void operator*=(A&& a)&& {
			(void)Matrix(make_expr<BinaryOperator>(Op::MulAssignment, NamedObjectBase::get_expr_as_temp(), EXPR(A, a)));
		}

		template<typename A, typename = std::enable_if_t<!IsBool && SameScalarType<This, A> && (SameSize<This, A> || Infos<A>::IsScalar)>>
		void operator/=(A&& a)& {
			(void)Matrix(make_expr<BinaryOperator>(Op::DivAssignment, NamedObjectBase::get_expr_as_ref(), EXPR(A, a)));
		}

		template<typename A, typename = std::enable_if_t<!IsBool && SameScalarType<This, A> && (SameSize<This, A> || Infos<A>::IsScalar)>>
		void operator/=(A&& a)&& {
			(void)Matrix(make_expr<BinaryOperator>(Op::DivAssignment, NamedObjectBase::get_expr_as_temp(), EXPR(A, a)));
		}

		template<typename A, typename = std::enable_if_t< IsVec&& IsInteger<Matrix>&& SameScalarType<Matrix, A> && (Infos<A>::IsScalar || SameSize<Matrix, A>) >  >
		void operator&=(A&& a)& {
			(void)Matrix(make_expr<BinaryOperator>(Op::BitwiseAndAssignment, NamedObjectBase::get_expr_as_ref(), EXPR(A, a)));
		}

		template<typename A, typename = std::enable_if_t< IsVec&& IsInteger<Matrix>&& SameScalarType<Matrix, A> && (Infos<A>::IsScalar || SameSize<Matrix, A>) >  >
		void operator&=(A&& a)&& {
			(void)Matrix(make_expr<BinaryOperator>(Op::BitwiseAndAssignment, NamedObjectBase::get_expr_as_temp(), EXPR(A, a)));
		}

		// ++
		template<bool b = !IsBool, typename = std::enable_if_t<b> >
		Matrix operator++()& {
			return { make_expr<UnaryOperator>(Op::PrefixIncrement, NamedObjectBase::get_expr_as_ref()) };
		}
		template<bool b = !IsBool, typename = std::enable_if_t<b> >
		Matrix operator++()&& {
			return { make_expr<UnaryOperator>(Op::PrefixIncrement, NamedObjectBase::get_expr_as_temp()) };
		}

		template<bool b = !IsBool, typename = std::enable_if_t<b> >
		Matrix operator++(int)& {
			return { make_expr<UnaryOperator>(Op::PostfixIncrement, NamedObjectBase::get_expr_as_ref()) };
		}
		template<bool b = !IsBool, typename = std::enable_if_t<b> >
		Matrix operator++(int)&& {
			return { make_expr<UnaryOperator>(Op::PostfixIncrement, NamedObjectBase::get_expr_as_temp()) };
		}

		// --
		template<bool b = !IsBool, typename = std::enable_if_t<b> >
		Matrix operator--()& {
			return { make_expr<UnaryOperator>(Op::PrefixDecrement, NamedObjectBase::get_expr_as_ref()) };
		}
		template<bool b = !IsBool, typename = std::enable_if_t<b> >
		Matrix operator--()&& {
			return { make_expr<UnaryOperator>(Op::PrefixDecrement, NamedObjectBase::get_expr_as_temp()) };
		}

		template<bool b = !IsBool, typename = std::enable_if_t<b> >
		Matrix operator--(int)& {
			return { make_expr<UnaryOperator>(Op::PostfixDecrement, NamedObjectBase::get_expr_as_ref()) };
		}
		template<bool b = !IsBool, typename = std::enable_if_t<b> >
		Matrix operator--(int)&& {
			return { make_expr<UnaryOperator>(Op::PostfixDecrement, NamedObjectBase::get_expr_as_temp()) };
		}

		template<bool b = IsBool, typename = std::enable_if_t<b> >
		Matrix operator!()& {
			return { make_expr<UnaryOperator>(Op::UnaryNegation, NamedObjectBase::get_expr_as_ref()) };
		}
		template<bool b = IsBool, typename = std::enable_if_t<b> >
		Matrix operator!()&& {
			return { make_expr<UnaryOperator>(Op::UnaryNegation, NamedObjectBase::get_expr_as_temp()) };
		}
	};

	// operator*
	template<typename A, typename B, typename = std::enable_if_t<
		!Infos<A>::IsBool&& SameScalarType<A, B> && (Infos<A>::IsScalar || Infos<B>::IsScalar || SameSize<A, B> || Infos<A>::ColCount == Infos<B>::RowCount)>>
		typename AlgebraMulInfos<A, B>::ReturnType operator*(A&& a, B&& b)
	{
		return { make_expr<BinaryOperator>(AlgebraMulInfos<A, B>::Operator, EXPR(A,a), EXPR(B,b)) };
	}

	// operator /
	template<typename A, typename B, typename = std::enable_if_t<
		!Infos<A>::IsBool&& SameScalarType<A, B> && (Infos<A>::IsScalar || Infos<B>::IsScalar || SameSize<A, B>)>>
		typename AlgebraInfos<A, B>::ReturnType operator/(A&& a, B&& b)
	{
		return { make_expr<BinaryOperator>(AlgebraInfos<A, B>::OperatorDiv, EXPR(A,a), EXPR(B,b)) };
	}

	// operator +
	template<typename A, typename B, typename = std::enable_if_t<
		!Infos<A>::IsBool&& SameScalarType<A, B> && (Infos<A>::IsScalar || Infos<B>::IsScalar || SameSize<A, B>)>>
		typename AlgebraInfos<A, B>::ReturnType operator+(A&& a, B&& b)
	{
		return { make_expr<BinaryOperator>(AlgebraInfos<A, B>::OperatorAdd, EXPR(A,a), EXPR(B,b)) };
	}

	// operator -
	template<typename A, typename B, typename = std::enable_if_t<
		!Infos<A>::IsBool&& SameScalarType<A, B> && (Infos<A>::IsScalar || Infos<B>::IsScalar || SameSize<A, B>)>>
		typename AlgebraInfos<A, B>::ReturnType operator-(A&& a, B&& b)
	{
		return { make_expr<BinaryOperator>(AlgebraInfos<A, B>::OperatorSub, EXPR(A,a), EXPR(B,b)) };
	}

	// operator <
	template<typename A, typename B, typename = std::enable_if_t< Infos<A>::IsScalar&& SameMat<A, B> >>
	Scalar<bool> operator<(A&& a, B&& b) {
		return { make_expr<BinaryOperator>(Op::ScalarLessThanScalar, EXPR(A,a), EXPR(B,b)) };
	}

	// operator >
	template<typename A, typename B, typename = std::enable_if_t< Infos<A>::IsScalar >>
	Scalar<bool> operator>(A&& a, B&& b) {
		return { make_expr<BinaryOperator>(Op::ScalarGreaterThanScalar, EXPR(A,a), EXPR(B,b)) };
	}

	// operator <= 
	template<typename A, typename B, typename = std::enable_if_t< Infos<A>::IsScalar&& SameMat<A, B> >>
	Scalar<bool> operator<=(A&& a, B&& b) {
		return { make_expr<BinaryOperator>(Op::ScalarLessThanEqualScalar, EXPR(A,a), EXPR(B,b)) };
	}

	// operator <= 
	template<typename A, typename B, typename = std::enable_if_t< Infos<A>::IsScalar&& SameMat<A, B> >>
	Scalar<bool> operator>=(A&& a, B&& b) {
		return { make_expr<BinaryOperator>(Op::ScalarGreaterThanEqualScalar, EXPR(A,a), EXPR(B,b)) };
	}

	//operator ||
	template<typename A, typename B, typename = std::enable_if_t< SameMat<A, bool>&& SameMat<A, B>>>
	Scalar<bool> operator||(A&& a, B&& b) {
		return { make_expr<BinaryOperator>(Op::LogicalOr, EXPR(A,a), EXPR(B,b)) };
	}

	//operator &
	template<typename A, typename B, typename = std::enable_if_t< IsInteger<A>&& IsInteger<B> && (Infos<A>::IsScalar || Infos<B>::IsScalar || SameSize<A, B>)>>
	typename AlgebraInfos<A, B>::ReturnType operator&(A&& a, B&& b) {
		return { make_expr<BinaryOperator>(Op::BitwiseAnd, EXPR(A,a), EXPR(B,b)) };
	}

	//operator &&
	template<typename A, typename B, typename = std::enable_if_t< SameMat<A, bool>&& SameMat<A, B>>>
	Scalar<bool> operator&&(A&& a, B&& b) {
		return { make_expr<BinaryOperator>(Op::LogicalAnd, EXPR(A,a), EXPR(B,b)) };
	}

	//operator ==
	template<typename A, typename B, typename = std::enable_if_t< Infos<A>::IsScalar&& SameMat<A, B> >>
	Scalar<bool> operator==(A&& a, B&& b) {
		return { make_expr<BinaryOperator>(Op::Equality, EXPR(A,a), EXPR(B,b)) };
	}

	//operator !=
	template<typename A, typename B, typename = std::enable_if_t< Infos<A>::IsScalar&& SameMat<A, B> >>
	Scalar<bool> operator!=(A&& a, B&& b) {
		return { make_expr<BinaryOperator>(Op::NotEquality, EXPR(A,a), EXPR(B,b)) };
	}

	// operator |
	template<typename A, typename B, typename = std::enable_if_t< Infos<A>::IsVec&& Infos<A>::IsInteger&& SameMat<A, B> >>
	Vector<typename Infos<A>::ScalarType, Infos<A>::RowCount> operator|(A&& a, B&& b) {
		return { make_expr<BinaryOperator>(Op::BitwiseOr, EXPR(A,a), EXPR(B,b)) };

	}

	// operator <<
	template<typename A, typename B, typename = std::enable_if_t<Infos<A>::IsVec&& Infos<A>::IsInteger&& SameScalarType<A, B> && (SameSize<A, B> || Infos<B>::IsScalar) >>
	Vector<typename Infos<A>::ScalarType, Infos<A>::RowCount> operator<<(A&& a, B&& b) {
		return { make_expr<BinaryOperator>(Op::BitwiseLeftShift, EXPR(A,a), EXPR(B,b)) };
	}

	// operator >>
	template<typename A, typename B, typename = std::enable_if_t< Infos<A>::IsVec&& Infos<A>::IsInteger&& SameScalarType<A, B> && (SameSize<A, B> || Infos<B>::IsScalar)>>
	Vector<typename Infos<A>::ScalarType, Infos<A>::RowCount> operator>>(A&& a, B&& b) {
		return { make_expr<BinaryOperator>(Op::BitwiseRightShift, EXPR(A,a), EXPR(B,b)) };
	}

	//template< typename T, typename Ds, std::size_t R, std::size_t C, typename ... Qs>
	//class MatrixArray : public NamedObject<MatrixArray<T, Ds, R, C, Qs...>> {
	//public:

	//	using Qualifiers = TList<Qs...>;
	//	using Base = NamedObject<MatrixArray<T, Ds, R, C, Qs...>>;

	//	using ArrayComponent = MatrixInterface<T, R, C, typename GetArrayFromList<typename Ds::Tail>::Type, Qs...>;
	//	static constexpr bool IsArray = true;

	//	using ArrayDimensions = Ds;
	//	static constexpr std::size_t ComponentCount = Ds::Front;

	//	MatrixArray() : Base() {}

	//	template<std::size_t N>
	//	explicit MatrixArray(const char(&name)[N]) : Base(name) {}

	//	MatrixArray(const Expr& expr) : Base(expr) { }

	//	template<typename U, typename V, typename ... Us, typename = std::enable_if_t<
	//		!(std::is_same_v<Expr, Us> || ...) && (SameType<Us, ArrayComponent> && ...) && (ComponentCount == 0 || 2 + sizeof...(Us) == ComponentCount)
	//		>>
	//		explicit MatrixArray(U&& u, V&& v, Us&& ... us) : Base("", ObjFlags::Default, CtorFlags::Initialisation, EXPR(Us, us)...)
	//	{
	//	}

	//	template<typename Index>
	//	ArrayComponent operator [](Index&& index) const& {
	//		return { make_expr<ArraySubscript>(NamedObjectBase::get_expr_as_ref(), EXPR(Index, index)) };
	//	}

	//	template<typename Index>
	//	ArrayComponent operator [](Index&& index) const&& {
	//		return { make_expr<ArraySubscript>(NamedObjectBase::get_expr_as_temp(), EXPR(Index, index)) };
	//	}

	//};

	//template< typename T, std::size_t R, std::size_t C, typename ...Qs>
	//struct MatrixIndirection<T, R, C, TList<Qs...>> {
	//	using Type = Matrix<T, R, C, Qs...>;
	//};

	//template< typename T, typename Ds, std::size_t R, std::size_t C, typename ...Qs>
	//struct MatrixArrayIndirection<T, Ds, R, C, TList<Qs...>> {
	//	using Type = MatrixArray<T, Ds, R, C, Qs...>;
	//};

	//template<typename T, std::size_t R, std::size_t C, typename ... Qs>
	//struct QualifiedIndirection<Matrix<T, R, C>, Qs... > {
	//	using Type = MatrixInterface<T, R, C, Qs...>;
	//};

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