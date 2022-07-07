#pragma once

#include "TemplateHelpers.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <type_traits>

namespace csl
{
	namespace swizzling
	{
		template<char... chars>
		struct CharSeq : VList<char, chars...>
		{
			template<char c>
			static constexpr std::size_t GetIndex()
			{
				constexpr std::array equals{ (chars == c)... };
				for (std::size_t index = 0; index < equals.size(); ++index)
					if (equals[index])
						return index + 1;
				return 0;
			}
		};

		using SwizzleRGBA = CharSeq<'r', 'g', 'b', 'a'>;
		using SwizzleXYZW = CharSeq<'x', 'y', 'z', 'w'>;
		using SwizzleSTPQ = CharSeq<'s', 't', 'p', 'q'>;

		using AllowedSwizzleSets = TList<SwizzleRGBA, SwizzleXYZW, SwizzleSTPQ>;

		template<char c>
		struct SwizzleGetter
		{
			template<typename Set>
			struct IsInSet
			{
				static constexpr bool Value = (Set::template GetIndex<c>() != 0);
			};
		};

		template<char c>
		using MatchedSwizzleSet = typename Matching<SwizzleGetter<c>::template IsInSet, AllowedSwizzleSets>::Values;

		template<char ...cs>
		struct MatchedSwizzleSets
		{
			static constexpr bool AllValid = ((MatchedSwizzleSet<cs>::Size == 1) &&...);
			static_assert(AllValid, "Invalid Swizzle Set");
			using AllValidSets = TList<typename MatchedSwizzleSet<cs>::template At<0> ...>;
			using ValidSets = RemoveDuplicates<AllValidSets>;
		};

		template<char ...>
		constexpr bool are_unique = true;

		template<char c, char d, char ...cs>
		constexpr bool are_unique<c, d, cs...> = (c != d) && are_unique<d, cs...>;
	}

	template<char... chars>
	class Swizzle
	{
	private:
		using Sets = typename swizzling::MatchedSwizzleSets<chars...>::ValidSets;
		static_assert(Sets::Size == 1, "Swizzle sets cannot be mixed");
		using Set = typename Sets::template At<0>;

	public:
		static constexpr std::size_t Size = sizeof...(chars);
		static_assert(Size >= 1, "Minimum swizzling size is 1");
		static_assert(Size <= 4, "Maximum swizzling size is 4");

		static constexpr std::size_t HighestComponent = std::max({ Set::template GetIndex<chars>() ... });
		static constexpr bool NoDuplicates = swizzling::are_unique<chars...>;

		constexpr Swizzle() = default;

		template<char other_c>
		constexpr Swizzle<chars..., other_c> operator,(Swizzle<other_c>) const
		{
			return {};
		}
	};

	namespace swizzles
	{
		namespace rgba
		{
			inline constexpr Swizzle<'r'> r;
			inline constexpr Swizzle<'g'> g;
			inline constexpr Swizzle<'b'> b;
			inline constexpr Swizzle<'a'> a;
		}

		namespace xyzw
		{
			inline constexpr Swizzle<'x'> x;
			inline constexpr Swizzle<'y'> y;
			inline constexpr Swizzle<'z'> z;
			inline constexpr Swizzle<'w'> w;
		}

		namespace stpq
		{
			inline constexpr Swizzle<'s'> s;
			inline constexpr Swizzle<'t'> t;
			inline constexpr Swizzle<'p'> p;
			inline constexpr Swizzle<'q'> q;
		}

		namespace all
		{
			using namespace rgba;
			using namespace xyzw;
			using namespace stpq;
		}
	}
}