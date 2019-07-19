#include "tests.h"

#include <csl/Shaders.h>

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
		(GetArray<uvec4>::Size<8>) uintsArray
	);

	Array<Uniform<uvec4>, 8> myArray("myArray");
	shader.main([&] {
		Uint chan = Uint(0u) << "chan";
		Uint uu = myArray[3][z];
		chan = uu;

		Uint uuu = basic[3].uintsArray[chan][z];
		chan = uuu; 
	
		
		basic[3].myInnerBlocks[3].mats[2][4][y] = 3.14;
	});

	std::cout << shader.str() << std::endl;
}

void testArgCleaning()
{
	using namespace csl;

	//TypeMerger< TypeList<float>, TypeList<>, TypeList<double> >::Type;
	//TypeMerger< TypeList<>, TypeList<>, TypeList<double>, TypeList<>, TypeList<float> >::Type;
}
