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

here is an example of the collision and physics system in a 2d world where balls fall down to a drain and are spwed out at the top:
![](assets/Balls.png)

and an example where i made an ant simulation where the ants ai would work purely based on two pheromone grids to navigate, where the grid is updated in parallel via the job system:
![](assets/antsim.png)


dependencies:
the dependencies are all available in vcpkg, there is no integration of the dependencies outside of vcpkg.
the dependencies are all linked staticly for windows on x64
the configuration triple is: package:x64-windows-static
  * glfw3
  * glew
  * robin_hood-hashing
  * boost
  * yaml-cpp
