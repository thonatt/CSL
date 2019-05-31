#pragma once

#include "MatrixTypesTest.h"

namespace glsl_1_10 {

	template<typename A, typename B,
		typename = std::enable_if_t< 
		NoBools<A, B> && IsConvertibleTo<A,Vec<FLOAT,Infos<CT<B>>::rows>> && IsConvertibleTo<B, Vec<FLOAT, Infos<CT<A>>::rows>>
		>
	>
	Vec<FLOAT, Infos<CT<A>>::rows> pow(A && a, B && b) {
		return { createFCallExp("pow", EX(A,a), EX(B,b)) };
	}

	template<typename R_A, typename A = CleanType<R_A>,
		typename = std::enable_if_t< 
			Infos<A>::cols == 1 && Infos<A>::scalar_type == FLOAT
		> >
	Float length(R_A && v) {
		return Float(createFCallExp("length", getExp<R_A>(v)));
	}

}

namespace glsl_1_20 {
	using namespace glsl_1_10;
}

namespace glsl_1_30 {
	using namespace glsl_1_20;

	// ScalarType nType, unsigned int N, SamplerIsArray is_array,

	template<typename R_A, typename R_S, typename A = CleanType<R_A>, typename S = CleanType<R_S>,
		typename SI = SamplerInfos<S>, typename ... R_B ,
		typename = std::enable_if_t<
		(SI::access_type == SAMPLER ) && (SI::type == BASIC) && 
		IsVecF<A> &&
		Infos<A>::rows == ( SI::size + ( (SI::flags & IS_ARRAY) ? 1 : 0 ) ) &&
		(sizeof...(R_B) == 0 || ( sizeof...(R_B) == 1 && IsFloat<CleanType<R_B>...> ) )
	> >
		Vec< SI::scalar_type, 4> texture(R_S && sampler, R_A &&  P, R_B && ... bias) {
		return {
			createFCallExp("texture", getExp<R_S>(sampler), getExp<R_A>(P), getExp<R_B>(bias)...)
		};
	}
}

namespace glsl_1_40 {
	using namespace glsl_1_30;
}

namespace glsl_1_50 {
	using namespace glsl_1_40;
}

namespace glsl_3_30 {
	using namespace glsl_1_50;
}

namespace glsl_4_00 {
	using namespace glsl_3_30;

	template<typename R_A, typename A = CleanType<R_A>,
		typename = std::enable_if_t<
		Infos<A>::cols == 1 && Infos<A>::scalar_type == DOUBLE
	> >
		Float length(R_A && v) {
		return Float(createFCallExp("length", getExp<R_A>(v) ));
	}
}

namespace glsl_4_10 {
	using namespace glsl_4_00;
}

namespace glsl_4_20 { 
	using namespace glsl_4_10;
}

namespace glsl_4_30 {
	using namespace glsl_4_20;
}

namespace glsl_4_40 {
	using namespace glsl_4_30;
}

namespace glsl_4_50 {
	using namespace glsl_4_40;
}