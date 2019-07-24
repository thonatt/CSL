# C++ integrated Shading Language

CSL is a C++ header-only library for writing OpenGL shaders directly inside computer graphics applications code. The concept is that shader correctness is checked at compile-time while a string corresponding to the GLSL code is produced at run-time. CSL aims to provide convenient and maintainable shader writing thanks to :

+ Having a syntax as close as possible to GLSL.
+ Checking GLSL specification compliance at compile-time as much as possible.
+ The possibility to use C++ as meta language for clean shader generation.

By default, CSL does not require any dependency as it relies on the STL and some Boost Preprocessor files that are present in the repo. *Optionnaly*, it is possible to only clone `\src\core\` if the Boost Preprocessor is already available from elsewhere.

CSL also provides a [shader suite](https://github.com/thonatt/CSL/tree/master/src/shader_suite) which includes several shaders, from didactic examples to shaders used in complex graphics applications.

# How to

As CSL is a header-only library, a simple include of the file `<csl\core.hpp>` is enough to use it. Here is a simple vertex shader example, with its corresponding output.

<details>
    <summary>Show example</summary>
<table>
  <tr>
    <th>Code</th>
    <th>Output</th> 
  </tr>
  <tr>
    <td>
        
  ```cpp
#include <csl/core.hpp>
#include <iostream>

int main() {
      using namespace csl::vert_330;  
      Shader shader;

      In<vec3, Layout<Location<0>> position;

      shader.main([&]{
            gl_Position = vec4(position, 1.0);
      });

      std::cout << shader.str() << std::endl;
}
```
   </td>
    <td>
  
```cpp
   #version 330

   layout(location = 0) in vec3 position;

   void main()
   {
      gl_Position = vec4(position, 1.0);
   }
```
   </td> 
  </tr>
</table>
</details>

For readability purposes, all examples assume named variables. See the [naming variables](#naming-variables) section for more details.

## CSL syntax

+ [Shader setup](#shader-setup)
+ [Basic and Sampler types](#basic-and-sampler-types)
+ [Naming variables](#naming-variables)
+ [Operators and Swizzles](#operators-and-swizzles)
+ [Memory and Layout qualifiers](#memory-and-layout-qualifiers)
+ [Arrays and Functions](#arrays-and-functions)
+ [Code structure](#code-structure)
+ [Structs and Interface blocks](#structs-and-interface-blocks)

### Shader setup

Shader type and GLSL version are setup using the corresponding namespace. For example, `using namespace csl::vert_330` gives access to the built-in functions and built-in variables for a vertex shader with GLSL 3.30. Starting a new shader requires to create a novel variable of type `Shader`. This type contains two important member functions. The first one is `Shader::main` which allows to setup the main using a lambda function as argument. The second one is `Shader::str`which retrieves the `std::string` associated to the shader.

### Basic and Sampler types

CSL defines the types used in GLSL. Most CSL types have the exact same typename as their GLSL counterpart. For example, `vec3`, `ivec2`, `dmat4`, `mat4x3`, `sampler2DArray`, `uimageCube` are all valid types that can be used as is. The only exceptions are `double`, `float`, `int` and `bool` as they would conflict with C++ keywords. Valid associated CSL typenames are `Double`, `Float`, `Int`, `Bool` - and `Uint` to keep the consistency.

Constructors and declarations are identical to GLSL. CSL and C++ basic types can also be mixed.

<details>
    <summary>Show example</summary>
<table>
  <tr>
    <th>Code</th>
    <th>Output</th> 
  </tr>
  <tr>
    <td>
        
  ```cpp
    Uint counter = 0;
    vec2 uv = vec2(1.0, 2.0);
    vec4 color = vec4(0.0, uv, Float(counter));
    bvec3 m = bvec3(true, false, false); 
