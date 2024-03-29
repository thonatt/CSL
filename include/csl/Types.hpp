#pragma once

#include "TemplateHelpers.hpp"
#include "Qualifiers.hpp"

#include <cstdint>
#include <type_traits>
#include <utility>

namespace csl
{
	template<typename T, std::size_t R, std::size_t C>
	class Matrix;

	template<typename T, std::size_t R>
	using Vector = Matrix<T, R, 1>;

	template<typename T>
	using Scalar = Vector<T, 1>;

	enum class SamplerFlags : std::uint32_t
	{
		Sampler = 1 << 0,
		Image = 1 << 1,

		Basic = 1 << 2,
		Cube = 1 << 3,
		Rectangle = 1 << 4,
		Buffer = 1 << 5,
		Atomic = 1 << 6,

		Multisample = 1 << 7,
		Array = 1 << 8,
		Shadow = 1 << 9,

		MaskAccess = Sampler | Image,
		MaskType = Basic | Cube | Rectangle | Buffer | Atomic,
		MaskAttribute = Multisample | Array | Shadow,

		Default = Sampler | Basic,
	};
	constexpr SamplerFlags operator|(const SamplerFlags a, const SamplerFlags b) {
		return static_cast<SamplerFlags>(static_cast<std::underlying_type_t<SamplerFlags>>(a) | static_cast<std::underlying_type_t<SamplerFlags>>(b));
	}
	constexpr SamplerFlags operator&(const SamplerFlags a, const SamplerFlags b) {
		return static_cast<SamplerFlags>(static_cast<std::underlying_type_t<SamplerFlags>>(a) & static_cast<std::underlying_type_t<SamplerFlags>>(b));
	}

	template<typename T, std::size_t N, SamplerFlags Flags = SamplerFlags::Default>
	class Sampler;

	struct NoInit {};

	///

	template<typename T, typename Ds, typename ... Qs>
	struct ArrayInterface;

	template<typename T, typename ...Qs>
	struct TypeInterface;

	template<typename List>
	struct GetArrayFromList;

	template<std::size_t ... Ns>
	struct GetArrayFromList<SizeList<Ns...>> {
		using Type = Array<Ns...>;
	};

	template<typename T>
	struct IsArray {
		static constexpr bool Value = false;
		using Dimensions = SizeList<>;
	};

	template<std::size_t ...Ns>
	struct IsArray<Array<Ns...>> {
		static constexpr bool Value = true;
		using Dimensions = SizeList<Ns...>;
	};

	template<typename ...Qs>
	struct ArrayInfos
	{
		using List = TList<Qs...>;

		template<typename T>
		struct ArrayPred {
			static bool constexpr Value = IsArray<T>::Value;
		};

		using Matches = Matching<ArrayPred, List>;
		static constexpr bool HasArray = (Matches::Values::Size > 0);
		static constexpr std::size_t Index = (HasArray ? Matches::Indexes::Back : 0);
		using Dimensions = std::conditional_t<HasArray, typename IsArray<typename List::template At<Index>>::Dimensions, SizeList<>>;
		static constexpr bool Value = (Dimensions::Size > 0);
	};

	template<>
	struct ArrayInfos<> {
		using List = TList<>;
		static constexpr bool HasArray = false;
		using Dimensions = SizeList<>;
		static constexpr bool Value = false;
	};

	template<typename T, typename Enable = void>
	struct Infos {
		static constexpr bool IsCSLType = false;
		static constexpr bool IsArray = false;
		static constexpr bool IsInteger = false;
		static constexpr std::size_t RowCount = 0;
		static constexpr std::size_t ColCount = 0;
		static constexpr std::size_t NumElements = 0;
		using ScalarType = void;
		using ArrayDimensions = SizeList<>;
	};

	template<typename T>
	struct Infos<const T> : Infos<T> {
		static constexpr bool IsConst = true;
	};

	template<typename T>
	struct Infos<T&> : Infos<T> { };

	template<typename T>
	struct Infos<const T&> : Infos<T> { };

	template<typename T, std::size_t R, std::size_t C>
	struct Infos<Matrix<T, R, C>>
	{
		static constexpr bool IsConst = false;
		static constexpr bool IsConstant = false;
		static constexpr bool IsScalar = (R == 1 && C == 1);
		static constexpr bool IsVec = (C == 1);
		static constexpr bool IsSquare = (R == C);

