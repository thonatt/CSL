#pragma once

#include "NamedObjects.hpp"
#include "Swizzles.hpp"

#define EX(type, var) getExp(std::forward<type>(var))

namespace csl {

	namespace core {

		template<typename T>
		struct MatrixConvertor { };

		template<> struct MatrixConvertor<Int> {
			operator int() {
				return 0;
			}
		};

		template<> struct MatrixConvertor<Uint> {
			operator uint() {
				return 0;
			}
		};

		template<> struct MatrixConvertor<Bool> {
			operator bool() &;

			operator bool() && {
				//std::cout << " bool const && " << NamedObjectBase::getExTmp()->str() << std::endl;
				return false;
			}
		};

		/// matrix class

		template<ScalarType type, uint NR, uint NC>
		class Matrix : public NamedObject<Matrix<type, NR, NC>>, public MatrixConvertor<Matrix<type, NR, NC>> {

		public:

		protected:
			static const bool isBool = (type == BOOL);
			static const bool isScalar = (NC == 1 && NR == 1);

		public:

			explicit Matrix(const std::string& _name, ObjFlags obj_flags)
				: NamedObject<Matrix>(_name, obj_flags)
			{
			}

			explicit Matrix(
				const string& _name,
				OpFlags ctor_flags
			) : NamedObject<Matrix>(_name, ctor_flags)
			{
			}

			Matrix(const std::string& name = "")
				: Matrix(name, ObjFlags::IS_TRACKED)
			{
			}

			template<size_t N>
			Matrix(const char(&s)[N])
				: Matrix(s, ObjFlags::IS_TRACKED)
			{
			}

			Matrix(const Ex & _ex, OpFlags ctor_flags = OpFlags::NONE, ObjFlags obj_flags = ObjFlags::IS_TRACKED, const std::string & s = "")
				: NamedObject<Matrix>(_ex, ctor_flags, obj_flags, s)
			{
			}

			Matrix(const NamedObjectInit<Matrix> & obj) : NamedObject<Matrix>(obj) {}

			Matrix(const Matrix & other) : NamedObject<Matrix>(other) {}

			Matrix(Matrix && other) : NamedObject<Matrix>(other) {}

			//glsl constructors

			// matXY(a,b,...)
			template<typename U, typename V, typename ...Us, typename = std::enable_if_t <
				AreValid<U, V, Us...> && MatElements<U, V, Us... > == NR * NC
			> >
				explicit Matrix(U && u, V && v, Us && ...us)
				: NamedObject<Matrix>(
					OpFlags::PARENTHESIS | OpFlags::DISPLAY_TYPE, 
					ObjFlags::IS_TRACKED, 
					"",
					EX(U, u), EX(V, v), EX(Us, us)...)
			{
			}


			// matXY(cpp types) and matXY(matWZ)
			template<typename T, typename = std::enable_if_t <
				EqualType<Matrix, T> || EqualDim<Matrix, T>
				|| std::is_same<T, int>::value
			> >
				Matrix(T && x)
				: NamedObject<Matrix>(
					EqualMat<Matrix, T> ? OpFlags::NONE : (OpFlags::DISPLAY_TYPE | OpFlags::PARENTHESIS),
					ObjFlags::IS_TRACKED, 
					"",
					EX(T, x))
			{
			}

			// operators =

			Matrix operator=(Matrix && other) & {
				return { createExp<MiddleOperator<ASSIGNMENT>>(" = ", NamedObjectBase::getExRef(), other.getExTmp()) };
			}

			Matrix operator=(Matrix && other) && {
				return { createExp<MiddleOperator<ASSIGNMENT>>(" = ", NamedObjectBase::getExTmp(), other.getExTmp()) };
			}

			Matrix operator=(const Matrix & other) & {
				return { createExp<MiddleOperator<ASSIGNMENT>>(" = ", NamedObjectBase::getExRef(), other.getExRef()) };
			}