```
</td>
    <td>
  
```cpp
	uint counter = 0;
	vec2 uv = vec2(1.0, 2.0);
	vec4 color = vec4(0.0, uv, float(counter));
	bvec3 m = bvec3(true, false, false);
```
</td> 
  </tr>
</table>
</details>

### Naming variables

Because C++ objects do not have names, it is not possible to forward directly the CSL variable names to the GLSL output. As a consequence, CSL will perfom automatic variable naming. **It has no effect when actually compiling the shader on the GPU**. Still, it has a big impact on the shader readability.

<details>
    <summary>Automatic naming example</summary>
<table>
  <tr>
    <th>Code with automatic naming</th>
    <th>Output</th> 
  </tr>
  <tr>
    <td>
        
  ```cpp
In<vec3> normal;
In<vec3> position;
Uniform<vec3> eye;

shader.main([&] {
    Float alpha = 1.2;
    vec3 V = normalize(eye - position);
    vec3 N = normalize(normal);

    Float result;
    result = alpha * dot(N, V);
});
```
</td>
    <td>
  
```cpp
   in vec3 vec3_0;
   in vec3 vec3_1;
   uniform vec3 vec3_2;

   void main()
   {
      float float_0 = 1.2;
      vec3 vec3_4 = normalize(vec3_2 - vec3_1);
      vec3 vec3_5 = normalize(vec3_0);
      float float_1;
      float_1 = float_0*dot(vec3_5, vec3_4);
   }
```
</td> 
  </tr>
</table>
</details>

Therefore, it is possible, yet **completely optionnal**, to provide a name to any CSL variable. It can be done either when declaring a variable using the `(const std::string &)` constructor, or when initializing a variable using the `<<(const std::string &)` operator. Such manual naming is rather cumbersome, but it is sometimes usefull to name locally variables when debugging.

<details>
    <summary>Manual naming example</summary>
<table>
  <tr>
    <th>Code with manual naming</th>
    <th>Output</th> 
  </tr>
  <tr>
    <td>
        
  ```cpp
//naming during variable declaration
In<vec3> normal("normal");
In<vec3> position("position");
Uniform<vec3> eye("eye");

shader.main([&] {
    //naming during variable initialisation
    Float alpha = Float(1.2) << "alpha";
    vec3 V = eye - position << "V";
    vec3 N = normalize(normal) << "N";

    //naming during variable declaration
    Float result("result");
    result = alpha * dot(N, V);
});
```
</td>
    <td>
  
```cpp
in vec3 normal;
in vec3 position;
uniform vec3 eye;

void main()
{
  float alpha = 1.2;
  vec3 V = eye - position;
  vec3 N = normalize(normal);
  float result;
  result = alpha*dot(N, V);
}
```
</td> 
  </tr>
</table>
</details>

### Operators and Swizzles

As C++ and GLSL share a common C base syntax, most of the operators keywords are identical and can be used as is. This includes for example:
+ `+`, `-`, `*`, `/` and their assignment operator counterparts,
+ `==`, `<`, `>` , `&&` and other binary relational or bitwise operators,
+ `[int]` for component or row access

One exception is the ternary operator ` ? : `. Even if the synthax is identical between C++ and GLSL, it cannot be overloaded. Therefore it is replaced by a global function `ternary` with the 3 arguments.

Swizzles are GLSL-specific operators for verstatile vector components acces. In order to preserve all the swizzling possibilities while keeping the code simple, CSL uses global variables such as `x` `y` `z` or `w`. The syntax for swizzle accessing is for example `myVec[x,z,x];`. To prevent namespace pollution, each of these swizzle variable belongs to a specific namespace corresponding to its swizzle set. Available namespaces are `csl::swizzles::xyzw`, `csl::swizzles::rgba`, `csl::swizzles::stpq` and `csl::swizzles::all` which includes the previous three.

<details>
    <summary>Swizzle examples</summary>
<table>
  <tr>
    <th>Code</th>
    <th>Output</th> 
  </tr>
  <tr>
    <td>
        
  ```cpp
