#pragma once

#include "Types.hpp"
#include "NamedObjects.hpp"

namespace v2 {

	template<SamplerAccessType Access, typename T, std::size_t N, SamplerType Type, SamplerFlags Flags>
	class Sampler : public NamedObject<Sampler<Access, T, N, Type, Flags>> {
	public:

		virtual ~Sampler() = default;
		using Base = NamedObject<Sampler>;
		using ArrayDimensions = SizeList<>;
		using Qualifiers = TList<>;
		using QualifierFree = Sampler;

		Sampler(const std::string& s = "", const ObjFlags obj_flags = ObjFlags::Default)
			: Base(s, obj_flags) {
		}

		Sampler(const Expr& expr, const ObjFlags obj_flags = ObjFlags::Default)
			: Base(expr, obj_flags)
		{
		}

	};

}