			Matrix operator=(const Matrix & other) && {
				return { createExp<MiddleOperator<ASSIGNMENT>>(" = ", NamedObjectBase::getExTmp(), other.getExRef()) };
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// swizzles accessors

			template<SwizzleSet Set, uint Dim, uint Bytes, uint Size,
				typename = std::enable_if_t<NC == 1 && NR != 1 && Set != MIXED_SET && Dim <= NR > >
				Vec<type, Size> operator[](const SwizzlePack<Set, Dim, Bytes, Size, REPEATED> & swizzle) const &
			{
				return { createExp<MemberAccessor>(NamedObjectBase::getExRef(), swizzle.getStr()) };
			}

			template<SwizzleSet Set, uint Dim, uint Bytes, uint Size,
				typename = std::enable_if_t<NC == 1 && NR != 1 && Set != MIXED_SET && Dim <= NR > >
				Vec<type, Size> operator[](const SwizzlePack<Set, Dim, Bytes, Size, NON_REPEATED> & swizzle) const &
			{
				return { createExp<MemberAccessor>(NamedObjectBase::getExRef(), swizzle.getStr()) };
			}

			template<SwizzleSet Set, uint Dim, uint Bytes, uint Size,
				typename = std::enable_if_t<NC == 1 && NR != 1 && Set != MIXED_SET && Dim <= NR > >
				Vec<type, Size> operator[](const SwizzlePack<Set, Dim, Bytes, Size, REPEATED> & swizzle) const &&
			{
				return { createExp<MemberAccessor>(NamedObjectBase::getExTmp(), swizzle.getStr(), MemberAccessor::ObjStatus::TEMP) };
			}

			template<SwizzleSet Set, uint Dim, uint Bytes, uint Size,
				typename = std::enable_if_t<NC == 1 && NR != 1 && Set != MIXED_SET && Dim <= NR > >
				Vec<type, Size> operator[](const SwizzlePack<Set, Dim, Bytes, Size, NON_REPEATED> & swizzle) const &&
			{
				return { createExp<MemberAccessor>(NamedObjectBase::getExTmp(), swizzle.getStr(), MemberAccessor::ObjStatus::TEMP) };
			}

			// array subscript accessor
			template<typename U, typename = std::enable_if_t<!isScalar && IsInteger<U> > >
			typename std::conditional_t< NC == 1, Scalar<type>, Vec<type, NR> > operator[](U && u) const &
			{
				return { createExp<ArraySubscript>(NamedObjectBase::getExRef(), EX(U,u)) };
			}

			// array subscript accessor tmp
			template<typename U, typename = std::enable_if_t<!isScalar && IsInteger<U> > >
			typename std::conditional_t< NC == 1, Scalar<type>, Vec<type, NR> > operator[](U && u) const &&
			{
				return { createExp<ArraySubscript>(NamedObjectBase::getExTmp(), EX(U,u)) };
			}


			/////////////////////////////////////////////////////////////////////////////
			// unary operators

			template<bool b = !isBool, typename = std::enable_if_t<b> >
			Matrix operator++() & {
				return { createExp<PrefixUnary>("++", NamedObjectBase::getExRef()) };
			}
			template<bool b = !isBool, typename = std::enable_if_t<b> >
			Matrix operator++() && {
				return { createExp<PrefixUnary>("++", NamedObjectBase::getExTmp()) };
			}

			template<bool b = !isBool, typename = std::enable_if_t<b> >
			Matrix operator++(int) & {
				return { createExp<PostfixUnary>("++ ", NamedObjectBase::getExRef()) };
			}
			template<bool b = !isBool, typename = std::enable_if_t<b> >
			Matrix operator++(int) && {
				return { createExp<PostfixUnary>("++ ", NamedObjectBase::getExTmp()) };
			}

			template<bool b = !isBool, typename = std::enable_if_t<b> >
			Matrix operator--() & {
				return { createExp<PrefixUnary>("--", NamedObjectBase::getExRef()) };
			}
			template<bool b = !isBool, typename = std::enable_if_t<b> >
			Matrix operator--() && {
				return { createExp<PrefixUnary>("--", NamedObjectBase::getExTmp()) };
			}

			template<bool b = !isBool, typename = std::enable_if_t<b> >
			Matrix operator--(int) & {
				return { createExp<PostfixUnary>("-- ", NamedObjectBase::getExRef()) };
			}
			template<bool b = !isBool, typename = std::enable_if_t<b> >
			Matrix operator--(int) && {
				return { createExp<PostfixUnary>("-- ", NamedObjectBase::getExTmp()) };
			}

			template<bool b = !isBool, typename = std::enable_if_t<b> >
			Matrix operator-() & {
				return { createExp<PrefixUnary>("-", NamedObjectBase::getExRef()) };
			}
			template<bool b = !isBool, typename = std::enable_if_t<b> >
			Matrix operator-() && {
				return { createExp<PrefixUnary>("-", NamedObjectBase::getExTmp()) };
			}

			template<bool b = IsVec<Matrix, BOOL>, typename = std::enable_if_t<b> >
			Matrix operator!() & {
				if (isScalar) {
					return { createExp<PrefixUnary>("!", NamedObjectBase::getExRef()) };
				}
				return { createFCallExp("not", NamedObjectBase::getExRef()) };
			}

			template<bool b = IsVec<Matrix, BOOL>, typename = std::enable_if_t<b> >
			Matrix operator!() && {
				if (isScalar) {
					return { createExp<PrefixUnary>("!", NamedObjectBase::getExTmp()) };
				}
				return { createFCallExp("not", NamedObjectBase::getExTmp()) };
			}

			////////////////////////////////////////////////////////////////////////////////
			////operators X=

			template<typename A,
				typename = std::enable_if_t<NotBool<A> && (EqualMat<Matrix, A> || IsScalar<A>)  >  >
				void operator+=(A && a) & {
				Matrix(createExp<MiddleOperator<ASSIGNMENT>>(" += ", NamedObjectBase::getExRef(), EX(A, a)));
			}

			template<typename A,
				typename = std::enable_if_t<NotBool<A> && (EqualMat<Matrix, A> || IsScalar<A>)  >  >
				void operator+=(A && a) && {
				Matrix(createExp<MiddleOperator<ASSIGNMENT>>(" += ", NamedObjectBase::getExTmp(), EX(A, a)));
			}

			template<typename A,
				typename = std::enable_if_t<NotBool<A> && (EqualMat<Matrix, A> || IsScalar<A>)  >  >
				void operator-=(A && a)  & {
				Matrix(createExp<MiddleOperator<ASSIGNMENT>>(" -= ", NamedObjectBase::getExRef(), EX(A, a)));
			}

			template<typename A,
				typename = std::enable_if_t<NotBool<A> && (EqualMat<Matrix, A> || IsScalar<A>)  >  >
				void operator-=(A && a) && {
				Matrix(createExp<MiddleOperator<ASSIGNMENT>>(" -= ", NamedObjectBase::getExTmp(), EX(A, a)));
			}

			template<typename A,
				typename = std::enable_if_t<NotBool<A> && (EqualMat<Matrix, A> || IsScalar<A>)  >  >
				void operator*=(A && a)  & {
				Matrix(createExp<MiddleOperator<ASSIGNMENT>>(" *= ", NamedObjectBase::getExRef(), EX(A, a)));
			}

			template<typename A,
				typename = std::enable_if_t<NotBool<A> && (EqualMat<Matrix, A> || IsScalar<A>)  >  >
				void operator*=(A && a) && {
				Matrix(createExp<MiddleOperator<ASSIGNMENT>>(" *= ", NamedObjectBase::getExTmp(), EX(A, a)));
			}

			template<typename A,
				typename = std::enable_if_t<NotBool<A> && (EqualMat<Matrix, A> || IsScalar<A>)  >  >
				void operator/=(A && a)  & {
				Matrix(createExp<MiddleOperator<ASSIGNMENT>>(" /= ", NamedObjectBase::getExRef(), EX(A, a)));
			}

			template<typename A,
				typename = std::enable_if_t<NotBool<A> && (EqualMat<Matrix, A> || IsScalar<A>)  >  >
				void operator/=(A && a) && {
				Matrix(createExp<MiddleOperator<ASSIGNMENT>>(" /= ", NamedObjectBase::getExTmp(), EX(A, a)));
			}

			template<typename A, typename = std::enable_if_t<
				IsVecInteger<Matrix> && SameScalarType<Matrix, A> && (IsScalar<A> || EqualDim<Matrix, A>) >  >
				void operator&=(A && a) & {
				Matrix(createExp<MiddleOperator<ASSIGNMENT>>(" &= ", NamedObjectBase::getExRef(), EX(A, a)));
			}

			template<typename A, typename = std::enable_if_t<
				IsVecInteger<Matrix> && SameScalarType<Matrix, A> && (IsScalar<A> || EqualDim<Matrix, A>) >  >
				void operator&=(A && a) && {
				Matrix(createExp<MiddleOperator<ASSIGNMENT>>(" &= ", NamedObjectBase::getExTmp(), EX(A, a)));
			}

			////////////////////////////////////////////////////////////////////////////////////////////////////
			// member functions

			template<bool b = !isScalar, typename = std::enable_if_t<b> >
			Int length() & {
				return { createExp<MemberFunctionAccessor<0>>(NamedObjectBase::getExRef(), "length") };
			}

			Int length() && = delete;
		};

