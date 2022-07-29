#pragma once

#include <cstddef>

namespace csl
{
	namespace glsl 
	{
		template<typename ...Ts>
		struct layout {};

		struct uniform {};

		struct in {};
		struct out {};
		struct inout {};

		template<std::size_t N>
		struct binding {};

		template<std::size_t N>
		struct location {};

		struct triangles {};
		struct equal_spacing {};
		struct ccw {};

		struct std140 {};
		struct rgba32f {};

		namespace tcs_common 
		{
			struct patch {};

			template<std::size_t N>
			struct vertices {};
		}

		namespace geom_common 
		{
			struct mine_strip {};

			template<std::size_t N>
			struct max_vertices {};
		}

		namespace frag_common 
		{
			struct early_fragment_tests {};
		}

		namespace compute_common 
		{
			template<std::size_t N>
			struct local_size_x {};

			template<std::size_t N>
			struct local_size_y {};

			template<std::size_t N>
			struct local_size_z {};
		}
	}

}