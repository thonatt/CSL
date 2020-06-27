#pragma once

#include "../Preprocessor.hpp"

#include "../Types.hpp"

#include "../NamedObjects.hpp"

namespace v2 {

#define EX(type, var) get_expr(std::forward<type>(var))

//#define ARG_TYPE_IT(r, data, i, elem)  CSL_PP2_COMMA_IF(i) typename FIRST(elem)
//#define ARG_IT(r, data, i, elem) CSL_PP2_COMMA_IF(i) FIRST(elem) && SECOND(elem)
//#define ARG_EXPR_IT(r, data, i, elem) , get_expr(std::forward<FIRST(elem) >(SECOND(elem) ))

//#define CSL_MAKE_OP(condition, return_type, name, ...) \
//	template<CSL_PP2_ITERATE(ARG_TYPE_IT, __VA_ARGS__), typename = std::enable_if_t<condition>> \
//	return_type name (CSL_PP2_ITERATE(ARG_IT, __VA_ARGS__)) { \
//		return { make_funcall(CSL_PP2_CONCAT(Op::,name) CSL_PP2_ITERATE(ARG_EXPR_IT, __VA_ARGS__)) };  \
//	}

#define ARG_TYPE(elem) typename FIRST(elem)
#define ARG(elem) FIRST(elem) && SECOND(elem)
#define ARG_EXPR(elem) get_expr(std::forward<FIRST(elem) >(SECOND(elem)))

#define CSL_MAKE_OP_1(condition, return_type, name, var) \
	template<ARG_TYPE(var), typename = std::enable_if_t<condition>> \
	return_type name (ARG(var)) { \
		return { make_funcall(CSL_PP2_CONCAT(Op::,name), ARG_EXPR(var)) };  \
	}

#define CSL_MAKE_OP_2(condition, return_type, name, varA, varB) \
	template<ARG_TYPE(varA), ARG_TYPE(varB), typename = std::enable_if_t<condition>> \
	return_type name (ARG(varA), ARG(varB)) { \
		return { make_funcall(CSL_PP2_CONCAT(Op::,name), ARG_EXPR(varA), ARG_EXPR(varB)) };  \
	}

#define CSL_MAKE_OP_3(condition, return_type, name, varA, varB, varC) \
	template<ARG_TYPE(varA), ARG_TYPE(varB), ARG_TYPE(varC), typename = std::enable_if_t<condition>> \
	return_type name (ARG(varA), ARG(varB), ARG(varC)) { \
		return { make_funcall(CSL_PP2_CONCAT(Op::,name), ARG_EXPR(varA), ARG_EXPR(varB), ARG_EXPR(varC)) };  \
	}

#define GENXTYPE_OP_GENXTYPE(type_in, var_in, type_out, name) \
	CSL_MAKE_OP_1(IsVec<A CSL_PP2_COMMA type_in>, Vector<type_out CSL_PP2_COMMA Infos<A>::RowCount>, name, (A, var_in))

#define FLOAT_OP_GENTYPE(name) \
	CSL_MAKE_OP_1(IsVecF<A>, Vector<float CSL_PP2_COMMA 1>, name, (A, a))

#define GENXTYPE_OP_GENXTYPE_2(type_in, var_in_1, var_in_2, type_out, name) \
	CSL_MAKE_OP_2(IsVec<A CSL_PP2_COMMA type_in>, Vector<type_out CSL_PP2_COMMA Infos<A>::RowCount>, name, (A, var_in))

#define GENTYPE_OP_SCALAR_OR_GENTYPE_2(name) \
	CSL_MAKE_OP_2(IsVecF<A> && (SameMat<A CSL_PP2_COMMA B> || IsFloat<B>), Vector<float CSL_PP2_COMMA Infos<A>::RowCount>, name, (A, a), (B, b))

#define RELATIONAL_GENTYPE_OP(name) \
	CSL_MAKE_OP_2(IsVecF<A> && SameMat<A CSL_PP2_COMMA B>, Vector<bool CSL_PP2_COMMA Infos<A>::RowCount>, name, (A, a), (B, b))

#define FLOAT_OP_GENTYPE_2(name) \
	CSL_MAKE_OP_2(IsVecF<A> && SameMat<A CSL_PP2_COMMA B>, Vector<float CSL_PP2_COMMA 1>, name, (A, a), (B, b))

#define GENTYPE_OP_GENTYPE(name) GENXTYPE_OP_GENXTYPE(float, v, float, name)
#define GENTYPE_OP_GENTYPE_IT(r, data, i, elem) GENTYPE_OP_GENTYPE(elem) 
#define RELATIONAL_GENTYPE_OP_IT(r, data, i, elem) RELATIONAL_GENTYPE_OP(elem)
#define FLOAT_OP_GENTYPE_2_IT(r, data, i, elem) FLOAT_OP_GENTYPE_2(elem)
#define GENTYPE_OP_SCALAR_OR_GENTYPE_2_IT(r, data, i, elem) GENTYPE_OP_SCALAR_OR_GENTYPE_2(elem)

