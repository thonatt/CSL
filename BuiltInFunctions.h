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