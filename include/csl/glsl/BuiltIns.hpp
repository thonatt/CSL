#pragma once

#include <csl/NamedObjects.hpp>
#include <csl/Preprocessor.hpp>
#include <csl/Types.hpp>

#include <csl/glsl/Types.hpp>

namespace csl
{

#define EXPR(type, var) get_expr(std::forward<type>(var))
#define ARG_TYPE(elem) typename CSL_PP_FIRST(elem)
#define ARG(elem) CSL_PP_FIRST(elem) && CSL_PP_SECOND(elem)
#define ARG_EXPR(elem) EXPR(CSL_PP_FIRST(elem), CSL_PP_SECOND(elem))

#define CSL_MAKE_OP_1(condition, return_type, name, var) \
	template<ARG_TYPE(var), std::enable_if_t<condition, int> = 0> \
	return_type name (ARG(var)) { \
		return { make_funcall( Op :: name , ARG_EXPR(var)) };  \
	}

#define CSL_MAKE_OP_2(condition, return_type, name, varA, varB) \
	template<ARG_TYPE(varA), ARG_TYPE(varB), std::enable_if_t<condition, int> = 0> \
	return_type name (ARG(varA), ARG(varB)) { \
		return { make_funcall( Op :: name, ARG_EXPR(varA), ARG_EXPR(varB)) };  \
	}

#define CSL_MAKE_OP_3(condition, return_type, name, varA, varB, varC) \
	template<ARG_TYPE(varA), ARG_TYPE(varB), ARG_TYPE(varC), std::enable_if_t<condition, int> = 0> \
	return_type name (ARG(varA), ARG(varB), ARG(varC)) { \
		return { make_funcall( Op :: name , ARG_EXPR(varA), ARG_EXPR(varB), ARG_EXPR(varC)) };  \
	}

#define GENXTYPE_OP_GENXTYPE(type_in, var_in, type_out, name) \
	CSL_MAKE_OP_1(IsVec<A CSL_PP_COMMA type_in>, Vector<type_out CSL_PP_COMMA Infos<A>::RowCount>, name, (A, var_in))

#define FLOAT_OP_GENTYPE(name) \
	CSL_MAKE_OP_1(IsVecF<A>, Vector<float CSL_PP_COMMA 1>, name, (A, a))

#define GENXTYPE_OP_GENXTYPE_2(type_in, var_in_1, var_in_2, type_out, name) \
	CSL_MAKE_OP_2(IsVec<A CSL_PP_COMMA type_in>, Vector<type_out CSL_PP_COMMA Infos<A>::RowCount>, name, (A, var_in))

#define GENTYPE_OP_SCALAR_OR_GENTYPE_2(name) \
	CSL_MAKE_OP_2(IsVecF<A> && (SameMat<A CSL_PP_COMMA B> || IsFloat<B>), Vector<float CSL_PP_COMMA Infos<A>::RowCount>, name, (A, a), (B, b))

#define RELATIONAL_GENTYPE_OP(name) \
	CSL_MAKE_OP_2(IsVecF<A> && SameMat<A CSL_PP_COMMA B>, Vector<bool CSL_PP_COMMA Infos<A>::RowCount>, name, (A, a), (B, b))

#define FLOAT_OP_GENTYPE_2(name) \
	CSL_MAKE_OP_2(IsVecF<A> && SameMat<A CSL_PP_COMMA B>, Vector<float CSL_PP_COMMA 1>, name, (A, a), (B, b))

#define GENTYPE_OP_GENTYPE(name) GENXTYPE_OP_GENXTYPE(float, v, float, name)
#define GENTYPE_OP_GENTYPE_IT(data, i, elem) GENTYPE_OP_GENTYPE(elem) 
#define RELATIONAL_GENTYPE_OP_IT(data, i, elem) RELATIONAL_GENTYPE_OP(elem)
#define FLOAT_OP_GENTYPE_2_IT(data, i, elem) FLOAT_OP_GENTYPE_2(elem)
#define GENTYPE_OP_SCALAR_OR_GENTYPE_2_IT(data, i, elem) GENTYPE_OP_SCALAR_OR_GENTYPE_2(elem)

