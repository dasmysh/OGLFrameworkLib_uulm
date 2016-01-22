# OGLFramework_uulm
OpenGL framework for the VISCOM group at ulm university

Dependencies:
- CUDA 7.0
- OpenGL 4.4
- Boost (http://www.boost.org/)
- GLEW (http://glew.sourceforge.net/)
- FreeImage (http://freeimage.sourceforge.net/)

Dependencies included as submodules:
- GLM (http://glm.g-truc.net/)
- ImGUI (https://github.com/ocornut/imgui)

To pull all submodules from remote do

```git submodule update --init
```

or use the `--recursive` when cloning the repository.

The dependencies not included are expected to be in the Visual Studio include / library directories.

Other fonts can be used by generating Bitmap fonts using BMFont by AngelCode (http://www.angelcode.com/products/bmfont/) and converting them to a distance field be using the Distance Field AngelCode Font Converter by bitsquid (http://bitsquid.blogspot.de/2010/04/distance-field-based-rendering-of.html).
