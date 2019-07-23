# C++ integrated Shading Language

CSL is a C++ header-only library for writing OpenGL shaders directly inside computer graphics applications code. The concept is that shader correctness is checked at compile-time while a string corresponding to the GLSL code is produced at run-time. CSL aims to provide convenient and maintainable shader writing thanks to :

+ Having a syntax as close as possible to GLSL.
+ Checking GLSL specification compliance at compile-time as much as possible.
+ The possibility to use C++ as meta language for clean shader generation.

CSL also provides a [shader suite](https://github.com/thonatt/CSL/tree/master/src/shader_suite) which includes several shaders, from didactic examples to shaders used in complex graphics applications.

By default, CSL does not require any dependency as it relies on the STL and some Boost Preprocessor files that are present in the repo. *Optionnaly*, it is possible to only clone `\src\core\` if the Boost Preprocessor is already available from elsewhere.

# How to

As CSL is a header-only library, a simple include of the file `<csl\core.hpp>` is enough to use it. Here is a simple vertex shader example, with its corresponding output.

<details>
    <summary>Code test</summary>
  
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
</details>

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

Shader type and GLSL version are setup using the corresponding namespace, here `csl::vert_330` for a vertex shader with GLSL 3.30.

## CSL syntax

### Basic and Sampler types

### Operators and Swizzles

### Memory and Layout qualifiers

### Arrays and Functions

### Structs and Interface blocks

### Code structure

### Built-in functions and variables

### Variable naming

