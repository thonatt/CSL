#pragma once

#include "TemplateHelpers.hpp"
#include "Qualifiers.hpp"

#include <cstdint>
#include <tuple>
#include <utility>

namespace v2 {

	using uint = std::uint32_t;

	template<template<typename, typename> typename Pred, typename A, typename B>
	constexpr bool EqualLists = false;

	template<template<typename, typename> typename Pred, typename ...As, typename ...Bs>
	constexpr bool EqualLists<Pred, TList<As...>, TList<Bs...>> = (sizeof...(As) == sizeof...(Bs)) && (Pred<As, Bs>::Value && ... && true);

	template<typename List>
	struct GetArrayFromList;

	template<std::size_t ... Ns>
	struct GetArrayFromList<SizeList<Ns...>> {
		using Type = Array<Ns...>;
	};


	///////////////////////////////////

	template<typename T, std::size_t R, std::size_t C, typename ...Qs>
	class Matrix;

	template<typename T, std::size_t R, typename ...Qs>
	using Vector = Matrix<T, R, 1, Qs...>;

	template<typename T, typename ...Qs>
	using Scalar = Vector<T, 1, Qs...>;

	enum class SamplerAccessType : std::size_t { Sampler, Image };
	enum class SamplerType : std::size_t { Basic, Cube, Rectangle, MultiSample, Buffer, Atomic };
	enum class SamplerFlags : std::size_t {
		None = 0,
		Array = 1 << 1,
		Shadow = 1 << 2
	};
	constexpr SamplerFlags operator|(const SamplerFlags a, const SamplerFlags b) {
		return static_cast<SamplerFlags>(static_cast<std::size_t>(a) | static_cast<std::size_t>(b));
	}
	constexpr bool operator&(const SamplerFlags a, const SamplerFlags b) {
		return static_cast<bool>(static_cast<std::size_t>(a)& static_cast<std::size_t>(b));
	}

	template<SamplerAccessType Access, typename T, std::size_t N, SamplerType Type = SamplerType::Basic, SamplerFlags Flags = SamplerFlags::None, typename ...Qs>
	class Sampler;

	template<typename T, typename Ds, typename ... Qs>
	struct ArrayInterface;

	template<typename T, typename ...Qs>
	struct TypeInterface;

	enum class ObjFlags : std::size_t;

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
	struct ArrayInfos {
		using List = TList<Qs...>;

		template<typename T>
		struct ArrayPred {
			static bool constexpr Value = IsArray<T>::Value;
		};

		using Matches = Matching<ArrayPred, List>;
		static constexpr bool HasArray = (Matches::Values::Size > 0);
		static constexpr std::size_t Id = (HasArray ? Matches::Ids::Back : 0);
		using Dimensions = std::conditional_t<HasArray, typename IsArray<typename List::template GetType<Id>>::Dimensions, SizeList<>>;
		static constexpr bool Value = (Dimensions::Size > 0);
	};

	template<>
	struct ArrayInfos<> {
		using List = TList<>;
		static constexpr bool HasArray = false;
		static constexpr std::size_t Id = 0;
		using Dimensions = SizeList<>;
		static constexpr bool Value = false;
	};

	template<typename T>
	struct Infos {
		static constexpr bool IsValid = false;
		static constexpr bool IsArray = false;
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

	template<typename T, std::size_t R, std::size_t C, typename ...Qs>
	struct Infos<Matrix<T, R, C, Qs...>> {
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
		static constexpr bool IsInteger = std::is_same_v<T, int> || std::is_same_v<T, uint>;

		static constexpr bool IsValid = true;

		using ArrayDimensions = typename ArrayInfos<Qs...>::Dimensions;

		using Type = Matrix<T, R, C, Qs...>;
		using ScalarType = T;
		using QualifierFree = Matrix<T, R, C>;
	};

	template<typename T, typename Ds, typename ... Qs>
	struct Infos<ArrayInterface<T, Ds, Qs...>> : Infos<T> {
		using ArrayDimensions = Ds;
	};

	template<typename T, typename ...Qs>
	struct Infos<TypeInterface<T, Qs...>> : Infos<T> {
		using ArrayDimensions = typename ArrayInfos<Qs...>::Dimensions;
	};

	template<SamplerAccessType Access, typename T, std::size_t N, SamplerType sType, SamplerFlags sFlags, typename ...Qs>
	struct Infos<Sampler<Access, T, N, sType, sFlags, Qs...>> {
		using ScalarType = T;
		static constexpr std::size_t DimensionCount = N;
		static constexpr SamplerAccessType AccessType = Access;
		static constexpr SamplerType Type = sType;
		static constexpr SamplerFlags Flags = sFlags;

		using ArrayDimensions = SizeList<>;

		// mandatory for overload resolution
		static constexpr std::size_t RowCount = 0;
		static constexpr std::size_t ColCount = 0;
	};

	//template<typename T, typename Ds, std::size_t R, std::size_t C, typename ...Qs>
	//struct Infos<MatrixArray<T, Ds, R, C, Qs...>> : Infos<Matrix<T, R, C, Qs...>> {
	//	//TODO fix me
	//};

