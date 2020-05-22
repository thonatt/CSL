#pragma once

#include <cstddef>
#include <type_traits>
namespace v2 {

	namespace detail_swizzling {

		enum class SwizzleStatus : std::size_t { Unique = 0, Repeated = 1 };

		template<char...>
		constexpr bool is_different_from = false;

		template<char c, char ...cs>
		constexpr bool is_different_from<c, cs...> = ((c != cs) && ...);

		template<char ...>
		constexpr bool are_unique = true;

		template<char c, char ...cs>
		constexpr bool are_unique<c, cs...> = is_different_from<c, cs...> && are_unique<cs...>;

		template<char... chars>
		struct CharList {};

		using SwizzleRGBA = CharList<'r', 'g', 'b', 'a'>;
		using SwizzleXYZW = CharList<'x', 'y', 'z', 'w'>;
		using SwizzleSTPQ = CharList<'s', 't', 'p', 'q'>;

		template<char c, typename T>
		constexpr bool is_in_set = false;

		template<char c, char... chars>
		constexpr bool is_in_set<c, CharList<chars...>> = ((c == chars) || ...);

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
	class Swizzle {

	private:
		using Set = detail_swizzling::SwizzleSet<c>;
		
		static_assert(sizeof...(chars) <= 3, "maximum swizzling size is 4");
		static_assert(!std::is_same_v<Set, std::false_type>, "invalid swizzle set");
		static_assert((std::is_same_v<Set, detail_swizzling::SwizzleSet<chars>>&& ...), "swizzle sets cant be mixed");

	public:
		static constexpr bool unique = detail_swizzling::are_unique<chars...>;

		constexpr Swizzle() { }

		template<char other_c>
		constexpr Swizzle<c, chars..., other_c>
			operator,(const Swizzle<other_c>& other) const
		{
			return { };
		}
	};

	namespace swizzles {

		namespace rgba {
			constexpr Swizzle<'r'> r;
			constexpr Swizzle<'g'> g;
			constexpr Swizzle<'b'> b;
			constexpr Swizzle<'a'> a;
		}

		namespace xyzw {
			constexpr Swizzle<'x'> x;
			constexpr Swizzle<'y'> y;
			constexpr Swizzle<'z'> z;
			constexpr Swizzle<'w'> w;
		}

		namespace stpq {
			constexpr Swizzle<'s'> s;
			constexpr Swizzle<'t'> t;
			constexpr Swizzle<'p'> p;
			constexpr Swizzle<'q'> q;
		}

		namespace all {
			using namespace rgba;
			using namespace xyzw;
			using namespace stpq;
		}

	}
}