		// logical binary operators
		template<typename A, typename B,
			typename = std::enable_if_t< EqualMat<A, Bool> &&  EqualMat<B, Bool> > >
			Bool operator&&(A && a, B && b)
		{
			return { createExp<MiddleOperator<LOGICAL_AND>>(" && ", EX(A,a), EX(B,b)) };
		}

		template<typename A, typename B,
			typename = std::enable_if_t< EqualMat<A, Bool> &&  EqualMat<B, Bool> > >
			Bool operator||(A && a, B && b)
		{
			return { createExp<MiddleOperator<LOGICAL_OR>>(" || ", EX(A,a), EX(B,b)) };
		}

		template<typename A, typename B,
			typename = std::enable_if_t< EqualDim<A, B> > >
			Bool operator==(A && a, B && b)
		{
			return { createExp<MiddleOperator<EQUALITY>>(" == ", EX(A, a), EX(B, b)) };
		}


		template<typename A, typename B,
			typename = std::enable_if_t< NoBools<A, B> && EqualDim<A, B> > >
			Bool operator!=(A && a, B && b)
		{
			return { createExp<MiddleOperator<EQUALITY>>(" != ", EX(A, a), EX(B, b)) };
		}


		// bitwise operators
		template<typename A, typename B, typename C = Infos<A>, typename =
			std::enable_if_t< IsVecInteger<A> && IsVecInteger<B> && SameScalarType<A, B> && (IsScalar<A> || IsScalar<B> || EqualDim<A, B>) > >
			Vec<C::scalar_type, C::rows> operator&(A && a, B && b)
		{
			return { createExp<MiddleOperator<BITWISE_AND>>(" & ", EX(A, a), EX(B, b)) };
		}