	//////////////////////////////////////////////////////////////

	//template<>
	//struct Infos<Expr> {
	//	static constexpr std::size_t NumElements = 0;
	//	using ScalarType = void;
	//};

	//template<>
	//struct Infos<ObjFlags> {
	//	static constexpr std::size_t NumElements = 0;
	//	using ScalarType = void;
	//};

	//template<std::size_t N>
	//struct Infos<const char(&)[N]> {
	//	static constexpr std::size_t NumElements = 0;
	//	using ScalarType = void;
	//	static constexpr std::size_t RowCount = 0;
	//	static constexpr std::size_t ColCount = 0;
	//};

	//template<>
	//struct Infos<std::string> {
	//	static constexpr std::size_t NumElements = 0;
	//	using ScalarType = void;
	//	static constexpr std::size_t RowCount = 0;
	//	static constexpr std::size_t ColCount = 0;
	//};

	////////////////////////////////////////////////////////////////////////

	template<typename ...Ts>
	constexpr std::size_t NumElements = (Infos<Ts>::NumElements + ...);

	template<typename ...Ts>
	constexpr bool SameSize = (Infos<Ts>::RowCount == ...) && (Infos<Ts>::ColCount == ...);

	template<typename A, typename B>
	constexpr bool HasMoreElementsThan = Infos<A>::NumElements >= Infos<B>::NumElements;

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
	constexpr bool IsFloat = SameMat<T, float>;

	template<>
	struct Infos<double> : Infos<Matrix<float, 1, 1>> {
		static constexpr bool IsConstant = true;
		static constexpr bool IsValid = true;
	};

	template<>
	struct Infos<float> : Infos<Matrix<float, 1, 1>> {
		static constexpr bool IsConstant = true;
		static constexpr bool IsValid = true;
	};

	template<>
	struct Infos<int> : Infos<Matrix<int, 1, 1, TList<>>> {
		static constexpr bool IsConstant = true;
		static constexpr bool IsValid = true;
	};


	template<>
	struct Infos<std::size_t> : Infos<Matrix<int, 1, 1, TList<>>> {
		static constexpr bool IsConstant = true;
		static constexpr bool IsValid = true;
	};

	template<>
	struct Infos<unsigned int> : Infos<Matrix<unsigned int, 1, 1, TList<>>> {
		static constexpr bool IsConstant = true;
		static constexpr bool IsValid = true;
	};

	template<>
	struct Infos<bool> : Infos<Matrix<bool, 1, 1, TList<>>> {
		static constexpr bool IsConstant = true;
		static constexpr bool IsValid = true;
	};

	//template<>
	//struct Infos<void> : Infos<Matrix<void, 0, 0>> {
	//};

	template<typename A, typename B>
	struct AlgebraMulInfos {
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
	struct AlgebraInfos {
		static_assert(SameScalarType<A, B>);
		using ReturnType = typename Infos<std::conditional_t<Infos<A>::IsScalar, B, A>>::QualifierFree;

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
	using RemoveArrayFromQualifiers = RemoveAt<typename Matching<IsArray, TList<Qs...>>::Ids, TList<Qs...>>;

	template<typename T, typename ... Qs>
	struct QualifiedIndirection;

	template<typename T, typename QList>
	struct TypeInterfaceIndirection;

	template<typename T, typename ...Qs>
	struct TypeInterfaceIndirection<T, TList<Qs...>> {
		using Type = TypeInterface<T, Qs...>;
	};

	template<typename T, std::size_t R, std::size_t C, typename ...Qs>
	struct TypeInterfaceIndirection<Matrix<T, R, C>, TList<Qs...>> {
		using Type = Matrix<T, R, C, Qs...>;
	};
	template<SamplerAccessType Access, typename T, std::size_t N, SamplerType sType, SamplerFlags Flags, typename ...Qs>
	struct TypeInterfaceIndirection<Sampler<Access, T, N, sType, Flags>, TList<Qs...>> {
		using Type = Sampler<Access, T, N, sType, Flags, Qs...>;
	};

	template<typename T, typename Ds, typename QList>
	struct ArrayInterfaceIndirection;

	template<typename T, typename Ds, typename ...Qs>
	struct ArrayInterfaceIndirection<T, Ds, TList<Qs...>> {
		using Type = ArrayInterface<T, Ds, Qs...>;
	};

	template<typename T, typename ... Qs>
	using Qualify = std::conditional_t<
		ArrayInfos<Qs...>::Value,
		typename ArrayInterfaceIndirection<T, typename ArrayInfos<Qs...>::Dimensions, RemoveArrayFromQualifiers<Qs...> >::Type,
		typename TypeInterfaceIndirection<T, RemoveArrayFromQualifiers<Qs...> >::Type
	>;


	//template<typename T, typename ... Qs>
	//using Qualify = typename QualifiedIndirection<typename Infos<T>::Type, Qs...>::Type;

}
