#pragma once

#include "MatrixTypesTest.h"

namespace glsl_1_10 {

	template<typename R_A, typename A = CleanType<R_A>,
		typename = std::enable_if_t< 
			Infos<A>::cols == 1 && Infos<A>::scalar_type == FLOAT
		> >
	Float length(R_A && v) {
		return Float(createExp(std::make_shared<FunctionOp<>>("length"), getExp<R_A>(v)));
	}

}

namespace glsl_1_20 {
	using namespace glsl_1_10;
}

namespace glsl_1_30 {
	using namespace glsl_1_20;

	template<typename R_A, typename A = CleanType<R_A>, ScalarType nType, unsigned int N, SamplerIsArray is_array, typename ... R_B ,
		typename = std::enable_if_t<
		IsVecF<A> && (Infos<A>::rows == N + (int)is_array) && (sizeof...(R_B) == 0 || ( sizeof...(R_B) == 1 && IsFloat<CleanType<R_B>...> ) )
	> >
		Vec<nType, 4> texture(const Sampler<SAMPLER, nType, N, BASIC, is_array> & sampler, R_A &&  P, R_B && ... bias) {
		return Vec<nType, 4>(createExp(std::make_shared<FunctionOp<>>("texture"), getExpForced<false>(sampler), getExp<R_A>(P), getExp<R_B>(bias)... ));
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
		return Float(createExp(std::make_shared < FunctionOp<> > ("length"), getExp<R_A>(v)));
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