#pragma once

#include "Types.hpp"
#include "NamedObjects.hpp"

namespace csl {

	template<typename T, std::size_t N, SamplerFlags Flags>
	class Sampler : public NamedObject<Sampler<T, N, Flags>> 
	{
	public:
		using Base = NamedObject<Sampler>;

		Sampler(Dummy) : Base() {}

		Sampler() : Sampler("") {}

		Sampler(const std::string& s, const ObjFlags obj_flags = ObjFlags::Default)
			: Base(s, obj_flags, SizeList<>{}, TList<>{})
		{
		}

		Sampler(const Expr& expr, const ObjFlags obj_flags = ObjFlags::Default)
			: Base(expr, obj_flags, SizeList<>{}, TList<>{})
		{
		}

		static constexpr bool IsValid() { return true; }
	};
}
