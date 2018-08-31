#pragma once
#include "Context.h"

enum ShaderType { VERTEX, FRAGMENT };
enum GLSLversion { _450 = 450 };

class ShaderBase {
public:
	friend BlockBase;
	friend Context;

	ShaderBase(GLSLversion _version = _450) {
		Ctx().setShader(this);
		version = _version;
		mainFun = [] {};
	}

	void main(const std::function<void(void)> & _main) {		
		mainFun = _main;
	}

	const std::string & getStr() {
		
		if (!shaderEnded) {
			Ctx().endShader();
			shaderString = "#version " + std::to_string((int)version) + "\n \n";

			for (const auto & s : structDeclarations) {
				shaderString += s + "\n";
			}

			for (const auto & line : lines) {
				for (int i = 0; i < line.offset; ++i) {
					shaderString += "  ";
				}
				shaderString += line.cmd + "\n";
			}
			shaderEnded = true;
		}
	
		return shaderString;
	}

protected:
	std::vector<Line> lines;
	std::vector<std::string> structDeclarations;
	std::string shaderString;
	std::function<void(void)> mainFun;
	GLSLversion version;
	bool shaderEnded = false;
};

std::vector<Line> & BlockBase::getLines() { return currentShader->lines; }

void Context::endShader() {
	begin_function_declaration<void>("main");
	currentShader->mainFun();
	end_function_declaration();
}

void Context::addStruct(const std::string & s) {
	currentShader->structDeclarations.push_back(s);
}

#define GLSL_BUILT_IN(name,storage,type) storage<type> name = storage<type>(#name);

const Bool True("true"), False("false");

namespace fs {
	class FragmentShader : public ShaderBase {
	public:
		FragmentShader(GLSLversion _version = _450) : ShaderBase(_version) {}
	};

	GLSL_BUILT_IN(gl_FragCoord, In, vec4);
	GLSL_BUILT_IN(gl_FragDepth, Out, Float);

}

namespace vs {
	class VertexShader : public ShaderBase { 
	public:
		VertexShader(GLSLversion _version = _450) : ShaderBase(_version) {}
	};

	GLSL_BUILT_IN(gl_Position, In, vec4);
	GLSL_BUILT_IN(gl_VertexID, In, Int);

}

#undef GLSL_FRAG_BUILT_IN
