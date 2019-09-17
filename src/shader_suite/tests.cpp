#include "tests.h"

#include <csl/Core.hpp>

void testSwitch()
{
	using namespace csl;
	using namespace csl::frag_410;
	using namespace csl::swizzles::all;

	Uint i = Uint(0u) << "i";
	GL_SWITCH(i) {
		GL_CASE(1u) : {
			++i;
			GL_FOR(Int j = 0; j < 5; ++j) {
				i += 2;
			}
			GL_IF(i > 6) {
				GL_SWITCH(j) {
					GL_CASE(12) : {
						GL_BREAK;
					}
				GL_DEFAULT: {}
				}
			} GL_ELSE{
				--i;
			}
			GL_BREAK;
		}

		GL_CASE(2u) : { --i; }

	GL_DEFAULT: { i += 2; }
	}

	Bool bb("bb");
	GL_FOR(Int k = Int(0) << "k"; bb; ++k) {
		k *= 2;
	}
}

void testStructsMacros()
{
	using namespace csl;
	using namespace csl::frag_450;
	using namespace csl::swizzles::xyzw;

	Shader shader;


	using MyMats = Array<mat4, 12>;
	GL_STRUCT(InnerBlock,
		(MyMats) mats
	);

	using MyArray = Array<InnerBlock, 12>;
	GL_INTERFACE_BLOCK(Out<>, BasicBlock, basic, 7,
		(vec3) myVec3,
		(MyArray) myInnerBlocks,
		(Array<uvec4>::Size<8>) uintsArray
	);

	Array<Uniform<uvec4>, 8> myArray("myArray");
	shader.main([&] {
		Uint chan = Uint(0u) << "chan";
		Uint uu = myArray[3][z];
		chan = uu;

		Uint uuu = basic[3].uintsArray[chan][z];
		chan = uuu; 
	
		Array<Array<vec3, 2>, 2>  myQrray = { Array<vec3, 2>(vec3(0.0), vec3(1.0)),  Array<vec3, 2>(vec3(2.0), vec3(3.0)) };
		myQrray[5][2] = vec3(1.0);
		
		basic[3].myInnerBlocks[3].mats[2][4][y] = 3.14;
	});

	std::cout << shader.str() << std::endl;
}

void testArgCleaning()
{
	using namespace csl;

	using namespace csl::frag_430;
	using namespace csl::swizzles::xyzw;
	Shader shader;

	std::is_same<ArithmeticBinaryReturnType<vec2&, vec2&>, vec2>::value;

	auto g = [](vec3 a, vec3 b = "b") {
		a += b;
	};

	call_with_only_non_default_args(g);

	//LambdaInfos<decltype(g)>::ArgTup;

	//call_f_all_args_empty(g);
	//call_f_non_default_args_empty(g);

	auto add = declareFunc<vec3, mat4, void>("add",
		[](vec3 a = "a", vec3 b = "b") { GL_RETURN(a + b); },
		[](mat4 a, mat4 b = "chosen", mat4 c = "") { GL_RETURN(a + b + c); },
		[] {}
	);

	auto nulll = declareFunc<void>(
		[]{ GL_RETURN; }
	);

	//auto notenough = declareFunc<>("notenough");

	//vec3 r1 = add(vec3(0), vec3(1));
	//mat4 r2 = add(mat4(0), mat4(1), mat4(2));
	//r1 = (r2*vec4(r1, 1.0))[x, y, z];
	//
	std::cout << shader.str() << std::endl;

	//auto ff = [](vec3 a = "a", vec3 b = "b") { GL_RETURN(a + b); };
	//auto gg = [](mat4 a, mat4 b) { GL_RETURN(a + b); };


	//using F = decltype(ff);
	//using G = decltype(gg);

	//std::stringstream ss;
	//int t = 0;

	////FDeclImpl<TList<vec3, mat4>, 0, F, G>::str(ss, t, 0, "add", {});

	//std::cout << ss.str() << std::endl;

	//OverloadResolution<TList<F, G>, TList<ivec3, ivec3>, 0 >::value;
	//using R = OverloadResolutionType<TList<vec3, ivec3>, TList<F, G>, TList<vec3, vec3>>;
	//std::is_same<R, vec3>::value;


	//decltype((void)std::declval<F>() (std::declval<std::tuple_element_t<Is, T>>()...)

	//static_assert(getMinNumArgs<decltype(f), int, int, int>() == 1, "!" );

	//auto tup = createTuple<int, double>(0, 2.0);
	
	//using T = typename GetStdFunc<decltype(decltype(f)::operator())>::Type;
	//std::is_same_v<T, std::function<void(int, int, int)>>;

	//f({}, {}, {});

	//Uint counter = 0;
	//vec2 uv = vec2(1.0, 2.0);
	//vec4 color = vec4(0.0, uv, Float(counter));
	//bvec3 m = bvec3(true, false, false);

	//using namespace csl::swizzles::xyzw;
	//using namespace csl::swizzles::rgba;
	//using namespace csl::swizzles::stpq;

	//using namespace csl::swizzles::all;

	//TypeMerger< TypeList<float>, TypeList<>, TypeList<double> >::Type;
	//TypeMerger< TypeList<>, TypeList<>, TypeList<double>, TypeList<>, TypeList<float> >::Type;
}



void testArgsOrder()
{

	//getArgOrderStr();

	//auto e = [](Arg i = 0, Arg d = 1, Arg k = 2) {
	//};

	//auto f = [&](Arg i = 0, Arg d = 1) {
	//	e(i, d);
	//};

	//auto sf = [](Arg i, Arg d = 1) {
	//};
	//
	//auto g = [&](Arg i = 0) {
	//	f(i);
	//};

	//auto ff = [&](auto&&... args) -> decltype(auto) { 
	//	printArgs(args...);
	//	return f(std::forward<decltype(args)>(args)...); 
	//};

	//f();
	//ff();
	//g();

	//h();
	//j();

	//curry(f);

}
