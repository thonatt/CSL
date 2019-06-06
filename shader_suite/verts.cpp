#include "verts.h"

#include "../Shaders.h"

std::string testVert()
{
	using namespace all_swizzles;
	using namespace vert_330;

	Shader shader;

	//GL_DISCARD;

	return shader.str();
}