	namespace glsl_110 {

		using namespace glsl;

		CSL_PP_ITERATE(GENTYPE_OP_GENTYPE_IT,
			dFdx,
			dFdy,
			abs,
			sin,
			cos,
			tan,
			exp,
			log,
			sqrt,
			floor,
			ceil,
			fract,
			exp2,
			log2,
			normalize,
			atan,
			acos,
			asin,
			radians,
			degrees
		);

		CSL_PP_ITERATE(RELATIONAL_GENTYPE_OP_IT,
			greaterThan,
			lessThan,
			greaterThanEqual,
			lessThenEqual,
			equal,
			notEqual
		);

		FLOAT_OP_GENTYPE(length);

		CSL_MAKE_OP_3(
			IsVecF<A>&& SameMat<A CSL_PP_COMMA B> && (SameMat<A CSL_PP_COMMA C> || IsFloat<C>),
			Vector<float CSL_PP_COMMA Infos<A>::RowCount>,
			mix,
			(A, a), (B, b), (C, c)
		);

		CSL_MAKE_OP_3(
			IsVecF<C>&& SameMat<A CSL_PP_COMMA B> && (SameMat<A CSL_PP_COMMA C> || IsFloat<A>),
			Vector<float CSL_PP_COMMA Infos<C>::RowCount>,
			smoothstep,
			(A, edge0), (B, edge1), (C, x)
		);

		CSL_MAKE_OP_3(
			IsVecF<A>&& IsVecF<B>&& SameMat<B CSL_PP_COMMA C> && (SameSize<A CSL_PP_COMMA B> || IsFloat<B>),
			Vector<float CSL_PP_COMMA Infos<A>::RowCount>,
			clamp,
			(A, x), (B, minVal), (C, maxVal)
		);

		CSL_MAKE_OP_2(
			IsVecF<A>&& IsVecF<B> && (SameMat<A CSL_PP_COMMA B> || IsFloat<A> || IsFloat<B>),
			Vector<float CSL_PP_COMMA std::max(Infos<A>::RowCount CSL_PP_COMMA Infos<B>::RowCount) >,
			pow,
			(A, a), (B, b)
		);

		CSL_MAKE_OP_2(
			IsVecF<I>&& SameMat<I CSL_PP_COMMA N>,
			Vector<float CSL_PP_COMMA Infos<I>::RowCount>,
			reflect,
			(I, i), (N, n)
		);

		CSL_MAKE_OP_2(
			IsVecF<A >&& Infos<A>::RowCount == 3 && SameMat<A CSL_PP_COMMA B>,
			Vector<float CSL_PP_COMMA 3>,
			cross,
			(A, a), (B, b)
		);

		CSL_PP_ITERATE(FLOAT_OP_GENTYPE_2_IT,
			distance,
			dot
		);

		CSL_PP_ITERATE(GENTYPE_OP_SCALAR_OR_GENTYPE_2_IT,
			max,
			min,
			mod,
			atan
		);
	}

	namespace glsl_120 {
		using namespace glsl_110;

		CSL_MAKE_OP_1
			(Infos<A>::IsFloat && (Infos<A>::RowCount > 0) && (Infos<A>::ColCount > 0), 
			Matrix<float CSL_PP_COMMA Infos<A>::ColCount CSL_PP_COMMA Infos<A>::RowCount>,
			transpose,
			(A, a)
		);
	}

	namespace glsl_130 {
		using namespace glsl_120;

