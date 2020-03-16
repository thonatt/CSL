#pragma once

#include "Matrix.hpp"
#include "Preprocessor.hpp"

#define EX(type, var) getExp(std::forward<type>(var))

#define TYPE_OP_TYPE(type_in, var_str, type_out, name) \
	template<typename A, typename = std::enable_if_t< EqualMat<A, type_in> > > \
	type_out name(A&& var_str) { \
		return { createFCallExp(CSL_PP_STR(name), EX(A,var_str)) };	\
	}

#define ANYTYPE_OP_ANYTYPE(type_in, type_out, name) \
	template<typename A, typename = std::enable_if_t< IsVec<A, type_in> > > \
	Vec<type_out, Infos<A>::rows> name(A&& a) { \
		return { createFCallExp(CSL_PP_STR(name), EX(A,a)) };	\
	}

#define BVEC_OP_GENTYPE(name) ANYTYPE_OP_ANYTYPE(FLOAT, BOOL, name)

#define GENTYPE_OP_GENTYPE(r, data, i, elem) ANYTYPE_OP_ANYTYPE(FLOAT, FLOAT, elem)

#define RELATIONAL_GENTYPE_OP(r, data, i, elem) \
	template<typename A, typename B, typename = std::enable_if_t< IsVecF<A> && EqualMat<A,B> > > \
	Vec<BOOL, Infos<A>::rows> elem(A && a, B && b) { \
		return { createFCallExp(CSL_PP_STR(elem), EX(A,a), EX(B,b) ) };	\
	} 