using namespace csl::swizzles::rgba;
vec4 col;
vec4 out;
out[b, g, r] = col[r, g, b];

//can you guess what is actually assigned ?
out[a] = col[b, a, r][b, g][g];
```
</td>
    <td>
  
```cpp
   vec4 col;
   vec4 out;
   out.bgr = col.rgb;
   out.a = col.bar.bg.g;

```
</td> 
  </tr>
</table>
</details>

### Memory and Layout qualifiers


### Arrays and Functions

Arrays in CSL are a template class with the internal type and the size as parameters. Unspecified or zero size are used for implicitely sized GLSL arrays. Indexing is done with the usual `[]` operator. Multi dimensional arrays are supported as nested arrays. 

<details>
    <summary>Array examples</summary>
<table>
  <tr>
    <th>Code</th>
    <th>Output</th> 
  </tr>
  <tr>
    <td>
        
  ```cpp
//array declaration with size
Array<vec3, 5> vec3A("myVec3A");

//unspecified array initialisation
Array<Float> floatA = Array<Float>(0.0, 1.0, 2.0) << "floatA";

//multi dimensionnal array
Array<Array<mat3, 2>, 2> matA = Array<Array<mat3, 2>, 2>(
	Array<mat3, 2>(mat3(0), mat3(1)),
	Array<mat3, 2>(mat3(2), mat3(3))
	) << "matA";

//array accessors
vec3A[0] = floatA[1] * matA[0][0]* vec3A[1];
```
</td>
    <td>
  
```cpp
vec3 myVec3A[5];
float floatA[] = float[](0.0, 1.0, 2.0);
mat3 matA[2][2] = mat3[2][2](mat3[2](mat3(0), mat3(1)), mat3[2](mat3(2), mat3(3)));
myVec3A[0] = floatA[1]*matA[0][0]*myVec3A[1];

```
</td> 
  </tr>
</table>
</details>

Functions in CSL are objects that can be created using the `makeFunc` template function. The return type must be explicitely specified as template parameter. The function can then be called later in the code using the usual `()` operator. Functions overloading in not possible in CSL.

<details>
    <summary>Function examples</summary>
<table>
  <tr>
    <th>Code</th>
    <th>Output</th> 
  </tr>
  <tr>
    <td>
        
  ```cpp
	//empty function
	auto fun = makeFunc<void>([]() {
		GL_RETURN;
	});

	//named function with named parameters
	auto add = makeFunc<vec3>("add", [](vec3 a, vec3 b) {
		GL_RETURN(a + b);
	}, "a", "b");

	//function with some named parameters
	auto addI = makeFunc<Int>([](Int a, Int b) {
		GL_RETURN(a + b);
	}, "a");

	//function calling another function
	auto sub = makeFunc<vec3>([&](vec3 a, vec3 b) {
		GL_RETURN(add(a, -b));
	});
```
</td>
    <td>
  
```cpp
   void function_0()
   {
      return;
   }

   vec3 add(vec3 a, vec3 b)
   {
      return a + b;
   }

   int function_1(int a, int int_0)
   {
      return a + int_0;
   }

   vec3 function_2(vec3 vec3_2, vec3 vec3_1)
   {
      return add(vec3_2, -vec3_1);
   }

```
</td> 
  </tr>
</table>
</details>

### Code structure

Selection, iteration and jump statements are available in CSL. As C++ and GLSL share the same keywords, CSL redefines them using macros with the syntax `GL_KEYWORD`. Their behavior is mostly identical to to C++ and GLSL. Here are the few limitations:
+ A `GL_SWITCH` **must** contain a `GL_DEFAULT` case.
+ Syntax for `case value :` is `GL_CASE(value) :`.
+ Condition and loop in `GL_FOR( init-expression; condition-expression; loop-expression)` must not contain more than one statement.

### Structs and Interface blocks


### Built-in functions and variables



