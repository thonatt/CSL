#pragma once

#include "StringHelpers.h"

enum SwizzleSet { RGBA, XYZW, STPQ, MIXED_SET };

template<uint Dim, uint Size, SwizzleSet Set, uint Bytes, bool Repeated>
class SwizzlePack {
public:
	SwizzlePack(const std::string & c) : s(std::make_shared<std::string>(c)) { }

	template<uint _Dim, SwizzleSet _Set, uint _Bytes,
		typename ReturnPack = SwizzlePack<
		MaxUint<Dim, _Dim>,
		Size + 1,
		Set == _Set ? Set : MIXED_SET,
		Bytes ^ _Bytes,
		Repeated || (Bytes & _Bytes) != 0
	> >
		ReturnPack operator,(const SwizzlePack<_Dim, 1, _Set, _Bytes, false> & other) const {
		return ReturnPack(*s + *other.s);
	}

public:
	stringPtr s;
};

namespace rgba {
	const SwizzlePack<1, 1, RGBA, 1, false> r("r");
	const SwizzlePack<2, 1, RGBA, 2, false> g("g");
	const SwizzlePack<3, 1, RGBA, 4, false> b("b");
	const SwizzlePack<4, 1, RGBA, 8, false> a("a");
}

namespace xywz {
	const SwizzlePack<1, 1, XYZW, 1, false> x("x");
	const SwizzlePack<2, 1, XYZW, 2, false> y("y");
	const SwizzlePack<3, 1, XYZW, 4, false> z("z");
	const SwizzlePack<4, 1, XYZW, 8, false> w("w");
}

namespace stpq {
	const SwizzlePack<1, 1, STPQ, 1, false> s("s");
	const SwizzlePack<2, 1, STPQ, 2, false> t("t");
	const SwizzlePack<3, 1, STPQ, 4, false> p("p");
	const SwizzlePack<4, 1, STPQ, 8, false> q("q");
}

namespace all_swizzles {
	using namespace rgba;
	using namespace xywz;
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