#define BOOL_OP(name) \
	template<typename A, typename = std::enable_if_t< IsVecB<A> > > \
	Bool name(A && a) { \
		return { createFCallExp(#name, EX(A,a)) };	\
	}

namespace csl {

	namespace core {

		namespace glsl_110 {

			template<typename A, typename = std::enable_if_t< IsVecF<A> > >
			Vec<FLOAT, Infos<A>::rows> dFdx(A&& a) {
				return { createFCallExp("dFdx", EX(A,a)) };
			}

			template<typename A, typename = std::enable_if_t< IsVecF<A> > >
			Vec<FLOAT, Infos<A>::rows> dFdy(A&& a) {
				return { createFCallExp("dFdy", EX(A,a)) };
			}

			template<typename A, typename = std::enable_if_t< IsVecF<A> > >
			Float length(A && a) {
				return { createFCallExp("length", EX(A,a)) };
			}

			template<typename A, typename B, typename = std::enable_if_t< IsVecF<A> && EqualMat<A, B> > >
			Float distance(A && a, B && b) {
				return { createFCallExp("distance", EX(A,a), EX(B,b)) };
			}

			template<typename A, typename B, typename = std::enable_if_t<
				NoBools<A, B> && IsConvertibleTo<A, Vec<FLOAT, Infos<B>::rows>> && IsConvertibleTo<B, Vec<FLOAT, Infos<A>::rows>>
			> >
				Vec<FLOAT, Infos<A>::rows> pow(A && a, B && b) {
				return { createFCallExp("pow", EX(A,a), EX(B,b)) };
			}

			template<typename A, typename B, typename = std::enable_if_t<
				IsVecF<A> && EqualMat<A, B>
			> >
				Float dot(A && a, B && b) {
				return { createFCallExp("dot", EX(A,a), EX(B,b)) };
			}

			template<typename I, typename N, typename = std::enable_if_t<
				IsVecF<I> && EqualMat<I, N>
			> >
				Vec<FLOAT, Infos<I>::rows> reflect(I && i, N && n) {
				return { createFCallExp("reflect", EX(I,i), EX(N,n)) };
			}

			template<typename A, typename B, typename C, typename = std::enable_if_t<
				IsVecF<A> && EqualMat<A, B> && EqualMat<A, C>
			> >
				Vec<FLOAT, Infos<A>::rows> faceforward(A&& n, B&& i, C&& nRef) {
				return { createFCallExp("faceforward", EX(A,n), EX(B,i), EX(C,nRef)) };
			}
			
			template<typename A, typename B, typename C, typename = std::enable_if_t<
				IsVecF<A> && EqualMat<A, B> && IsFloat<C>
			> >
				Vec<FLOAT, Infos<A>::rows> refract(A&& i, B&& n, C&& eta) {
				return { createFCallExp("refract", EX(A,i), EX(B,n), EX(C,eta)) };
			}

			template<typename A, typename B, typename = std::enable_if_t<
				IsVecF<A> && (EqualMat<A, B> || IsFloat<B>)
			> >
				Vec<FLOAT, Infos<A>::rows> max(A && a, B && b) {
				return { createFCallExp("max", EX(A,a), EX(B,b)) };
			}

			template<typename A, typename B, typename = std::enable_if_t<
				IsVecF<A> && (EqualMat<A, B> || IsFloat<B>)
			> >
				Vec<FLOAT, Infos<A>::rows> min(A && a, B && b) {
				return { createFCallExp("min", EX(A,a), EX(B,b)) };
			}

			template<typename A, typename B, typename = std::enable_if_t<
				IsVecF<A> && (EqualMat<A, B> || IsFloat<B>)
			> >
				Vec<FLOAT, Infos<A>::rows> mod(A && a, B && b) {
				return { createFCallExp("mod", EX(A,a), EX(B,b)) };
			}

			template<typename A, typename B, typename = std::enable_if_t<
				IsVecF<A> && EqualMat<A,B> && FunctionOut<B>
			>>
				Vec<FLOAT, Infos<A>::rows> modf(A&& x, B&& i) {
				return { createFCallExp("modf", EX(A,x), EX(B,i)) };
			}

			template<typename A, typename B, typename C, typename = std::enable_if_t<
				IsVecF<A> && EqualMat<A, B> && (EqualMat<A, C> || IsFloat<C>)
			> >
				Vec<FLOAT, Infos<A>::rows> mix(A && a, B && b, C && c) {
				return { createFCallExp("mix", EX(A,a), EX(B,b), EX(C,c)) };
			}

			template<typename A, typename B, typename = std::enable_if_t <
				EqualMat<A, vec3> && EqualMat<B, vec3>
			> >
				vec3 cross(A && a, B && b) {
				return { createFCallExp("cross", EX(A,a), EX(B,b)) };
			}

			template<typename S, typename P, typename DX, typename DY, typename SI = SamplerInfos<S>,
				typename = std::enable_if_t<
				(SI::access_type == SAMPLER) && (SI::type == BASIC || SI::type == CUBE) && (SI::flags == 0) &&
				IsVecF<P> && Infos<P>::rows == (SI::type == CUBE ? 3 : SI::size) && EqualMat<P, DX> && EqualMat<P, DY>
			> >
				Vec< SI::scalar_type, 4> textureGrad(S&& sampler, P&& point, DX&& dx, DY&& dy) {
				return { createFCallExp("textureGrad", EX(S, sampler) , EX(P, point), EX(DX, dx), EX(DY, dy)) };
			}

			template<typename A, typename B, typename C, typename = std::enable_if_t<
				IsVecF<A> && EqualMat<B, C> && (EqualMat<A, B> || IsConvertibleTo<B, Float>)
			> >
				Vec<FLOAT, Infos<A>::rows> clamp(A && x, B && minVal, C && maxVal) {
				return { createFCallExp("clamp", EX(A,x), EX(B,minVal), EX(C, maxVal)) };
			}

			template<typename A, typename B, typename C, typename = std::enable_if_t<
				IsVecF<C> && EqualMat<B, A> && (EqualMat<A, C> || IsConvertibleTo<A, Float>)
			> >
				Vec<FLOAT, Infos<C>::rows> smoothstep(A && edge0, B && edge1, C && x) {
				return { createFCallExp("smoothstep", EX(A,edge0), EX(B,edge1), EX(C, x)) };
			}

			template<typename A, typename B,typename = std::enable_if_t<
				IsVecF<A> && (EqualMat<A, B> || IsFloat<B>)
			> >
				Vec<FLOAT, Infos<A>::rows> step(A&& edge, B&& x) {
				return { createFCallExp("step", EX(A,edge), EX(B,x)) };
			}

			template<typename A, typename B, typename = std::enable_if_t <
				IsSquare<A> && (Infos<A>::rows >= 2) && EqualMat<A, B>
			> >
				Matrix<FLOAT, Infos<A>::rows, Infos<A>::cols> matrixCompMult(A&& x, B&& y) {
				return { createFCallExp("matrixCompMult", EX(A,x), EX(B,y)) };
			}

			CSL_PP_ITERATE(GENTYPE_OP_GENTYPE, abs, sin, cos, tan, exp, log, sqrt, floor, ceil, fract, exp2, log2, normalize, atan, acos, asin, radians, degrees);

			CSL_PP_ITERATE(RELATIONAL_GENTYPE_OP, greaterThan, lessThan, greaterThanEqual, lessThenEqual, equal, notEqual);

			template<typename A, typename B, typename = std::enable_if_t <
				IsVecF<A> && EqualMat<A, B>
			> >
				Matrix<FLOAT, Infos<A>::rows, Infos<A>::cols> atan(A&& y, B&& x) {
				return { createFCallExp("atan", EX(A,y), EX(B,x)) };
			}

			BOOL_OP(any);
			BOOL_OP(all);

			BVEC_OP_GENTYPE(isinf);
		}

		namespace glsl_120 {
			using namespace glsl_110;

			template<typename A, typename B, typename = std::enable_if_t <
				IsVecF<A> && IsVecF<B> && (Infos<A>::rows >= 2) && (Infos<A>::rows >= 2)
			> >
				Matrix<FLOAT, Infos<A>::rows, Infos<B>::rows> outerProduct(A&& c, B&& r) {
				return { createFCallExp("outerProduct", EX(A,c), EX(B,r)) };
			}

			template<typename A, typename = std::enable_if_t<
				(Infos<A>::scalar_type == FLOAT) && (Infos<A>::cols >= 2) && (Infos<A>::rows >= 2)
			> >
				Matrix<FLOAT, Infos<A>::cols, Infos<A>::rows> transpose(A&& m) {
				return { createFCallExp("transpose", EX(A,m)) };
			}

		}

		namespace glsl_130 {
			using namespace glsl_120;

			// ScalarType nType, unsigned int N, SamplerIsArray is_array,

			template<typename S, typename P, typename SI = SamplerInfos<S>,
				typename = std::enable_if_t<
				(SI::access_type == SAMPLER) && (SI::type == BASIC) && ((SI::flags & IS_SHADOW) == 0) &&
				IsVecF<P> &&
				Infos<P>::rows == (SI::size + ((SI::flags & IS_ARRAY) ? 1 : 0))
			> >
				Vec< SI::scalar_type, 4> texture(S && sampler, P && point) {
				return {
					createFCallExp("texture", EX(S, sampler) , EX(P, point))
				};
			}

			template<typename S, typename P, typename SI = SamplerInfos<S>, typename B,
				typename = std::enable_if_t<
				(SI::access_type == SAMPLER) && (SI::type == BASIC) && (SI::flags == 0) &&
				IsVecF<P> && IsFloat<B> &&
				Infos<P>::rows == (SI::size + ((SI::flags & IS_ARRAY) ? 1 : 0))
			> >
				Vec< SI::scalar_type, 4> texture(S && sampler, P && point, B && bias) {
				return { createFCallExp("texture", EX(S, sampler) , EX(P, point), EX(B, bias)) };
			}

			template<typename S, typename P, typename L, typename SI = SamplerInfos<S>,
				typename = std::enable_if_t<
				(SI::access_type == SAMPLER) && (SI::type == BASIC) && (SI::flags == 0) &&
				IsVecInteger<P> && EqualMat<L, Int> && Infos<P>::rows == SI::size
			> >
				Vec< SI::scalar_type, 4> texelFetch(S && sampler, P && point, L && lod) {
				return { createFCallExp("texelFetch", EX(S, sampler) , EX(P, point), EX(L, lod)) };
			}

			template<typename S, typename P, typename L, typename SI = SamplerInfos<S>,
				typename = std::enable_if_t<
				(SI::access_type == SAMPLER) && (SI::type == BASIC || SI::type == CUBE) && (SI::flags == 0) &&
				IsVecF<P> && Infos<P>::rows == (SI::type == CUBE ? 3 : SI::size) && IsFloat<L>
			> >
				Vec< SI::scalar_type, 4> textureLod(S && sampler, P && point, L && lod) {
				return { createFCallExp("textureLod", EX(S, sampler) , EX(P, point), EX(L, lod)) };
			}

			template<typename A, typename B, typename C, typename IA = Infos<A>, typename = std::enable_if_t<
				IsVecInteger<A> && EqualMat<B, C> && (EqualMat<A, B> || IsConvertibleTo<B, Vec<IA::scalar_type, 1>>)
			> >
				Vec<IA::scalar_type, IA::rows> clamp(A && x, B && minVal, C && maxVal) {
				return { createFCallExp("clamp", EX(A,x), EX(B,minVal), EX(C, maxVal)) };
			}
		
			CSL_PP_ITERATE(GENTYPE_OP_GENTYPE, sinh, cosh, tanh, inversesqrt, sign, round, roundEven);

			BVEC_OP_GENTYPE(isnan);
		}

		namespace glsl_140 {
			using namespace glsl_130;

			template<typename A, typename = std::enable_if_t<
				IsSquare<A> && (Infos<A>::rows >= 2) && (Infos<A>::scalar_type == FLOAT)
				>>
				Matrix<FLOAT, Infos<A>::rows, Infos<A>::cols> inverse(A&& m) {
				return { createFCallExp("inverse", EX(A,m)) };
			}
		}

		namespace glsl_150 {
			using namespace glsl_140;

			template<typename A, typename = std::enable_if_t<
					IsSquare<A> && (Infos<A>::rows >= 2) && (Infos<A>::scalar_type == FLOAT)
				>> 
				Float determinant(A&& m) {
				return { createFCallExp("determinant", EX(A,m)) };
			}
		}

		namespace glsl_330 {
			using namespace glsl_150;

			ANYTYPE_OP_ANYTYPE(FLOAT, INT, floatBitsToInt);
			ANYTYPE_OP_ANYTYPE(FLOAT, UINT, floatBitsToUint);
			ANYTYPE_OP_ANYTYPE(INT, FLOAT, intBitsToFloat);
			ANYTYPE_OP_ANYTYPE(UINT, FLOAT, uintBitsToFloat);
		}

		namespace glsl_400 {
			using namespace glsl_330;

			template<typename A, typename = std::enable_if_t<
				Infos<A>::cols == 1 && Infos<A>::scalar_type == DOUBLE
			> >
				Float length(A && v) {
				return Float(createFCallExp("length", EX(A, v)));
			}

			template<typename A, typename B, typename = std::enable_if_t<
				Infos<A>::cols == 1 && Infos<A>::scalar_type == DOUBLE && Infos<B>::cols == 1 && Infos<B>::scalar_type == DOUBLE
			> >
				Float distance(A && a, B && b) {
				return Float(createFCallExp("distance", EX(A, a), EX(B, b)));
			}

			template<typename A, typename = std::enable_if_t<
				(Infos<A>::scalar_type == DOUBLE) && (Infos<A>::cols >= 2) && (Infos<A>::rows >= 2)
			> >
				Matrix<DOUBLE, Infos<A>::cols, Infos<A>::rows> transpose(A&& m) {
				return { createFCallExp("transpose", EX(A,m)) };
			}

			template<typename S, typename P, typename L, typename SI = SamplerInfos<S>,
				typename = std::enable_if_t<
				(SI::access_type == SAMPLER) && (SI::type == CUBE) && (SI::flags & IS_ARRAY) &&
				IsVecF<P> && Infos<P>::rows == 4 && IsFloat<L>
			> >
				Vec< SI::scalar_type, 4> textureLod(S && sampler, P && point, L && lod) {
				return {
					createFCallExp("textureLod", EX(S, sampler) , EX(P, point), EX(L, lod))
				};
			}

			template<typename A, typename B, typename C, typename IA = Infos<A>, typename =
				std::enable_if_t< IsVecInteger<A> && EqualMat<B, Int> && EqualMat <C, Int> > >
				Vec<IA::scalar_type, IA::rows> bitfieldExtract(A && value, B && offset, C && bits)
			{
				return { createFCallExp("bitfieldExtract", EX(A, value) , EX(B, offset), EX(C, bits)) };
			}

			template<typename A, typename B, typename C, typename =
				std::enable_if_t< (IsVecF<A> || IsVecD<A>) && EqualMat<A, B> && EqualMat <A, C> > >
				Vec<Infos<A>::scalar_type, Infos<A>::rows> fma(A&& a, B&& b, C&& c)
			{
				return { createFCallExp("fma", EX(A, a) , EX(B, b), EX(C, b)) };
			}

			template<typename A, typename B, typename C, typename =
				std::enable_if_t< (IsVecF<A> || IsVecD<A>) && IsVecI<B> && EqualDim<A, B> > >
				Vec<Infos<A>::scalar_type, Infos<A>::rows> ldexp(A&& x, B&& exp)
			{
				return { createFCallExp("ldexp", EX(A, x) , EX(B, exp)) };
			}

			template<typename A, typename = std::enable_if_t<
				IsSquare<A> && (Infos<A>::rows >= 2) && (Infos<A>::scalar_type == DOUBLE)
				>>
				Double determinant(A&& m) {
				return { createFCallExp("determinant", EX(A,m)) };
			}

			template<typename A, typename = std::enable_if_t<
				IsSquare<A> && (Infos<A>::rows >= 2) && (Infos<A>::scalar_type == DOUBLE)
				>>
				Matrix<DOUBLE, Infos<A>::rows, Infos<A>::cols> inverse(A&& m) {
				return { createFCallExp("inverse", EX(A,m)) };
			}

			TYPE_OP_TYPE(vec2, v, Uint, packUnorm2x16);
			TYPE_OP_TYPE(vec4, v, Uint, packUnorm4x8);
			TYPE_OP_TYPE(vec4, v, Uint, packSnorm4x8);
			TYPE_OP_TYPE(Uint, p, vec2, unpackUnorm2x16);
			TYPE_OP_TYPE(Uint, p, vec4, unpackUnorm4x8);
			TYPE_OP_TYPE(Uint, p, vec4, unpackSnorm4x8);

			TYPE_OP_TYPE(uvec2, v, Double, packDouble2x32);
			TYPE_OP_TYPE(Double, d, uvec2, unpackDouble2x32);

			template<typename A, typename B, typename = std::enable_if_t<
				IsVecF<A> && IsVecI<B> && EqualDim<A, B> && FunctionOut<B>
				>>
				Vec<FLOAT, Infos<A>::rows> frexp(A&& x, B&& exp) {
				return { createFCallExp("frexp", EX(A,x), EX(B,exp)) };
			}

			template<typename A, typename B, typename C, typename = std::enable_if_t<
					IsVecU<A> && EqualMat<A,B> && EqualMat<A,C> && FunctionOut<C>
				>>
				Vec<UINT, Infos<A>::rows> uaddCarry(A&& x, B&& y, C&& carry) {
				return { createFCallExp("uaddCarry", EX(A,x), EX(B,y), EX(C,carry)) };
			}

			template<typename A, typename B, typename C, typename = std::enable_if_t<
				IsVecU<A> && EqualMat<A, B> && EqualMat<A, C> && FunctionOut<C>
				>>
				Vec<UINT, Infos<A>::rows> usubBorrow(A&& x, B&& y, C&& carry) {
				return { createFCallExp("usubBorrow", EX(A,x), EX(B,y), EX(C,carry)) };
			}

			template<typename A, typename B, typename C, typename D, typename = std::enable_if_t<
				IsVecU<A> && EqualMat<A, B> && EqualMat<A, C> && EqualMat<A, D> && FunctionOut<C> && FunctionOut<D>
				>>
				void umulExtended(A&& x, B&& y, C&& msb, D&& lsb) {
				listen().addEvent(createFCallExp("umulExtended", EX(A, x), EX(B, y), EX(C, msb), EX(D, lsb)));
			}
			
			template<typename A, typename B, typename C, typename D, typename = std::enable_if_t<
				IsVecI<A> && EqualMat<A, B> && EqualMat<A, C> && EqualMat<A, D> && FunctionOut<C> && FunctionOut<D>
				>>
				void imulExtended(A&& x, B&& y, C&& msb, D&& lsb) {
				listen().addEvent(createFCallExp("imulExtended", EX(A, x), EX(B, y), EX(C, msb), EX(D, lsb)));
			}

			template<typename A, typename B, typename C, typename D, typename = std::enable_if_t<
				IsVecInteger<A> && EqualMat<A, B> && IsInt<C> && IsInt<D>
				>>
			Vec< Infos<A>::scalar_type, Infos<A>::rows > bitfieldInsert(A&& base, B&& insert, C&& offset, D&& bits) {
				return { createFCallExp("bitfieldInsert", EX(A,base), EX(B,insert), EX(C,offset), EX(D,bits)) };
			}

			template<typename A, typename = std::enable_if_t<IsVecInteger<A>> >
			Vec<Infos<A>::scalar_type, Infos<A>::rows> bitfieldReverse(A&& value) {
				return { createFCallExp("bitfieldReverse", EX(A,value)) };
			}

			template<typename A, typename = std::enable_if_t<IsVecInteger<A>> >
			Vec<INT, Infos<A>::rows> bitCount(A&& value) {
				return { createFCallExp("bitCount", EX(A,value)) };
			}

			template<typename A, typename = std::enable_if_t<IsVecInteger<A>> >
			Vec<INT, Infos<A>::rows> findLSB(A&& value) {
				return { createFCallExp("findLSB", EX(A,value)) };
			}

			template<typename A, typename = std::enable_if_t<IsVecInteger<A>> >
			Vec<INT, Infos<A>::rows> findMSB(A&& value) {
				return { createFCallExp("findMSB", EX(A,value)) };
			}
		}
			

		namespace glsl_410 {
			using namespace glsl_400;
		}

		namespace glsl_420 {
			using namespace glsl_410;

			TYPE_OP_TYPE(vec2, v, Uint, packSnorm2x16);
			TYPE_OP_TYPE(Uint, p, vec2, unpackSnorm2x16);

			TYPE_OP_TYPE(vec2, v, Uint, packHalf2x16);
			TYPE_OP_TYPE(Uint, v, vec2, unpackHalf2x16);
		}

		namespace glsl_430 {
			using namespace glsl_420;
		}

		namespace glsl_440 {
			using namespace glsl_430;
		}

		namespace glsl_450 {
			using namespace glsl_440;
		}

	}

} //namespace csl

#undef GENTYPE_OP_GENTYPE
#undef RELATIONAL_GENTYPE_OP
#undef BOOL_OP
#undef BVEC_OP_GENTYPE
#undef ANYTYPE_OP_ANYTYPE
#undef TYPE_OP_TYPE

#undef EX
