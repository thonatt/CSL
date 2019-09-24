#pragma once

#include "StringHelpers.hpp"

namespace csl {

	enum SwizzleSet { RGBA, XYZW, STPQ, MIXED_SET };
	enum SwizzeStatus : uint { NON_REPEATED = 0, REPEATED = 1 };

	template<SwizzleSet Set, uint Dim, uint Bytes = (1 << (Dim-1)), uint Size = 1, SwizzeStatus Status = NON_REPEATED>
	class SwizzlePack;

	template<SwizzleSet S1, SwizzleSet S2, uint D1, uint D2, uint B1, uint B2, uint Size, SwizzeStatus status>
	using OutSwizzle = SwizzlePack<
		(S1 == S2 ? S1 : MIXED_SET),
		MaxUint<D1, D2>,
		(B1 | B2),
		Size + 1,
		(status == REPEATED || (B1 & B2) != 0) ? REPEATED : NON_REPEATED
	>;

	template<SwizzleSet Set, uint Dim, uint Bytes, uint Size, SwizzeStatus Status>
	class SwizzlePack {
	public:
		SwizzlePack(const std::string & _s) : s(std::make_shared<std::string>(_s)) { }

		template<SwizzleSet _Set, uint _Dim, uint _Bytes>
		OutSwizzle<Set, _Set, Dim, _Dim, Bytes, _Bytes, Size, Status>
			operator,(const SwizzlePack<_Set, _Dim, _Bytes, 1> & other) const
		{
			return { *s + *other.s };
		}

		const stringPtr & getStrPtr() const {
			return s;
		}

	public:
		stringPtr s;
	};

	namespace swizzles {

		namespace rgba {
			const SwizzlePack<RGBA, 1> r("r");
			const SwizzlePack<RGBA, 2> g("g");
			const SwizzlePack<RGBA, 3> b("b");
			const SwizzlePack<RGBA, 4> a("a");
		}

		namespace xyzw {
			const SwizzlePack<XYZW, 1> x("x");
			const SwizzlePack<XYZW, 2> y("y");
			const SwizzlePack<XYZW, 3> z("z");
			const SwizzlePack<XYZW, 4> w("w");
		}

		namespace stpq {
			const SwizzlePack<STPQ, 1> s("s");
			const SwizzlePack<STPQ, 2> t("t");
			const SwizzlePack<STPQ, 3> p("p");
			const SwizzlePack<STPQ, 4> q("q");
		}

		namespace all {
			using namespace rgba;
			using namespace xyzw;
			using namespace stpq;
		}
	}


} //namespace csl
