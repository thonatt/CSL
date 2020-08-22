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
		struct Inout {};

		template<std::size_t N>
		struct Binding {};

		template<std::size_t N>
		struct Location {};

		struct Triangles {};
		struct Equal_spacing {};
		struct Ccw {};

		struct Std140 {};
		struct Rgba32f {};

		namespace tcs_common {
			struct Patch {};

			template<std::size_t N>
			struct Vertices {};
		}

		namespace geom_common {
			struct Line_strip {};

			template<std::size_t N>
			struct Max_vertices {};
		}

		namespace compute_common {
			template<std::size_t N>
			struct Local_size_x {};

			template<std::size_t N>
			struct Local_size_y {};
		}
	}

}