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

	namespace geom_common {
		struct Triangles {};
		struct Line_strip {};

		template<std::size_t N>
		struct Max_vertices {};
	}
}