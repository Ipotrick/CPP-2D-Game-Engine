This is a toy project i use to learn C++, OpenGL and designing software (for game engine development and) in general.
This is configured to be used for Visual Studio 2019 only. 

dependencies:
the dependencies are all available in vcpkg, there is no integration of the dependencies outside of vcpkg.
the dependencies are all linked staticly for windows on x64
the configuration triple is: package:x64-windows-static
  glfw3
  glew
  robin_hood-hashing
  boost
  yaml-cpp