		template<typename A, typename B, typename C = Infos<A>, typename =
			std::enable_if_t< IsVecInteger<A> && IsVecInteger<B> && SameScalarType<A, B> && (IsScalar<A> || IsScalar<B> || EqualDim<A, B>) > >
			Vec<C::scalar_type, C::rows> operator|(A && a, B && b)
		{
			return { createExp<MiddleOperator<BITWISE_OR>>(" | ", EX(A, a), EX(B, b)) };
		}

		template<typename A, typename B, typename C = Infos<A>, typename =
			std::enable_if_t< IsVecInteger<A> && IsVecInteger<B> && ((IsScalar<A> && IsScalar<B>) || IsScalar<B> || EqualDim<A, B>) > >
			Vec<C::scalar_type, C::rows> operator<<(A && a, B && b)
		{
			return { createExp<MiddleOperator<BITWISE_SHIFT>>(" << ", EX(A, a), EX(B, b)) };
		}

		template<typename A, typename B, typename C = Infos<A>, typename =
			std::enable_if_t< IsVecInteger<A> && IsVecInteger<B> && ((IsScalar<A> && IsScalar<B>) || IsScalar<B> || EqualDim<A, B>) > >
			Vec<C::scalar_type, C::rows> operator>>(A && a, B && b)
		{
			return { createExp<MiddleOperator<BITWISE_SHIFT>>(" >> ", EX(A, a), EX(B, b)) };
		}

		// > and < operators
		template<typename A, typename B,
			typename = std::enable_if_t< NoBools<A, B> && IsScalar<A> && IsScalar<B> > >
			Bool operator>(A && a, B && b)
		{
			return { createExp<MiddleOperator<RELATIONAL>>(" > ", EX(A, a), EX(B, b)) };
		}

		template<typename A, typename B,
			typename = std::enable_if_t< NoBools<A, B> && IsScalar<A> && IsScalar<B> > >
			Bool operator<(A && a, B && b)
		{
			return { createExp<MiddleOperator<RELATIONAL>>(" < ", EX(A, a), EX(B, b)) };
		}

		template<typename A, typename B,
			typename = std::enable_if_t< NoBools<A, B> && IsScalar<A> && IsScalar<B> > >
			Bool operator<=(A && a, B && b)
		{
			return { createExp<MiddleOperator<RELATIONAL>>(" <= ", EX(A, a), EX(B, b)) };
		}

		template<typename A, typename B,
			typename = std::enable_if_t< NoBools<A, B> && IsScalar<A> && IsScalar<B> > >
			Bool operator>=(A && a, B && b)
		{
			return { createExp<MiddleOperator<RELATIONAL>>(" >= ", EX(A, a), EX(B, b)) };
		}

