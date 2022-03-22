#pragma once

#include <algorithm>
#include <cstddef>
#include <type_traits>

namespace csl
{
	namespace swizzling
	{
		template<char...>
		constexpr bool is_different_from = false;

		template<char c, char ...cs>
		constexpr bool is_different_from<c, cs...> = ((c != cs) && ...);

		template<char ...>
		constexpr bool are_unique = true;

		template<char c, char ...cs>
		constexpr bool are_unique<c, cs...> = is_different_from<c, cs...> && are_unique<cs...>;

		template<char... chars>
		struct CharSeq : std::true_type {};

		using SwizzleRGBA = CharSeq<'r', 'g', 'b', 'a'>;
		using SwizzleXYZW = CharSeq<'x', 'y', 'z', 'w'>;
		using SwizzleSTPQ = CharSeq<'s', 't', 'p', 'q'>;

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

		template<char ... chars>
		constexpr std::size_t HighestComponent = std::max({ SwizzleIndex<chars> ... });

		template<char c, typename T>
		constexpr bool is_in_set = false;

		template<char c, char... chars>
		constexpr bool is_in_set<c, CharSeq<chars...>> = ((c == chars) || ...);

		template<char c, typename ...Sets>
		struct SwizzleGetter { using Type = std::false_type; };

		template<char c, typename Set, typename ...Sets>
		struct SwizzleGetter<c, Set, Sets...> {
			using Type = typename std::conditional_t< is_in_set<c, Set>, Set, typename SwizzleGetter<c, Sets...>::Type >;
		};

		template<char c>
		using SwizzleSet = typename SwizzleGetter<c, SwizzleRGBA, SwizzleXYZW, SwizzleSTPQ>::Type;
	}

	template<char c, char... chars>
	class Swizzle
	{
	public:
		static constexpr bool Unique = swizzling::are_unique<c, chars...>;
		static constexpr std::size_t Size = 1 + sizeof...(chars);
		static constexpr std::size_t HighestComponent = swizzling::HighestComponent<c, chars...>;

		constexpr Swizzle() = default;

		template<char other_c>
		constexpr Swizzle<c, chars..., other_c> operator,(Swizzle<other_c>) const
		{
			return {};
		}

	private:
		using Set = swizzling::SwizzleSet<c>;

		static_assert(Size <= 4, "Maximum swizzling size is 4");
		static_assert(Set(), "Invalid swizzle set");
		static_assert(((std::is_same_v<Set, swizzling::SwizzleSet<chars>>)&& ...), "Swizzle sets cannot be mixed");
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