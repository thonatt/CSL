//#include "tests.hpp"
//
//#include <csl/Core.hpp>
//#include <chrono>
//
//void test_old()
//{
//	using Clock = std::chrono::high_resolution_clock;
//
//	auto t0 = Clock::now();
//
//	using namespace csl::frag_430;
//	Shader shader;
//
//	CSL_STRUCT(Plop,
//		(vec3)v,
//		(Float)f
//	);
//
//	CSL_STRUCT(BigPlop,
//		(Plop)plop,
//		(Float)g
//	);
//
//	auto ffff = declareFunc<vec3>("f", [&](vec3 aa = "a", vec3 bb = "b") {
//		BigPlop b;
//		b.plop.v* BigPlop().plop.f;
//
//		CSL_IF(true) {
//			b.plop.v = 2.0 * b.plop.v;
//		} CSL_ELSE_IF(false) {
//			b.plop.v = 3.0 * b.plop.v;
//		} CSL_ELSE{
//			b.plop.v = 4.0 * b.plop.v;
//		}
//	});
//
//	using T = vec3;
//
//	Array<uniform<vec3>, 5> b;
//
//	uniform<sampler2D> s;
//
//	auto m = declareFunc<void>("main", [&]
//	{
//		using namespace csl::swizzles::xyzw;
//
//		mat3 m("m");
//		vec3 v("v");
//		Float f("f");
//
//		m* v;
//		f* m;
//		m* f;
//		f* v;
//		v* f;
//		m* m;
//		v* v;
//		f* f;
//
//		m + m;
//		v + v;
//		f + f;
//		m + f;
//		f + m;
//		v + f;
//		f + v;
//
//		m - m;
//		v - v;
//		f - f;
//		m - f;
//		f - m;
//		v - f;
//		f - v;
//
//		m / m;
//		v / v;
//		f / f;
//		m / f;
//		f / m;
//		v / f;
//		f / v;
//
//		(f * (v + v))[x, y, z];
//		(f + f * f)* (f * f + f);
//
//		greaterThan(cos(dFdx(f)), sin(dFdy(f)));
//
//		texture(s, v[x, y]);
//	});
//
//	auto t1 = Clock::now();
//
//	std::string str = shader.str();
//
//	auto t2 = Clock::now();
//
//	std::cout << "csl old test traversal : " << std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() / 1000.0 << " ms" << std::endl;
//	std::cout << "csl old string generation : " << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1000.0 << " ms" << std::endl;
//}
//
//
//void testSwitch()
//{
//	using namespace csl;
//	using namespace csl::frag_410;
//	using namespace csl::swizzles::all;
//
//	Uint i = Uint(0u) << "i";
//	CSL_SWITCH(i) {
//		CSL_CASE(1u) : {
//			++i;
//			CSL_FOR(Int j = 0; j < 5; ++j) {
//				i += 2;
//			}
//			CSL_IF(i > 6) {
//				CSL_SWITCH(j) {
//					CSL_CASE(12) : {
//						CSL_BREAK;
//					}
//				CSL_DEFAULT: {}
//				}
//			} CSL_ELSE{
//				--i;
//			}
//			CSL_BREAK;
//		}
//
//		CSL_CASE(2u) : { --i; }
//
//	CSL_DEFAULT: { i += 2; }
//	}
//
//	Bool bb("bb");
//	CSL_FOR(Int k = Int(0) << "k"; bb; ++k) {
//		k *= 2;
//	}
//}
//
//void testStructsMacros()
//{
//	using namespace csl;
//	using namespace csl::frag_450;
//	using namespace csl::swizzles::xyzw;
//
//	Shader shader;
//
//
//	using MyMats = Array<mat4, 12>;
//	CSL_STRUCT(InnerBlock,
//		(MyMats) mats
//	);
//
//	using MyArray = Array<InnerBlock, 12>;
//	CSL_INTERFACE_BLOCK(out<>, BasicBlock, basic, 7,
//		(vec3) myVec3,
//		(MyArray) myInnerBlocks,
//		(Array<uvec4>::Size<8>) uintsArray
//	);
//
//	Array<uniform<uvec4>, 8> myArray("myArray");
//	shader.main([&] {
//		Uint chan = Uint(0u) << "chan";
//		Uint uu = myArray[3][z];
//		chan = uu;
//
//		Uint uuu = basic[3].uintsArray[chan][z];
//		chan = uuu; 
//	
//		Array<Array<vec3, 2>, 2>  myQrray = { Array<vec3, 2>(vec3(0.0), vec3(1.0)),  Array<vec3, 2>(vec3(2.0), vec3(3.0)) };
//		myQrray[5][2] = vec3(1.0);
//		
//		basic[3].myInnerBlocks[3].mats[2][4][y] = 3.14;
//	});
//
//	std::cout << shader.str() << std::endl;
//}
//
//void testArgCleaning()
//{
//	using namespace csl;
//	using namespace csl::core;
//
//	using namespace csl::frag_430;
//	using namespace csl::swizzles::xyzw;
//	Shader shader;
//
//	std::is_same<ArithmeticBinaryReturnType<vec2&, vec2&>, vec2>::value;
//
//	auto g = [](vec3 a, vec3 b = "b") {
//		a += b;
//	};
//
//	call_with_only_non_default_args(g);
//
//	//LambdaInfos<decltype(g)>::ArgTup;
//
//	//call_f_all_args_empty(g);
//	//call_f_non_default_args_empty(g);
//
//	auto add = declareFunc<vec3, mat4, void>("add",
//		[](vec3 a = "a", vec3 b = "b") { CSL_RETURN(a + b); },
//		[](mat4 a, mat4 b = "chosen", mat4 c = "") { CSL_RETURN(a + b + c); },
//		[] {}
//	);
//
//	auto nulll = declareFunc<void>(
//		[]{ CSL_RETURN; }
//	);
//
//	//auto notenough = declareFunc<>("notenough");
//
//	//vec3 r1 = add(vec3(0), vec3(1));
//	//mat4 r2 = add(mat4(0), mat4(1), mat4(2));
//	//r1 = (r2*vec4(r1, 1.0))[x, y, z];
//	//
//	std::cout << shader.str() << std::endl;
//
//	//auto ff = [](vec3 a = "a", vec3 b = "b") { CSL_RETURN(a + b); };
//	//auto gg = [](mat4 a, mat4 b) { CSL_RETURN(a + b); };
//
//
//	//using F = decltype(ff);
//	//using G = decltype(gg);
//
//	//std::stringstream ss;
//	//int t = 0;
//
//	////FDeclImpl<TList<vec3, mat4>, 0, F, G>::str(ss, t, 0, "add", {});
//
//	//std::cout << ss.str() << std::endl;
//
//	//OverloadResolution<TList<F, G>, TList<ivec3, ivec3>, 0 >::value;
//	//using R = OverloadResolutionType<TList<vec3, ivec3>, TList<F, G>, TList<vec3, vec3>>;
//	//std::is_same<R, vec3>::value;
//
//
//	//decltype((void)std::declval<F>() (std::declval<std::tuple_element_t<Is, T>>()...)
//
//	//static_assert(getMinNumArgs<decltype(f), int, int, int>() == 1, "!" );
//
//	//auto tup = createTuple<int, double>(0, 2.0);
//	
//	//using T = typename GetStdFunc<decltype(decltype(f)::operator())>::Type;
//	//std::is_same_v<T, std::function<void(int, int, int)>>;
//
//	//f({}, {}, {});
//
//	//Uint counter = 0;
//	//vec2 uv = vec2(1.0, 2.0);
//	//vec4 color = vec4(0.0, uv, Float(counter));
//	//bvec3 m = bvec3(true, false, false);
//
//	//using namespace csl::swizzles::xyzw;
//	//using namespace csl::swizzles::rgba;
//	//using namespace csl::swizzles::stpq;
//
//	//using namespace csl::swizzles::all;
//
//	//TypeMerger< TypeList<float>, TypeList<>, TypeList<double> >::Type;
//	//TypeMerger< TypeList<>, TypeList<>, TypeList<double>, TypeList<>, TypeList<float> >::Type;
//}
//
//void testInArgs()
//{
//	using namespace csl::frag_430;
//
//	
//	Shader shader;
//
//	declareFunc<void>([](
//		in<vec3> in = "in", out<vec3> out = "out" ) {
//	});
//
//	std::cout << shader.str() << std::endl;
//}
//
//template<typename A, typename B>
//constexpr auto validMultiplication(int) -> decltype(A()* B(), true) { return true; }
//
//template<typename A, typename B>
//constexpr auto validMultiplication(long) { return false; }
//
//template<typename T>
//constexpr auto validInverse(int) -> decltype(csl::core::glsl_140::inverse(T()), true) { return true; }
//
//template<typename T>
//constexpr auto validInverse(long) { return false; }
//
//void testsCompliance()
//{
//	static_assert(validMultiplication<csl::mat4, csl::vec4>(0), "bouh");
//	static_assert(validMultiplication<csl::vec3, csl::vec3>(0), "bouh");
//	static_assert(validMultiplication<csl::Float, csl::vec3>(0), "bouh");
//
//	static_assert(!validMultiplication<csl::mat4, csl::vec3>(0), "bouh");
//	static_assert(!validMultiplication<csl::vec2, csl::mat4x3>(0), "bouh");
//
//	static_assert(validInverse<csl::mat4>(0), "bouh");
//	static_assert(!validInverse<csl::mat4x2>(0), "bouh");
//	static_assert(!validInverse<csl::vec3>(0), "bouh");
//}
//
//void testLzist()
//{
//
//	using namespace csl;
//
//	//CSL_BLOCK((a, b) c, d);
//
//	//CSL_BLOCK(() c, d);
//
//	//CSL_PP_ADD_COMMA(() a )
//}
//
//void testArgsOrder()
//{
//
//	//getArgOrderStr();
//
//	//auto e = [](Arg i = 0, Arg d = 1, Arg k = 2) {
//	//};
//
//	//auto f = [&](Arg i = 0, Arg d = 1) {
//	//	e(i, d);
//	//};
//
//	//auto sf = [](Arg i, Arg d = 1) {
//	//};
//	//
//	//auto g = [&](Arg i = 0) {
//	//	f(i);
//	//};
//
//	//auto ff = [&](auto&&... args) -> decltype(auto) { 
//	//	printArgs(args...);
//	//	return f(std::forward<decltype(args)>(args)...); 
//	//};
//
//	//f();
//	//ff();
//	//g();
//
//	//h();
//	//j();
//
//	//curry(f);
//
//}
