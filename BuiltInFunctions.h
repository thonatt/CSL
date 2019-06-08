#pragma once

#include "MatrixTypesTest.h"


#define GENTYPE_OP_GENTYPE(name) \
	template<typename A, typename = std::enable_if_t< IsVecF<A> > > \
		Vec<FLOAT, Infos<CT<A>>::rows> name(A && a) { \
		return { createFCallExp(#name, EX(A,a)) };	\
	} 

#define RELATIONAL_GENTYPE_OP(name) \
	template<typename A, typename B, typename = std::enable_if_t< IsVecF<A> && EqualMat<A,B> > > \
		Vec<BOOL, Infos<CT<A>>::rows> name(A && a, B && b) { \
		return { createFCallExp(#name, EX(A,a), EX(B,b) ) };	\
	} 

#define BOOL_OP(name) \
template<typename A, typename = std::enable_if_t< IsVecB<A> > > \
	Bool name(A && a) { \
		return { createFCallExp(#name, EX(A,a)) };	\
	}

namespace glsl_110 {

	template<typename A, typename = std::enable_if_t< IsVecF<A> > > 
		Float length(A && a) {		
		return { createFCallExp("length", EX(A,a)) };	
	} 

	template<typename A, typename B, typename = std::enable_if_t<
		NoBools<A, B> && IsConvertibleTo<A,Vec<FLOAT,Infos<CT<B>>::rows>> && IsConvertibleTo<B, Vec<FLOAT, Infos<CT<A>>::rows>> 
	> >
	Vec<FLOAT, Infos<CT<A>>::rows> pow(A && a, B && b) {
		return { createFCallExp("pow", EX(A,a), EX(B,b)) };
	}

	template<typename A, typename B, typename = std::enable_if_t<
		IsVecF<A> && EqualMat<A,B>
	> >
		Float dot(A && a, B && b) {
		return { createFCallExp("dot", EX(A,a), EX(B,b)) };
	}

	template<typename I, typename N, typename = std::enable_if_t<
		IsVecF<I> && EqualMat<I, N>
	> >
		Vec<FLOAT, Infos<CT<I>>::rows> reflect(I && i, N && n) {
		return { createFCallExp("reflect", EX(I,i), EX(N,n)) };
	}

	template<typename A, typename B, typename = std::enable_if_t<
		IsVecF<A> && (EqualMat<A, B> || IsFloat<B>)
	> >
		Vec<FLOAT, Infos<CT<A>>::rows> max(A && a, B && b) {
		return { createFCallExp("max", EX(A,a), EX(B,b)) };
	}

	template<typename A, typename B, typename C, typename = std::enable_if_t<
		IsVecF<A> && EqualMat<A, B> && (EqualMat<A,C> || IsFloat<C>)
	> >
		Vec<FLOAT, Infos<CT<A>>::rows> mix(A && a, B && b, C && c) {
		return { createFCallExp("mix", EX(A,a), EX(B,b), EX(C,c)) };
	}

	template<typename A, typename B, typename = std::enable_if_t <
		EqualMat<A,vec3> && EqualMat<B, vec3>
	> >
		vec3 cross(A && a, B && b) {
		return { createFCallExp("cross", EX(A,a), EX(B,b)) };
	}

	template<typename A, typename = std::enable_if_t < IsVecB<A> > >
		Vec<BOOL, Infos<CT<A>>::rows> Not(A && a) {
		return { createFCallExp("not", EX(A,a)) };
	}

	template<typename A, typename B, typename C, typename = std::enable_if_t<
		IsVecF<A> && EqualMat<B,C> && (EqualMat<A,B> || IsConvertibleTo<B,Float> )
	> >
		Vec<FLOAT, Infos<CT<A>>::rows> clamp(A && x, B && minVal, C && maxVal) {
		return { createFCallExp("clamp", EX(A,x), EX(B,minVal), EX(C, maxVal)) };
	}

	GENTYPE_OP_GENTYPE(abs);
	GENTYPE_OP_GENTYPE(sin);
	GENTYPE_OP_GENTYPE(cos);
	GENTYPE_OP_GENTYPE(tan);
	GENTYPE_OP_GENTYPE(exp);
	GENTYPE_OP_GENTYPE(log);
	GENTYPE_OP_GENTYPE(sqrt);
	GENTYPE_OP_GENTYPE(ceil);
	GENTYPE_OP_GENTYPE(floor);
	GENTYPE_OP_GENTYPE(fract);
	GENTYPE_OP_GENTYPE(exp2);
	GENTYPE_OP_GENTYPE(log2);

	GENTYPE_OP_GENTYPE(normalize);

	RELATIONAL_GENTYPE_OP(greaterThan);
	RELATIONAL_GENTYPE_OP(lessThan);
	RELATIONAL_GENTYPE_OP(greaterThanEqual);
	RELATIONAL_GENTYPE_OP(lessThenEqual);
	RELATIONAL_GENTYPE_OP(equal);
	RELATIONAL_GENTYPE_OP(notEqual);

	BOOL_OP(any);
	BOOL_OP(all);

}

namespace glsl_120 {
	using namespace glsl_110;
}

namespace glsl_130 {
	using namespace glsl_120;

	// ScalarType nType, unsigned int N, SamplerIsArray is_array,

	template<typename S, typename P, typename SI = SamplerInfos<CT<S>>,
		typename = std::enable_if_t<
		(SI::access_type == SAMPLER ) && (SI::type == BASIC) && (SI::flags == 0) &&
		IsVecF<P> &&
		Infos<CT<P>>::rows == ( SI::size + ( (SI::flags & IS_ARRAY) ? 1 : 0 ) ) 
	> >
		Vec< SI::scalar_type, 4> texture(S && sampler, P && point) {
		return {
			createFCallExp("texture", EX(S, sampler) , EX(P, point) )
		};
	}

	template<typename S, typename P, typename SI = SamplerInfos<CT<S>>, typename B,
		typename = std::enable_if_t<
		(SI::access_type == SAMPLER) && (SI::type == BASIC) && (SI::flags == 0) &&
		IsVecF<P> && IsFloat<B> &&
		Infos<CT<P>>::rows == (SI::size + ((SI::flags & IS_ARRAY) ? 1 : 0))		
	> >
		Vec< SI::scalar_type, 4> texture(S && sampler, P && point, B && bias) {
		return {
			createFCallExp("texture", EX(S, sampler) , EX(P, point), EX(B, bias))
		};
	}

	template<typename S, typename P, typename L, typename SI = SamplerInfos<CT<S>>,
		typename = std::enable_if_t<
		(SI::access_type == SAMPLER) && (SI::type == BASIC) && (SI::flags == 0) &&
		IsInt<P> && EqualMat<L,Int> && Infos<CT<P>>::rows == SI::size
	> >
		Vec< SI::scalar_type, 4> texelFetch(S && sampler, P && point, L && lod ) {
		return {
			createFCallExp("texelFetch", EX(S, sampler) , EX(P, point), EX(L, lod) )
		};
	}

	template<typename S, typename P, typename L, typename SI = SamplerInfos<CT<S>>,
		typename = std::enable_if_t<
		(SI::access_type == SAMPLER) && (SI::type == BASIC || SI::type == CUBE) && (SI::flags == 0) &&
		IsVecF<P> && Infos<CT<P>>::rows == ( SI::type == CUBE ? 3 : SI::size ) && IsFloat<L>
	> >
		Vec< SI::scalar_type, 4> textureLod(S && sampler, P && point, L && lod) {
		return {
			createFCallExp("textureLod", EX(S, sampler) , EX(P, point), EX(L, lod))
		};
	}

	GENTYPE_OP_GENTYPE(sinh);
	GENTYPE_OP_GENTYPE(cosh);
	GENTYPE_OP_GENTYPE(tanh);
	GENTYPE_OP_GENTYPE(inversesqrt);

	GENTYPE_OP_GENTYPE(sign);
	GENTYPE_OP_GENTYPE(round);
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

	template<typename R_A, typename A = CleanType<R_A>,
		typename = std::enable_if_t<
		Infos<A>::cols == 1 && Infos<A>::scalar_type == DOUBLE
	> >
		Float length(R_A && v) {
		return Float(createFCallExp("length", getExp<R_A>(v) ));
	}

	template<typename S, typename P, typename L, typename SI = SamplerInfos<CT<S>>,
		typename = std::enable_if_t<
		(SI::access_type == SAMPLER) && (SI::type == CUBE) && (SI::flags & IS_ARRAY) &&
		IsVecF<P> && Infos<CT<P>>::rows == 4 && IsFloat<L>
	> >
		Vec< SI::scalar_type, 4> textureLod(S && sampler, P && point, L && lod) {
		return {
			createFCallExp("textureLod", EX(S, sampler) , EX(P, point), EX(L, lod))
		};
	}
}

namespace glsl_410 {
	using namespace glsl_400;
}

namespace glsl_420 { 
	using namespace glsl_410;
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

#undef GENTYPE_OP_GENTYPE
