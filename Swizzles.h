#pragma once

#include "ExpressionsTest.h"

enum SwizzleSet { RGBA, XYZW, STPQ, MIXED_SET };
enum SwizzeStatus : uint { NON_REPEATED = 0, REPEATED = 1 };

template<uint Dim, SwizzleSet Set, uint Bytes, uint Size = 1, SwizzeStatus Status = NON_REPEATED>
class SwizzlePack;

template<uint D1, uint D2, SwizzleSet S1, SwizzleSet S2, uint B1, uint B2, uint Size, SwizzeStatus status>
using OutSwizzle = SwizzlePack<
	MaxUint<D1, D2>, S1 == S2 ? S1 : MIXED_SET, (B1 | B2), Size + 1, ((status == REPEATED || (B1 & B2) != 0) ? REPEATED : NON_REPEATED) >;

template<uint Dim, SwizzleSet Set, uint Bytes, uint Size, SwizzeStatus Status>
class SwizzlePack {
public:
	SwizzlePack(const std::string & _s) : s(std::make_shared<std::string>(_s)) { }

	template<uint Dim_, SwizzleSet Set_, uint Bytes_>
	OutSwizzle<Dim, Dim_, Set, Set_, Bytes, Bytes_, Size, Status> 
	operator,(const SwizzlePack<Dim_, Set_, Bytes_, 1> & other) const
	{
		return { *s + *other.s };
	}

	const stringPtr & getStrPtr() const {
		return s;
	}

public:
	stringPtr s;
};

namespace rgba {
	const SwizzlePack<1, RGBA, 1> r("r");
	const SwizzlePack<2, RGBA, 2> g("g");
	const SwizzlePack<3, RGBA, 4> b("b");
	const SwizzlePack<4, RGBA, 8> a("a");
}

namespace xyzw {
	const SwizzlePack<1, XYZW, 1> x("x");
	const SwizzlePack<2, XYZW, 2> y("y");
	const SwizzlePack<3, XYZW, 4> z("z");
	const SwizzlePack<4, XYZW, 8> w("w");
}

namespace stpq {
	const SwizzlePack<1, STPQ, 1> s("s");
	const SwizzlePack<2, STPQ, 2> t("t");
	const SwizzlePack<3, STPQ, 4> p("p");
	const SwizzlePack<4, STPQ, 8> q("q");
}

namespace all_swizzles {
	using namespace rgba;
	using namespace xyzw;
	using namespace stpq;
}

namespace v_0 {
	void f_0() {}
}

namespace v_1 {
	using namespace v_0;
	void f_1() {}
}

namespace v_2 {
	using namespace v_1;
	void f_1(mat2 &) {}
	void f_2(Float &) {}
}

void test_namespaces() {
	using namespace v_2;
	f_1();
	
}
