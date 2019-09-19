# C++ integrated Shading Language

CSL is a C++ header-only library, self-transpiling into GLSL, which allows to write OpenGL shaders directly inside computer graphics applications code. The concept is that shader correctness is checked at compile-time while a string corresponding to the GLSL code is produced at run-time. CSL aims to provide convenient and maintainable shader writing thanks to :

+ Having a syntax as close as possible to GLSL.
+ Checking GLSL specification compliance at compile-time as much as possible.
+ The possibility to use C++ as meta language for clean shader generation.

CSL requires a C++14 compiler. It was tested on Visual Studio 2017, GCC 8.3.0 and Clang (Apple LLVM version 10.0.1).
CSL does not require any dependency as it only relies on the STL and some Boost Preprocessor files that are included in the repo. *Optionnaly*, it is possible to only clone `\src\csl\` if the Boost Preprocessor is already available from elsewhere.

CSL also provides a [shader suite](https://github.com/thonatt/CSL/tree/master/src/shader_suite) which includes several shaders, from didactic examples to shaders used in complex graphics applications.

**Disclaimer** : This project is a work in progress. The current coverage of the GLSL specification is only partial. The guideline is to add remaining operators/functions/features as soon as they needed in a CSL shader. In particular, the goal is to first make possible what is legal in GLSL. In a second time, the goal will be to make impossible what is not valid in GLSL. 

# Setup

As CSL is a header-only library, a simple include of the file `<csl\core.hpp>` is enough to use it. Here is small program, showing a vertex shader example and its corresponding output. CSL syntax is described in more detail in the [syntax section](#csl-syntax).

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
      Shader myShader;

      In<vec3, Layout<Location<0>> position;

      shader.main([&]{
            gl_Position = vec4(position, 1.0);
      });

      std::cout << myShader.str() << std::endl;
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

For readability purposes, all outputs are shown as if the code used named variables. See the [naming variables](#naming-variables) section for more details about the actual output.

# CSL syntax

As GLSL and C++ share a common C base language, their syntax are quite similar. However, the goal of CSL is to write C++ code that can perform introspection, while keeping a syntax close to GLSL, meaning that some tradeoffs had to be made. This section covers the subtleties of the CSL syntax (hopefully nothing too repulsive !). It is also a summary of what is possible in CSL:

+ [Setup a shader](#shader-setup)
+ [Basic and Sampler types](#basic-and-sampler-types)
+ [Naming variables](#naming-variables)
+ [Operators and Swizzles](#operators-and-swizzles)
+ [Memory and Layout qualifiers](#memory-and-layout-qualifiers)
+ [Arrays and Functions](#arrays-and-functions)
+ [Building blocks](#building-blocks)
+ [Structs and Interface blocks](#structs-and-interface-blocks)
+ [Subroutines](#subroutines)

## Shader setup

Shader type and GLSL version are setup using a specific namespace. For example, `using namespace csl::vert_330` gives access to the built-in functions and built-in variables for a vertex shader with GLSL 3.30. Starting a new shader requires to create a novel variable of type `Shader`. This type contains two important member functions. The first one is `Shader::main` which allows to setup the main using a lambda function with no argument that returns nothing. The second one is `Shader::str`, which retrieves the `std::string` associated to the shader that can be sent to the GPU. See the [previous section](#setup) for en example.

CSL currently assumes instructions are called sequentially so it is not thread-safe.

## Basic and Sampler types

CSL defines the types used in GLSL. Most CSL types have the exact same typename as their GLSL counterpart. For example, `void`, `vec3`, `ivec2`, `dmat4`, `mat4x3`, `sampler2DArray`, `uimageCube` are all valid types that can be used as is. The only exceptions are `double`, `float`, `int` and `bool` as they would conflict with C++ keywords. Valid associated CSL typenames are `Double`, `Float`, `Int`, `Bool` - and `Uint` to keep the consistency.

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

## Naming variables

Because C++ objects do not have names, it is not possible to forward directly the CSL variable names to the GLSL output. As a consequence, CSL will perfom automatic variable naming, which has a significant impact on the output shader readability.

<details>
    <summary>Automatic naming example</summary>
<table>
  <tr>
    <th>Code with automatic naming</th>
    <th>Actual output</th> 
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
   in vec3 v0;
   in vec3 v1;
   uniform vec3 v2;

   void main() {
      float x0 = 1.2;
      vec3 v4 = normalize(v2 - v1);
      vec3 v5 = normalize(v0);
      float x1;
      x1 = x0*dot(v5, v4);
   }

```
</td> 
  </tr>
