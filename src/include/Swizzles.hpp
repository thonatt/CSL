#pragma once

#include "TemplateHelpers.hpp"

#include <algorithm>
#include <cstddef>
#include <type_traits>

namespace csl
{
	namespace swizzling
	{
		template<char... chars>
		struct CharSeq {};

		using SwizzleRGBA = CharSeq<'r', 'g', 'b', 'a'>;
		using SwizzleXYZW = CharSeq<'x', 'y', 'z', 'w'>;
		using SwizzleSTPQ = CharSeq<'s', 't', 'p', 'q'>;

		using AllowedSwizzleSets = TList<SwizzleRGBA, SwizzleXYZW, SwizzleSTPQ>;

		template<char c, typename Set>
		constexpr bool is_in_set = false;

		template<char c, char... chars>
		constexpr bool is_in_set<c, CharSeq<chars...>> = ((c == chars) || ...);

		template<char c>
		struct SwizzleGetter
		{
			template<typename Set>
			struct IsInSet
			{
				static constexpr bool Value = is_in_set<c, Set>;
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

		template<char c>
		constexpr std::size_t SwizzleIndex = 0;

		template<> constexpr std::size_t SwizzleIndex<'r'> = 1;
		template<> constexpr std::size_t SwizzleIndex<'g'> = 2;
		template<> constexpr std::size_t SwizzleIndex<'b'> = 3;
		template<> constexpr std::size_t SwizzleIndex<'a'> = 4;

		template<> constexpr std::size_t SwizzleIndex<'x'> = 1;
		template<> constexpr std::size_t SwizzleIndex<'y'> = 2;
		template<> constexpr std::size_t SwizzleIndex<'z'> = 3;
		template<> constexpr std::size_t SwizzleIndex<'w'> = 4;

		template<> constexpr std::size_t SwizzleIndex<'s'> = 1;
		template<> constexpr std::size_t SwizzleIndex<'t'> = 2;
		template<> constexpr std::size_t SwizzleIndex<'p'> = 3;
		template<> constexpr std::size_t SwizzleIndex<'q'> = 4;
	}

	template<char... chars>
	class Swizzle
	{
	public:
		static constexpr bool Unique = swizzling::are_unique<chars...>;
		static constexpr std::size_t Size = sizeof...(chars);
		static constexpr std::size_t HighestComponent = std::max({ swizzling::SwizzleIndex<chars> ... });

		constexpr Swizzle() = default;

		template<char other_c>
		constexpr Swizzle<chars..., other_c> operator,(Swizzle<other_c>) const
		{
			return {};
		}

	private:
		static_assert(Size >= 1, "Minimum swizzling size is 1");
		static_assert(Size <= 4, "Maximum swizzling size is 4");

		using Sets = typename swizzling::MatchedSwizzleSets<chars...>::ValidSets;
		static_assert(Sets::Size == 1, "Swizzle sets cannot be mixed");
		using Set = typename Sets::template At<0>;
	};

	namespace swizzles
	{
		namespace rgba
		{
			constexpr Swizzle<'r'> r;
			constexpr Swizzle<'g'> g;
			constexpr Swizzle<'b'> b;
			constexpr Swizzle<'a'> a;
		}

		namespace xyzw
		{
			constexpr Swizzle<'x'> x;
			constexpr Swizzle<'y'> y;
			constexpr Swizzle<'z'> z;
			constexpr Swizzle<'w'> w;
		}

		namespace stpq
		{
			constexpr Swizzle<'s'> s;
			constexpr Swizzle<'t'> t;
			constexpr Swizzle<'p'> p;
			constexpr Swizzle<'q'> q;
		}

		namespace all
		{
			using namespace rgba;
			using namespace xyzw;
			using namespace stpq;
		}
	}
}