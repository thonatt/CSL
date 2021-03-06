#pragma once

#include "Types.hpp"
#include "NamedObjects.hpp"

namespace csl {

	template<SamplerAccessType Access, typename T, std::size_t N, SamplerType Type, SamplerFlags Flags, typename ...Qs>
	class Sampler : public NamedObject<Sampler<Access, T, N, Type, Flags, Qs...>> {
	public:

		using Base = NamedObject<Sampler>;
		using ArrayDimensions = SizeList<>;
		using Qualifiers = TList<Qs...>;
		using QualifierFree = Sampler;

		Sampler() : Base("") {}

		Sampler(Dummy) : Base() {}

		Sampler(const std::string& s, const ObjFlags obj_flags = ObjFlags::Default)
			: Base(s, obj_flags) {
		}

		Sampler(const Expr& expr, const ObjFlags obj_flags = ObjFlags::Default)
			: Base(expr, obj_flags)
		{
		}

	};

}