		static constexpr std::size_t NumElements = R * C;
		static constexpr std::size_t RowCount = R;
		static constexpr std::size_t ColCount = C;

		static constexpr bool IsBool = std::is_same_v<T, bool>;
		static constexpr bool IsFloat = std::is_same_v<T, float>;
		static constexpr bool IsFloating = std::is_same_v<T, float> || std::is_same_v<T, double>;
		static constexpr bool IsInteger = std::is_same_v<T, int> || std::is_same_v<T, unsigned int>;

		static constexpr bool IsCSLType = true;

		using ArrayDimensions = SizeList<>;

		using Type = Matrix<T, R, C>;
		using ScalarType = T;
	};

	template<typename T, typename Ds, typename ... Qs>
	struct Infos<ArrayInterface<T, Ds, Qs...>> : Infos<T> {
		using ArrayDimensions = Ds;
	};

	template<typename T, typename ...Qs>
	struct Infos<TypeInterface<T, Qs...>> : Infos<T> {
		using ArrayDimensions = typename ArrayInfos<Qs...>::Dimensions;
	};

	template<typename T, std::size_t N, SamplerFlags sFlags>
	struct Infos<Sampler<T, N, sFlags>> {
		using ScalarType = T;
		static constexpr std::size_t DimensionCount = N;
		static constexpr SamplerFlags Flags = sFlags;

		using ArrayDimensions = SizeList<>;

		// mandatory for overload resolution
		static constexpr std::size_t RowCount = 0;
		static constexpr std::size_t ColCount = 0;
	};

	namespace detail
	{
		template<typename>
		struct sfinae_true : std::true_type {};

		template<typename T>
		static auto is_csl_type(int) -> sfinae_true<decltype(T::IsCSLType())>;

		template<typename>
		static auto is_csl_type(long) -> std::false_type;

		template<typename T>
		constexpr bool IsCSLType()
		{
			if constexpr (decltype(is_csl_type<T>(0)){})
				return true;
			else
				return Infos<T>::IsCSLType;
		};
	}

	template<typename ...Ts>
	struct TypeInfos
	{
		using List = TList<Ts...>;

		template<typename T>
		struct TypePred {
			static bool constexpr Value = detail::IsCSLType<T>();
		};

		using Matches = Matching<TypePred, List>;
		using Values = typename Matches::Values;
		static_assert(Values::Size == 1);

		using T = typename Values::Front;
		using TIndex = typename Matches::Indexes;
	};

	template<>
	struct TypeInfos<>
	{
		using T = void;
		using TIndex = SizeList<0>;
	};

	////////////////////////////////////////////////////////////////////////

	template<typename ...Ts>
	constexpr std::size_t NumElements = (Infos<Ts>::NumElements + ...);

	template<typename ...Ts>
	constexpr bool SameSize = (Infos<Ts>::RowCount == ...) && (Infos<Ts>::ColCount == ...);

	template<typename ...Ts>
	constexpr bool SameScalarType = true;

	template<typename T, typename U, typename ...Ts>
	constexpr bool SameScalarType<T, U, Ts...> = std::is_same_v<typename Infos<T>::ScalarType, typename Infos<U>::ScalarType> && SameScalarType<U, Ts...>;

	template<typename A, typename B>
	constexpr bool SameDimensions = std::is_same_v<typename Infos<A>::ArrayDimensions, typename Infos<B>::ArrayDimensions>;

	template<typename A, typename B>
	constexpr bool SameType = std::is_same_v<A, B> || (SameSize<A, B> && SameScalarType<A, B> && SameDimensions<A, B>);

	template<typename A, typename B>
	constexpr bool SameMat = SameType<A, B> && SameSize<A, B>;

	template<typename T>
	constexpr bool IsInteger = Infos<T>::IsInteger;

	template<typename T, typename Scalar>
	constexpr bool IsVec = Infos<T>::IsVec && std::is_same_v<typename Infos<T>::ScalarType, Scalar>;

	template<typename T>
	constexpr bool IsVecF = IsVec<T, float>;

	template<typename T>
	constexpr bool IsVecI = IsVec<T, int>;

	template<typename T>
	constexpr bool IsFloat = SameMat<T, float>;

	template<>
	struct Infos<bool> : Infos<Matrix<bool, 1, 1>> {
		static constexpr bool IsConstant = true;
		static constexpr bool IsCSLType = true;
	};