</table>
</details>

Therefore, it is possible, **yet optionnal**, to provide a name to any CSL variable. It can be done either when declaring a variable using the `(const std::string &)` constructor, or when initializing a variable using the `<<(const std::string &)` operator. Manual naming is rather cumbersome, but may be useful for certain cases such as:
+ access to uniforms location with an explicit name
+ name consistency between vertex out and fragment in variables
+ output shader readability for debugging purposes

<details>
    <summary>Same example with manual naming</summary>
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

## Operators and Swizzles

As C++ and GLSL share a common C base syntax, most of the operators keywords are identical and can be used as is. This includes for example:
+ `+`, `-`, `*`, `/` and their assignment operator counterparts,
+ `==`, `<`, `>` , `&&` and other binary relational or bitwise operators,
+ `[]` for component or row access

One exception is the ternary operator ` ? : `. Even if the synthax is identical between C++ and GLSL, it cannot be overloaded. Therefore it is replaced by a macro `GL_TERNARY` with the 3 arguments.

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

## Memory and Layout qualifiers

Memory qualifiers are available in CSL in the form of template classes. Their template parameters are the underlying type and an optional `Layout`, which is itself a template class. Currently available memory qualifiers are `In`,`Out` and `Uniform`. Layout qualifiers are classes, which may be templated over an unsigned int when it requires a value. CSL layout qualifiers are identical to GLSL, except for beginning with an uppercase.

<details>
    <summary>Qualifier and layout examples</summary>
<table>
  <tr>
    <th>Code</th>
    <th>Output</th> 
  </tr>
  <tr>
    <td>
        
  ```cpp
	Out<vec4> color;
	In<vec3, Layout<Location<4>>> position;
	Uniform<Array<sampler2DArray, 8>, Layout<Binding<0>>> samplers;

	//in case of multiple occurences, last one prevails
	Uniform<mat4, Layout<Location<0>, Row_major, Location<1>> > mvp;
```
</td>
    <td>
  
```cpp
   out vec4 color;
   layout(location = 4) in vec3 position;
   layout(binding = 0) uniform sampler2DArray samplers[8];
   layout(row_major, location = 1) uniform mat4 mvp;

```
</td> 
  </tr>
</table>
</details>


## Arrays and Functions

Arrays in CSL are a template class similar to `std::array`, with parameters the internal type and the size. Unspecified or zero size are used for implicitely sized GLSL arrays. Indexing is done with the usual `[]` operator. Multi dimensional arrays are supported as nested arrays. 

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

Functions in CSL are objects that can be created using the `declareFunc` template function with a C++ lambda as parameter. The return type must be explicitely specified as template parameter. Returns are declared using the `GL_RETURN;` or `GL_RETURN(expression)` syntax. Parameters are named using default argument values. The function can be called later in the code using the usual `()` operator. Function overloading is possible in CSL by providing multiple return types and lambdas.

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
	//named function with named parameters
	auto add = declareFunc<vec3>("add",
		[](vec3 a = "a", vec3 b = "b") {
			GL_RETURN(a + b);
		}
	);

	//function with some named parameters
	auto addI = declareFunc<Int>(
		[](Int a, Int b = "b", Int c = "") {
			GL_RETURN(a + b + c);
		}
	);

	//function calling another function
	auto sub = declareFunc<vec3>([&](vec3 a, vec3 b) {
		fun();
		GL_RETURN(add(a, -b));
	});

	//named function with overload
	auto square = declareFunc<vec3, ivec3, void>( "square",
		[](vec3 a = "a") {
			GL_RETURN(a*a);
		}, 
		[](ivec3 b = "b") {
			GL_RETURN(b*b);
		},
		[] { GL_RETURN; }
	);