	namespace glsl_110 {

		using namespace glsl;

		CSL_PP2_ITERATE(GENTYPE_OP_GENTYPE_IT,
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

		CSL_PP2_ITERATE(RELATIONAL_GENTYPE_OP_IT,
			greaterThan,
			lessThan,
			greaterThanEqual,
			lessThenEqual,
			equal,
			notEqual
		);

		FLOAT_OP_GENTYPE(length);

		CSL_MAKE_OP_3(
			IsVecF<A>&& SameMat<A CSL_PP2_COMMA B> && (SameMat<A CSL_PP2_COMMA C> || IsFloat<C>),
			Vector<float CSL_PP2_COMMA Infos<A>::RowCount>,
			mix,
			(A, a), (B, b), (C, c)
		);

		CSL_MAKE_OP_3(
			IsVecF<C>&& SameMat<A CSL_PP2_COMMA B> && (SameMat<A CSL_PP2_COMMA C> || IsFloat<A>),
			Vector<float CSL_PP2_COMMA Infos<C>::RowCount>,
			smoothstep,
			(A, edge0), (B, edge1), (C, x)
		);

		CSL_MAKE_OP_3(
			IsVecF<A>&& SameMat<B CSL_PP2_COMMA C> && (SameMat<A CSL_PP2_COMMA B> || IsFloat<B>),
			Vector<float CSL_PP2_COMMA Infos<A>::RowCount>,
			clamp,
			(A, x), (B, minVal), (C, maxVal)
		);

		CSL_PP2_ITERATE(FLOAT_OP_GENTYPE_2_IT,
			distance,
			dot
		);

		CSL_PP2_ITERATE(GENTYPE_OP_SCALAR_OR_GENTYPE_2_IT,
			max,
			min,
			mod,
			atan
		);
	}

	namespace glsl_120 {
		using namespace glsl_110;

	}

	namespace glsl_130 {
		using namespace glsl_120;

		CSL_MAKE_OP_2(
			(Infos<S>::AccessType == SamplerAccessType::Sampler) &&
			(Infos<S>::Type == SamplerType::Basic) &&
			!(Infos<S>::Flags & SamplerFlags::Shadow) &&
			IsVecF<P>&&
			Infos<P>::RowCount == (Infos<S>::DimensionCount + ((Infos<S>::Flags & SamplerFlags::Array) ? 1 : 0)),
			Vector<typename Infos<S>::ScalarType CSL_PP2_COMMA 4>,
			texture,
			(S, sampler), (P, point)
		);

		CSL_MAKE_OP_3(
			(Infos<S>::AccessType == SamplerAccessType::Sampler) &&
			(Infos<S>::Type & SamplerType::Basic) &&
			!(Infos<S>::Flags& SamplerFlags::Shadow) &&
			IsVecF<P> && IsFloat<B> &&
			Infos<P>::RowCount == (Infos<S>::DimensionCount + ((Infos<S>::Flags & SamplerFlags::Array) ? 1 : 0)),
			Vector<typename Infos<S>::ScalarType CSL_PP2_COMMA 4>,
			texture,
			(S, sampler), (P, point), (B, biais)
		);

	}

	namespace glsl_140 {
		using namespace glsl_130;

	}

	namespace glsl_150 {
		using namespace glsl_140;

	}

	namespace glsl_330 {
		using namespace glsl_150;

	}

	namespace glsl_400 {
		using namespace glsl_330;

	}

	namespace glsl_410 {
		using namespace glsl_400;

	}

	namespace glsl_420 {
		using namespace glsl_410;

	}

#undef EX

}