		// + and - operators
		template<typename A, typename B,
			typename = std::enable_if_t< NoBools<A, B> && (EqualMat<A, B> || (IsScalar<A> || IsScalar<B>)) > >
			ArithmeticBinaryReturnType<A, B> operator+(A && a, B && b)
		{
			return { createExp<MiddleOperator<ADDITION>>(" + ", EX(A, a), EX(B, b)) };
		}

		template<typename A, typename B,
			typename = std::enable_if_t< NoBools<A, B> && (EqualMat<A, B> || (IsScalar<A> || IsScalar<B>)) > >
			ArithmeticBinaryReturnType<A, B> operator-(A && a, B && b)
		{
			return { createExp<MiddleOperator<SUBSTRACTION>>(" - ", EX(A, a), EX(B, b)) };
		}

		// * operators
		// matrix multiplication
		template<typename A, typename B,
			typename = std::enable_if_t< NoBools<A, B> && ValidForMatMultiplication<A, B> > >
			MultiplicationReturnType<A, B> operator*(A && a, B && b)
		{
			return { createExp<MiddleOperator<MULTIPLY>>("*", EX(A, a), EX(B, b)) };
		}

		// cwise multiplication
		template<typename A, typename B,
			typename = std::enable_if_t< NoBools<A, B> && (IsScalar<A> || IsScalar<B> || (!ValidForMatMultiplication<A, B> && EqualDim<A, B>)) > >
			ArithmeticBinaryReturnType<A, B> operator*(A && a, B && b)
		{
			return { createExp<MiddleOperator<MULTIPLY>>("*", EX(A, a), EX(B, b)) };
		}

		// ()/() operator
		template<typename A, typename B,
			typename = std::enable_if_t< NoBools<A, B> && (EqualMat<A, B> || (IsScalar<A> || IsScalar<B>)) > >
			ArithmeticBinaryReturnType<A, B> operator/(A && a, B && b)
		{
			return { createExp<MiddleOperator<DIVISION>>("/", EX(A, a), EX(B, b)) };
		}
	
		////////////////////////////////////
		// Arrrays
		// N == 0 for unspecified size arrays

		template<typename T, uint N>
		struct ArrayImpl : NamedObject<Array<T, N>> {

			template<uint M> using Size = ArrayImpl<T, M>;

			//template<bool b = (N != 0), typename = std::enable_if_t<b> > //TODO should be here but prevents gl_ClipDistance
			
			ArrayImpl(const std::string & _name = "", ObjFlags obj_flags = ObjFlags::IS_TRACKED)
				: NamedObject<ArrayImpl>(_name, obj_flags)
			{
			}

			ArrayImpl(
				const string& _name,
				OpFlags ctor_flags
			) : NamedObject<ArrayImpl>(_name, ctor_flags)
			{
			}

			ArrayImpl(const Ex & _ex, OpFlags ctor_flags = OpFlags::NONE, ObjFlags obj_flags = ObjFlags::IS_TRACKED, const std::string & s = "")
				: NamedObject<ArrayImpl>(_ex, ctor_flags, obj_flags, s)
			{
			}

			template<typename ... Us, typename = std::enable_if_t<
				AllTrue<EqualMat<Us, T>...> && (N == 0 || sizeof...(Us) == N)
			> >
				ArrayImpl(Us && ... us)
				: NamedObject<ArrayImpl<T, N>>(
					OpFlags::DISPLAY_TYPE | OpFlags::PARENTHESIS,
					ObjFlags::IS_TRACKED | ObjFlags::IS_USED, 
					"",
					EX(Us, us)...
					)
			{
			}

			ArrayImpl(const NamedObjectInit<ArrayImpl> & obj)
				: NamedObject<ArrayImpl>(obj)
			{
			}

			template<typename A,
				typename = std::enable_if_t< IsInteger<A> > >
				T operator[](A && a) const & {
				return { createExp<ArraySubscript>(NamedObjectBase::getExRef(), EX(A,a)) };
			}

			template<typename A,
				typename = std::enable_if_t< IsInteger<A> > >
				T operator[](A && a) const && {
				return { createExp<ArraySubscript>(NamedObjectBase::getExTmp(), EX(A,a)) };
			}

			//TODO why compilation fails without it ? should never be called
			static std::string typeStr(int trailing = 0) { return "dummyArrayStr"; }
		};

	}

} //namespace csl

#undef EX