```
</td>
    <td>
  
```cpp
   void function_0() {
   }


   vec3 add(vec3 a, vec3 b) {
      return a + b;
   }


   int function_1(int int_1, int b, int int_0) {
      return int_1 + b + int_0;
   }


   vec3 function_2(vec3 vec3_2, vec3 vec3_1) {
      function_0();
      return add(vec3_2, -vec3_1);
   }


   vec3 square(vec3 a) {
      return a*a;
   }

   ivec3 square(ivec3 b) {
      return b*b;
   }

   void square() {
      return;
   }
```
</td> 
  </tr>
</table>
</details>

## Building blocks

Selection, iteration and jump statements are available in CSL. As C++ and GLSL share the same keywords, CSL redefines them using macros with the syntax `GL_KEYWORD`, namely `GL_FOR`, `GL_CONTINUE`, `GL_BREAK`, `GL_WHILE`, `GL_IF`, `GL_ELSE`, `GL_ELSE_IF`, `GL_SWITCH`, `GL_CASE` and `GL_DEFAULT`. Their behavior is mostly identical to C++ and GLSL. Here are some comments and the few limitations:
+ A `GL_SWITCH` **must** contain a `GL_DEFAULT` case, even if it happens to be empty.
+ CSL syntax for `case value :` is `GL_CASE(value) :`.
+ Condition and loop in `GL_FOR( init-expression; condition-expression; loop-expression)` must not contain more than one statement each. There is no limit about the number of initialisation in the init-expression. Any of these expressions can also be empty.
+ Variables declared in `GL_FOR` args expressions outlive the scope of the `for` body. It is possible to prevent that by putting explicitly the for in a scope.
+ Statements can be nested

<details>
    <summary>Building blocks examples</summary>
<table>
  <tr>
    <th>Code</th>
    <th>Output</th> 
  </tr>
  <tr>
    <td>
        
  ```cpp
//empty for
GL_FOR(;;) { GL_BREAK; }

//nested building blocks
	GL_FOR(Int i = 0; i < 5; ++i) {
		GL_IF(i == 3) {
			++i;
			GL_CONTINUE;
		} GL_ELSE_IF(i < 3) {
			i += 3;
		} GL_ELSE {
			GL_FOR(; i > 1;)
				--i;
		}
	}
	//Not possible as i is still in the scope
	//Int i; 

	{
		GL_FOR(Int j = 0; j < 5;) {
			GL_WHILE(j != 3) {
				++j;
			}
		}
	}
	//OK since previous for was put in a scope
	Int j;

	GL_SWITCH(j) {
		GL_CASE(0) : { GL_BREAK; }
		GL_CASE(2) : { j = 3; }
		GL_DEFAULT: { j = 2; }
	}
```
</td>
    <td>
  
```cpp
   for( ; ; ){
      break;
   }
   for( int i = 0; i < 5; ++i){
      if( i == 3 ) {
         ++i;
         continue;
      } else if( i < 3 ) {
         i += 3;
      } else {
         for( ; i > 1; ){
            --i;
         }
      }
   }
   for( int j = 0; j < 5; ){
      while( j != 3 ){
         ++j;
      }
   }
   int j;
   switch( j ){
      case 0 : {
         break;
      }
      case 2 : {
         j = 3;
      }
      default : {
         j = 2;
      }
   }

