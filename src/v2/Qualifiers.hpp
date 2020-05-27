#pragma once

#include <cstddef>

namespace v2 {

	template<std::size_t ... Ns>
	struct Array {};

	template<typename ...Ts>
	struct Layout {};

	struct Uniform {};

	template<std::size_t N>
	struct Binding {};


}