	template<>
	struct Infos<float> : Infos<Matrix<float, 1, 1>> {
		static constexpr bool IsConstant = true;
		static constexpr bool IsCSLType = true;
	};

	template<>
	struct Infos<double> : Infos<float> { };

	template<>
	struct Infos<int> : Infos<Matrix<int, 1, 1>> {
		static constexpr bool IsConstant = true;
		static constexpr bool IsCSLType = true;
	};

	template<>
	struct Infos<unsigned int> : Infos<Matrix<unsigned int, 1, 1>> {
		static constexpr bool IsConstant = true;
		static constexpr bool IsCSLType = true;
	};

	template<>
	struct Infos<unsigned long> : Infos<unsigned int> { };

	template<>
	struct Infos<unsigned long long> : Infos<unsigned int> { };

	template<typename A, typename B>
	struct AlgebraMulInfos
	{
		static_assert(SameScalarType<A, B>);

		using ScalarType = typename Infos<A>::ScalarType;
		static constexpr std::size_t OutRowCount = Infos<std::conditional_t<Infos<A>::IsScalar, B, A>>::RowCount;
		static constexpr std::size_t OutColCount = Infos<std::conditional_t<Infos<B>::IsScalar, A, B>>::ColCount;
		using ReturnType = Matrix<ScalarType, OutRowCount, OutColCount>;

		static constexpr Op Operator =
			(SameSize<A, B> && Infos<A>::IsVec) ? Op::CWiseMul :
			(Infos<A>::IsScalar) ? Op::ScalarTimesMatrix :
			(Infos<B>::IsScalar) ? Op::MatrixTimesScalar : Op::MatrixTimesMatrix;
	};

	template<typename A, typename B>
	struct AlgebraInfos
	{
		static_assert(SameScalarType<A, B>);
		using ReturnType = typename Infos<std::conditional_t<Infos<A>::IsScalar, B, A>>::Type;

		static constexpr Op OperatorAdd =
			SameSize<A, B> ? Op::CWiseAdd :
			(Infos<A>::IsScalar) ? Op::ScalarAddMatrix : Op::MatrixAddScalar;

		static constexpr Op OperatorSub =
			SameSize<A, B> ? Op::CWiseSub :
			(Infos<A>::IsScalar) ? Op::ScalarSubMatrix : Op::MatrixSubScalar;

		static constexpr Op OperatorDiv =
			SameSize<A, B> ? Op::CWiseDiv :
			(Infos<A>::IsScalar) ? Op::ScalarDivMatrix : Op::MatrixDivScalar;
	};

	template<typename ...Qs>
	using RemoveArrayFromQualifiers = RemoveAt<typename Matching<IsArray, TList<Qs...>>::Indexes, TList<Qs...>>;

	template<typename T, typename ... Qs>
	struct QualifiedIndirection;

	template<typename T, typename QList>
	struct TypeInterfaceIndirection;

	template<typename T, typename ...Qs>
	struct TypeInterfaceIndirection<T, TList<Qs...>> {
		using Type = TypeInterface<T, Qs...>;
	};

	template<typename T, typename Ds, typename QList>
	struct ArrayInterfaceIndirection;

	template<typename T, typename Ds, typename ...Qs>
	struct ArrayInterfaceIndirection<T, Ds, TList<Qs...>> {
		using Type = ArrayInterface<T, Ds, Qs...>;
	};

	template<typename T, typename Qs>
	struct QualifyIndirectionDetail;

	template<typename T, typename ... Qs>
	struct QualifyIndirectionDetail<T, TList<Qs...>>
	{
		using Type = std::conditional_t<
			ArrayInfos<Qs...>::Value,
			typename ArrayInterfaceIndirection<T, typename ArrayInfos<Qs...>::Dimensions, RemoveArrayFromQualifiers<Qs...> >::Type,
			typename TypeInterfaceIndirection<T, RemoveArrayFromQualifiers<Qs...> >::Type
		>;
	};

	template<typename ...Ts>
	struct QualifyIndirection
	{
		using TInfos = TypeInfos<Ts...>;
		using T = typename TInfos::T;
		using QualifiersList = RemoveAt<typename TInfos::TIndex, TList<Ts...>>;
		using Type = typename QualifyIndirectionDetail<T, QualifiersList>::Type;
	};

	template<typename ...Ts>
	using Qualify = typename QualifyIndirection<Ts...>::Type;
}
