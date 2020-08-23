#pragma once

#include <cstddef>

namespace v2 {

	template<std::size_t ... Ns>
	struct Array {};

	template<>
	struct Array<> : Array<0>{};
}
