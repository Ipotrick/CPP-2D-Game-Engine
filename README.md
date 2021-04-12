# This is an (outdated) general purpose 2d realtime game/application engine
# I am currently rewriting this engine under the name daxa (got it up as a repo)
# This is configured to be used for Visual Studio 2019 only. 

It has the following features:
* simple job system
* multithreaded 2d renderer
* multithreaded collision system 
* physics system
* retained mode gui library
* entity component system based on sparse sets and paged arrays

dependencies:
the dependencies are all available in vcpkg, there is no integration of the dependencies outside of vcpkg.
the dependencies are all linked staticly for windows on x64
the configuration triple is: package:x64-windows-static
  * glfw3
  * glew
  * robin_hood-hashing
  * boost
  * yaml-cpp
