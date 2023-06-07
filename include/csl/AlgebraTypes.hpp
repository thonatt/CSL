#pragma once

#include "Controllers.hpp"
#include "NamedObjects.hpp"
#include "Swizzles.hpp"
#include "Types.hpp"

#include <typeinfo>
#include <vector>
#include <array>

#define EXPR(type, var) get_expr(std::forward<type>(var))
#define ENABLE_IF(...) std::enable_if_t<(__VA_ARGS__), int> = 0

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
		explicit operator bool()&
		{
			static_assert(IsBool, "Wrong conversion");

			if (context::shader_active())
				context::get().stacking_for_condition(Base::get_expr_as_ref());

			return false;
		}

		explicit operator bool()&&
		{
			static_assert(IsBool, "Wrong conversion");

			if (context::shader_active())
				context::get().stacking_for_condition(Base::get_expr_as_temp());

			return false;
		}

		Matrix(NoInit) : Base() {}

		Matrix() : Base("", ObjFlags::Default, {}, {}) {}

		template<std::size_t N>
		Matrix(const char(&name)[N], const ObjFlags obj_flags = ObjFlags::Default)
			: Base(name, obj_flags, {}, {}) {}

		Matrix(const Expr& expr, const ObjFlags obj_flags = ObjFlags::Default)
			: Base(expr, obj_flags) {}

		//TODO add is_convertible_to
		template<typename U, typename V, typename ...Vs,
			ENABLE_IF((NumElements<U, V, Vs...> == R * C) && SameScalarType<U, V, Vs...>) >
		explicit Matrix(U&& u, V&& v, Vs&&...vs)
			: Base("", ObjFlags::Default, CtorFlags::Initialisation, SizeList<>{}, TList<>{}, EXPR(U, u), EXPR(V, v), EXPR(Vs, vs)...)
		{
		}

		Matrix(Matrix&& other) : Base(other.get_expr_as_temp())
		{
		}

		template<typename U, ENABLE_IF(SameSize<Matrix, U>&& SameScalarType<Matrix, U>)>
		Matrix(const NamedObjectInit<U>& init) : Base(init) {}


		//TODO add IsConvertibleTo<Infos<M>::ScalarType, This>
		template<typename M, ENABLE_IF(SameSize<This, M> || Infos<M>::IsScalar)>
		Matrix(M&& other) : Matrix(make_expr<ConvertorOperator< M, This>>(EXPR(M, other))) { }

		template<typename M, ENABLE_IF(SameSize<Matrix, M>&& SameScalarType<Matrix, M>)>
		Matrix operator=(M&& other)& {
			return { make_expr<BinaryOperator>(Op::Assignment, NamedObjectBase::get_expr_as_ref(), EXPR(M,other)) };
		}

		template<typename M, ENABLE_IF(SameSize<Matrix, M>&& SameScalarType<Matrix, M>)>
		Matrix operator=(M&& other)&& {
			return { make_expr<BinaryOperator>(Op::Assignment, NamedObjectBase::get_expr_as_temp(), EXPR(M,other)) };
		}

		// swizzling
		template<char ...cs, ENABLE_IF(C == 1 && R != 1 && swizzling::SwizzleInfo<cs...>::HighestComponent <= R)>
		std::conditional_t<swizzling::SwizzleInfo<cs...>::NoRepetition, SubCol<sizeof...(cs)>, const SubCol<sizeof...(cs)>> operator()(Swizzle<cs> ... swizzles)& {
			return { make_expr<Swizzling<cs...>>(NamedObjectBase::get_expr_as_ref()) };
		}

		template<char ...cs, ENABLE_IF(C == 1 && R != 1 && swizzling::SwizzleInfo<cs...>::HighestComponent <= R)>
		std::conditional_t<swizzling::SwizzleInfo<cs...>::NoRepetition, SubCol<sizeof...(cs)>, const SubCol<sizeof...(cs)>> operator()(Swizzle<cs> ... swizzles)&& {
			return { make_expr<Swizzling<cs...>>(NamedObjectBase::get_expr_as_temp()) };
		}

		template<char ...cs, ENABLE_IF(C == 1 && R != 1 && swizzling::SwizzleInfo<cs...>::HighestComponent <= R)>
		std::conditional_t<swizzling::SwizzleInfo<cs...>::NoRepetition, SubCol<sizeof...(cs)>, const SubCol<sizeof...(cs)>> operator()(Swizzle<cs> ... swizzles) const& {
			return { make_expr<Swizzling<cs...>>(NamedObjectBase::get_expr_as_ref()) };
		}

		template<char ...cs, ENABLE_IF(C == 1 && R != 1 && swizzling::SwizzleInfo<cs...>::HighestComponent <= R)>
		std::conditional_t<swizzling::SwizzleInfo<cs...>::NoRepetition, SubCol<sizeof...(cs)>, const SubCol<sizeof...(cs)>> operator()(Swizzle<cs> ... swizzles) const&& {
			return { make_expr<Swizzling<cs...>>(NamedObjectBase::get_expr_as_temp()) };
		}

		// col access
		template<typename Index, ENABLE_IF(!IsScalar && IsInteger<Index>)>
		std::conditional_t<C == 1, Scalar, Col> operator[](Index&& index) const&
		{
			return { make_expr<ArraySubscript>(NamedObjectBase::get_expr_as_ref(), EXPR(Index, index)) };
		}

		// unaries
		template<bool b = !IsBool, ENABLE_IF(b)>
		Matrix operator-() const& {
			return { make_expr<UnaryOperator>(Op::UnarySub, NamedObjectBase::get_expr_as_ref()) };
		}

		template<bool b = !IsBool, ENABLE_IF(b)>
		Matrix operator-() const&& {
			return { make_expr<UnaryOperator>(Op::UnarySub, NamedObjectBase::get_expr_as_temp()) };
		}

		// X=
		template<typename A, ENABLE_IF(!IsBool && SameScalarType<This, A> && (SameSize<This, A> || Infos<A>::IsScalar))>
		void operator+=(A&& a)& {
			(void)Matrix(make_expr<BinaryOperator>(Op::AddAssignment, NamedObjectBase::get_expr_as_ref(), EXPR(A, a)));
		}

		template<typename A, ENABLE_IF(!IsBool && SameScalarType<This, A> && (SameSize<This, A> || Infos<A>::IsScalar))>
		void operator+=(A&& a)&& {
			(void)Matrix(make_expr<BinaryOperator>(Op::AddAssignment, NamedObjectBase::get_expr_as_temp(), EXPR(A, a)));
		}

		template<typename A, ENABLE_IF(!IsBool && SameScalarType<This, A> && (SameSize<This, A> || Infos<A>::IsScalar))>
		void operator-=(A&& a)& {
			(void)Matrix(make_expr<BinaryOperator>(Op::SubAssignment, NamedObjectBase::get_expr_as_ref(), EXPR(A, a)));
		}

		template<typename A, ENABLE_IF(!IsBool && SameScalarType<This, A> && (SameSize<This, A> || Infos<A>::IsScalar))>
		void operator-=(A&& a)&& {
			(void)Matrix(make_expr<BinaryOperator>(Op::SubAssignment, NamedObjectBase::get_expr_as_temp(), EXPR(A, a)));
		}

		template<typename A, ENABLE_IF(!IsBool && SameScalarType<This, A> && (SameSize<This, A> || Infos<A>::IsScalar))>
		void operator*=(A&& a)& {
			(void)Matrix(make_expr<BinaryOperator>(Op::MulAssignment, NamedObjectBase::get_expr_as_ref(), EXPR(A, a)));
		}

		template<typename A, ENABLE_IF(!IsBool && SameScalarType<This, A> && (SameSize<This, A> || Infos<A>::IsScalar))>
		void operator*=(A&& a)&& {
			(void)Matrix(make_expr<BinaryOperator>(Op::MulAssignment, NamedObjectBase::get_expr_as_temp(), EXPR(A, a)));
		}

		template<typename A, ENABLE_IF(!IsBool && SameScalarType<This, A> && (SameSize<This, A> || Infos<A>::IsScalar))>
		void operator/=(A&& a)& {
			(void)Matrix(make_expr<BinaryOperator>(Op::DivAssignment, NamedObjectBase::get_expr_as_ref(), EXPR(A, a)));
		}

		template<typename A, ENABLE_IF(!IsBool && SameScalarType<This, A> && (SameSize<This, A> || Infos<A>::IsScalar))>
		void operator/=(A&& a)&& {
			(void)Matrix(make_expr<BinaryOperator>(Op::DivAssignment, NamedObjectBase::get_expr_as_temp(), EXPR(A, a)));
		}

		template<typename A, ENABLE_IF(IsVec&& IsInteger<Matrix>&& SameScalarType<Matrix, A> && (Infos<A>::IsScalar || SameSize<Matrix, A>))>
		void operator&=(A&& a)& {
			(void)Matrix(make_expr<BinaryOperator>(Op::BitwiseAndAssignment, NamedObjectBase::get_expr_as_ref(), EXPR(A, a)));
		}

		template<typename A, ENABLE_IF(IsVec&& IsInteger<Matrix>&& SameScalarType<Matrix, A> && (Infos<A>::IsScalar || SameSize<Matrix, A>))>
		void operator&=(A&& a)&& {
			(void)Matrix(make_expr<BinaryOperator>(Op::BitwiseAndAssignment, NamedObjectBase::get_expr_as_temp(), EXPR(A, a)));
		}

		// ++
		template<bool b = !IsBool, ENABLE_IF(b)>
		Matrix operator++()& {
			return { make_expr<UnaryOperator>(Op::PrefixIncrement, NamedObjectBase::get_expr_as_ref()) };
		}
		template<bool b = !IsBool, ENABLE_IF(b)>
		Matrix operator++()&& {
			return { make_expr<UnaryOperator>(Op::PrefixIncrement, NamedObjectBase::get_expr_as_temp()) };
		}

		template<bool b = !IsBool, ENABLE_IF(b)>
		Matrix operator++(int)& {
			return { make_expr<UnaryOperator>(Op::PostfixIncrement, NamedObjectBase::get_expr_as_ref()) };
		}
		template<bool b = !IsBool, ENABLE_IF(b)>
		Matrix operator++(int)&& {
			return { make_expr<UnaryOperator>(Op::PostfixIncrement, NamedObjectBase::get_expr_as_temp()) };
		}

		// --
		template<bool b = !IsBool, ENABLE_IF(b)>
		Matrix operator--()& {
			return { make_expr<UnaryOperator>(Op::PrefixDecrement, NamedObjectBase::get_expr_as_ref()) };
		}
		template<bool b = !IsBool, ENABLE_IF(b)>
		Matrix operator--()&& {
			return { make_expr<UnaryOperator>(Op::PrefixDecrement, NamedObjectBase::get_expr_as_temp()) };
		}

		template<bool b = !IsBool, ENABLE_IF(b)>
		Matrix operator--(int)& {
			return { make_expr<UnaryOperator>(Op::PostfixDecrement, NamedObjectBase::get_expr_as_ref()) };
		}
		template<bool b = !IsBool, ENABLE_IF(b)>
		Matrix operator--(int)&& {
			return { make_expr<UnaryOperator>(Op::PostfixDecrement, NamedObjectBase::get_expr_as_temp()) };
		}

		// !
		template<bool b = IsBool, ENABLE_IF(b)>
		Matrix operator!()& {
			return { make_expr<UnaryOperator>(Op::UnaryNegation, NamedObjectBase::get_expr_as_ref()) };
		}
		template<bool b = IsBool, ENABLE_IF(b)>
		Matrix operator!()&& {
			return { make_expr<UnaryOperator>(Op::UnaryNegation, NamedObjectBase::get_expr_as_temp()) };
		}
	};

	// operator*
	template<typename A, typename B, ENABLE_IF(
		!Infos<A>::IsBool&& SameScalarType<A, B> && (Infos<A>::IsScalar || Infos<B>::IsScalar || SameSize<A, B> || Infos<A>::ColCount == Infos<B>::RowCount))>
		typename AlgebraMulInfos<A, B>::ReturnType operator*(A&& a, B&& b)
	{
		return { make_expr<BinaryOperator>(AlgebraMulInfos<A, B>::Operator, EXPR(A,a), EXPR(B,b)) };
	}

	// operator /
	template<typename A, typename B, ENABLE_IF(
		!Infos<A>::IsBool&& SameScalarType<A, B> && (Infos<A>::IsScalar || Infos<B>::IsScalar || SameSize<A, B>))>
		typename AlgebraInfos<A, B>::ReturnType operator/(A&& a, B&& b)
	{
		return { make_expr<BinaryOperator>(AlgebraInfos<A, B>::OperatorDiv, EXPR(A,a), EXPR(B,b)) };
	}

	// operator +
	template<typename A, typename B, ENABLE_IF(
		!Infos<A>::IsBool&& SameScalarType<A, B> && (Infos<A>::IsScalar || Infos<B>::IsScalar || SameSize<A, B>))>
		typename AlgebraInfos<A, B>::ReturnType operator+(A&& a, B&& b)
	{
		return { make_expr<BinaryOperator>(AlgebraInfos<A, B>::OperatorAdd, EXPR(A,a), EXPR(B,b)) };
	}

	// operator -
	template<typename A, typename B, ENABLE_IF(
		!Infos<A>::IsBool&& SameScalarType<A, B> && (Infos<A>::IsScalar || Infos<B>::IsScalar || SameSize<A, B>))>
		typename AlgebraInfos<A, B>::ReturnType operator-(A&& a, B&& b)
	{
		return { make_expr<BinaryOperator>(AlgebraInfos<A, B>::OperatorSub, EXPR(A,a), EXPR(B,b)) };
	}

	// operator <
	template<typename A, typename B, ENABLE_IF(Infos<A>::IsScalar&& SameMat<A, B>)>
	Scalar<bool> operator<(A&& a, B&& b) {
		return { make_expr<BinaryOperator>(Op::ScalarLessThanScalar, EXPR(A,a), EXPR(B,b)) };
	}

	// operator >
	template<typename A, typename B, ENABLE_IF(Infos<A>::IsScalar)>
	Scalar<bool> operator>(A&& a, B&& b) {
		return { make_expr<BinaryOperator>(Op::ScalarGreaterThanScalar, EXPR(A,a), EXPR(B,b)) };
	}

	// operator <= 
	template<typename A, typename B, ENABLE_IF(Infos<A>::IsScalar&& SameMat<A, B>)>
	Scalar<bool> operator<=(A&& a, B&& b) {
		return { make_expr<BinaryOperator>(Op::ScalarLessThanEqualScalar, EXPR(A,a), EXPR(B,b)) };
	}

	// operator <= 
	template<typename A, typename B, ENABLE_IF(Infos<A>::IsScalar&& SameMat<A, B>)>
	Scalar<bool> operator>=(A&& a, B&& b) {
		return { make_expr<BinaryOperator>(Op::ScalarGreaterThanEqualScalar, EXPR(A,a), EXPR(B,b)) };
	}

	//operator ||
	template<typename A, typename B, ENABLE_IF(SameMat<A, bool>&& SameMat<A, B>)>
	Scalar<bool> operator||(A&& a, B&& b) {
		return { make_expr<BinaryOperator>(Op::LogicalOr, EXPR(A,a), EXPR(B,b)) };
	}

	//operator &
	template<typename A, typename B, ENABLE_IF(IsInteger<A>&& IsInteger<B> && (Infos<A>::IsScalar || Infos<B>::IsScalar || SameSize<A, B>))>
	typename AlgebraInfos<A, B>::ReturnType operator&(A&& a, B&& b) {
		return { make_expr<BinaryOperator>(Op::BitwiseAnd, EXPR(A,a), EXPR(B,b)) };
	}

	//operator &&
	template<typename A, typename B, ENABLE_IF(SameMat<A, bool>&& SameMat<A, B>)>
	Scalar<bool> operator&&(A&& a, B&& b) {
		return { make_expr<BinaryOperator>(Op::LogicalAnd, EXPR(A,a), EXPR(B,b)) };
	}

	//operator ==
	template<typename A, typename B, ENABLE_IF(Infos<A>::IsScalar&& SameMat<A, B>)>
	Scalar<bool> operator==(A&& a, B&& b) {
		return { make_expr<BinaryOperator>(Op::Equality, EXPR(A,a), EXPR(B,b)) };
	}

	//operator !=
	template<typename A, typename B, ENABLE_IF(Infos<A>::IsScalar&& SameMat<A, B>)>
	Scalar<bool> operator!=(A&& a, B&& b) {
		return { make_expr<BinaryOperator>(Op::NotEquality, EXPR(A,a), EXPR(B,b)) };
	}

	// operator |
	template<typename A, typename B, ENABLE_IF(Infos<A>::IsVec&& IsInteger<A>&& SameMat<A, B>)>
	Vector<typename Infos<A>::ScalarType, Infos<A>::RowCount> operator|(A&& a, B&& b) {
		return { make_expr<BinaryOperator>(Op::BitwiseOr, EXPR(A,a), EXPR(B,b)) };
	}

	// operator <<
	template<typename A, typename B, ENABLE_IF(Infos<A>::IsVec&& IsInteger<A>&& SameScalarType<A, B> && (SameSize<A, B> || Infos<B>::IsScalar))>
	Vector<typename Infos<A>::ScalarType, Infos<A>::RowCount> operator<<(A&& a, B&& b) {
		return { make_expr<BinaryOperator>(Op::BitwiseLeftShift, EXPR(A,a), EXPR(B,b)) };
	}

	// operator >>
	template<typename A, typename B, ENABLE_IF(Infos<A>::IsVec&& IsInteger<A>&& SameScalarType<A, B> && (SameSize<A, B> || Infos<B>::IsScalar))>
	Vector<typename Infos<A>::ScalarType, Infos<A>::RowCount> operator>>(A&& a, B&& b) {
		return { make_expr<BinaryOperator>(Op::BitwiseRightShift, EXPR(A,a), EXPR(B,b)) };
	}

} //namespace csl

#undef ENABLE_IF
#undef EXPR