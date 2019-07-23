# C++ integrated Shading Language

CSL is a C++ header-only library for writing OpenGL shaders directly inside computer graphics applications code. The concept is that shader correctness is checked at compile-time while a string corresponding to the GLSL code is produced at run-time. CSL aims to provide convenient and maintainable shader writing thanks to :

+ Having a syntax as close as possible to GLSL.
+ Checking GLSL specification compliance at compile-time as much as possible.
+ The possibility to use C++ as meta language for clean shader generation.

By default, CSL does not require any dependency as it relies on the STL and some Boost Preprocessor files that are present in the repo. *Optionnaly*, it is possible to only clone `src\core\` if the Boost Preprocessor is already available from elsewhere.

# How to

As CSL is a header-only library, a simple include of the core file `src\csl\core.hpp` is enough to us it. Here is a simple example, with its corresponding output.

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