		CSL_MAKE_OP_2(
			(bool(Infos<S>::Flags& SamplerFlags::Sampler) &&
			bool(Infos<S>::Flags& SamplerFlags::Basic) &&
			!bool(Infos<S>::Flags& SamplerFlags::Shadow) &&
			IsVecF<P>&&
			Infos<P>::RowCount == (Infos<S>::DimensionCount + (bool(Infos<S>::Flags & SamplerFlags::Array) ? 1 : 0))),
			Vector<typename Infos<S>::ScalarType CSL_PP_COMMA 4>,
			texture,
			(S, sampler), (P, point)
		);

		CSL_MAKE_OP_3(
			(bool(Infos<S>::Flags& SamplerFlags::Sampler) &&
			bool(Infos<S>::Flags& SamplerFlags::Basic) &&
			!bool(Infos<S>::Flags& SamplerFlags::Shadow) &&
			IsVecF<P>&& IsFloat<B>&&
			Infos<P>::RowCount == (Infos<S>::DimensionCount + (bool(Infos<S>::Flags & SamplerFlags::Array) ? 1 : 0))),
			Vector<typename Infos<S>::ScalarType CSL_PP_COMMA 4>,
			texture,
			(S, sampler), (P, point), (B, biais)
		);

		GENXTYPE_OP_GENXTYPE(float, v, float, round);
		GENXTYPE_OP_GENXTYPE(float, v, float, sign);
		GENXTYPE_OP_GENXTYPE(int, v, int, abs);
	}

	namespace glsl_140 {
		using namespace glsl_130;

		CSL_MAKE_OP_1(Infos<A>::IsSquare&& Infos<A>::IsFloat,
			Matrix<float CSL_PP_COMMA Infos<A>::RowCount CSL_PP_COMMA  Infos<A>::RowCount>,
			inverse,
			(A, a));
	}

	namespace glsl_150 {
		using namespace glsl_140;
	}

	namespace glsl_330 {
		using namespace glsl_150;
	}

	namespace glsl_400 {
		using namespace glsl_330;

		CSL_MAKE_OP_3(Infos<A>::IsVec&& SameMat<B CSL_PP_COMMA Int>&& SameMat<C CSL_PP_COMMA Int>,
			Vector<typename Infos<A>::ScalarType CSL_PP_COMMA Infos<A>::RowCount>,
			bitfieldExtract,
			(A, value), (B, offset), (C, bits)
		);

		CSL_MAKE_OP_3(
			Infos<A>::IsVec&& Infos<B>::IsInteger&& SameMat<B CSL_PP_COMMA C> && (SameSize<A CSL_PP_COMMA B> || Infos<B>::IsScalar),
			Vector<typename Infos<A>::ScalarType CSL_PP_COMMA Infos<A>::RowCount>,
			clamp,
			(A, x), (B, minVal), (C, maxVal)
		);
	}

	namespace glsl_410 {
		using namespace glsl_400;

	}

	namespace glsl_420 {
		using namespace glsl_410;

		template<typename I, typename P, typename D, std::enable_if_t<
			bool(Infos<I>::Flags& SamplerFlags::Image) &&
			bool(Infos<I>::Flags& SamplerFlags::Basic) &&
			!bool(Infos<I>::Flags& SamplerFlags::Shadow) &&
			IsVecI<P> && IsVecF<D>&& Infos<D>::RowCount == 4 &&
			Infos<P>::RowCount == (Infos<I>::DimensionCount + (bool(Infos<I>::Flags & SamplerFlags::Array) ? 1 : 0)), int> = 0>
		void imageStore(I&& image, P&& p, D&& data)
		{
			context::get().push_expression(make_funcall(Op::imageStore, EXPR(I, image), EXPR(P, p), EXPR(D, data)));
		}
	}

	namespace glsl_430 {
		using namespace glsl_420;

		CSL_MAKE_OP_1(
			bool(Infos<I>::Flags & SamplerFlags::Image),
			Vector<int CSL_PP_COMMA Infos<I>::DimensionCount>,
			imageSize,
			(I, image)
		);
	}

}