#include <shaders/80_s_shader.h>
#include <shaders/dolphin.h>
#include <shaders/rendu_compilation.h>
#include <shaders/ogl4_sl_cookbook.h>
#include <shaders/examples.h>

#include "tests.h"

#include <iostream>
#include <chrono>

void printExamples() {
	arrays_example();
	swizzling_example();
	auto_naming_example();
	qualifier_example();
	functions_example();
	structure_stratements_example();
	structs_examples();
	interface_examples();
	struct_interface_comma_examples();
}

int main()
{
	//test_accessor();
	
	//testArgCleaning();
	//testStructsMacros();
	//testArgsOrder();

	printExamples();

	auto start = std::chrono::steady_clock::now();

	//auto blur_str = blurShader();

	//auto ambiant_str = ambiantShader();

	//auto ssao_str = ssaoShader();

	//auto discard_str = discardFrag();

	//auto transfeedBack_str = transfeedBackVertex();
	
	//auto eightiesShader_str = eightiesShader();

	//auto dolphin_vertex_str = dolphinVertex();

	//auto dolphin_frag_str = dolphinFragment();

	//auto phongShading_str = phongShading();

	auto per_tri_normal_str = per_triangle_normal_geom();

	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start);


	//std::cout << blur_str;
	//std::cout << ambiant_str;
	//std::cout << ssao_str;
	//std::cout << discard_str;
	//std::cout << transfeedBack_str;
	//std::cout << eightiesShader_str;
	//std::cout << dolphin_vertex_str;
	//std::cout << dolphin_frag_str;
	//std::cout << phongShading_str;
	std::cout << per_tri_normal_str;

	std::cout << "elapsed time : " << duration.count() << std::endl;
	
	return 0;
}
