#pragma once

#include <cstddef>

namespace v2
{
	namespace glsl {

		template<typename ...Ts>
		struct Layout {};

		struct Uniform {};

		struct In {};

		struct Out {};

		template<std::size_t N>
		struct Binding {};

		template<std::size_t N>
		struct Location {};

	}
}