```
</td> 
  </tr>
</table>
</details>

## Structs and Interface blocks

CSL structs are declared using the syntax `GL_STRUCT(StructTypename, member list ...);`. As members in C++ have no way to know if they belong to a struct, CSL has to use some form of reflection, based on C++ preprocessor magic. So to help the preprocessor looping over the members, one must declare the `member list` using *typed expressions*, which look like this: `(Type1) member1, (Type2) member2, ...`

<details>
    <summary>Struct examples</summary>
<table>
  <tr>
    <th>Code</th>
    <th>Output</th> 
  </tr>
  <tr>
    <td>
        
  ```cpp
	//struct declaration
	GL_STRUCT(MyBlock,
		(mat4) mvp,
		(vec4) center
	);

	//nested struct
	GL_STRUCT(MyBigBlock,
		(MyBlock) innerBlock,
		(vec4) center
	);

	//usage
	MyBigBlock bigBlock("block");
	MyBlock block = MyBlock(mat4(1), vec3(0)) << "block";

	block.center = bigBlock.innerBlock.mvp*block.center;
```
</td>
    <td>
  
```cpp
   struct MyBlock {
      mat4 mvp;
      vec4 center;
   }

   struct MyBigBlock {
      MyBlock innerBlock;
      vec4 center;
   }
   
   MyBigBlock bigBlock;
   MyBlock block = MyBlock(mat4(1), vec4(vec3(0)));
   block.center = bigBlock.innerBlock.mvp*bigBlock.center;
```
</td> 
  </tr>
</table>
</details>

Interface block are similar to structs with syntax `GL_INTERFACE_BLOCK(Qualifier, Typename, Name, ArraySize, member list ... );`. `Name` and `ArraySize` can be empty. `Qualifier` refers to the memory and layout qualifiers associated to the block. The syntax is identical to single variables except there is no need to specify the type. `Name` is name of the variable associated to the block. In case it is empty, it declares an unnamed interface block and the members belong directly to the current scope. If not empty, `ArraySize` refers to the size of the declared array. Again, a size of 0 is used to declare an implicitely sized array.  

<details>
    <summary>Interface block examples</summary>
<table>
  <tr>
    <th>Code</th>
    <th>Output</th> 
  </tr>
  <tr>
    <td>
        
  ```cpp
	//unnamed interface
	GL_INTERFACE_BLOCK(In<>, SimpleInterface, , ,
		(Float) current_time
	);

	//named array interface with qualifiers
	GL_INTERFACE_BLOCK(Out<Layout<Binding<0>>>, Output, out, 3,
		(vec3) position,
		(Float) velocity
	);

	out[0].position += current_time * out[0].velocity;
```
</td>
    <td>
  
```cpp
   in SimpleInterface {
      float current_time;
   };

   layout(binding = 0) out Output {
      vec3 position;
      float velocity;
   } out[3];
   
   out[0].position += current_time*out[0].velocity;
```
</td> 
  </tr>
</table>
</details>

Since the `member list` is parsed by the preprocessor, **members typename must not contain any comma**. To circumvent this issue, type aliases must be created. In the case of Arrays, the type helper `Array<T>::Size<N>` can be used as an alias for `Array<T,N>`.

<details>
    <summary>Type alias examples</summary>
<table>
  <tr>
    <th>Code</th>
    <th>Output</th> 
  </tr>
  <tr>
    <td>
        
  ```cpp
	using Quali = Uniform<Layout<Binding<0>, Std140>>;
	using vec4A = Array<vec4, 16>;
	GL_INTERFACE_BLOCK(Quali, MyInterface, vars, 2,
		(vec4A) myVecs,
		(Array<mat4>::Size<4>) myMats
	);
```
</td>
    <td>
  
```cpp
   layout(std140, binding = 0) uniform MyInterface {
      vec4 myVecs[16];
      mat4 myMats[4];
   } vars[2];
```
</td> 
  </tr>
</table>
</details>

Also since CSL must rely on a macro for structs and interface blocks, it means it can directly forward the members name (and interface block variable names). Therefore there is no need for either automatic or manual naming in those cases. 

## Subroutines

Not yet implemented.
