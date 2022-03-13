find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW CONFIG REQUIRED)
find_package(robin_hood CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS container)
find_package(yaml-cpp CONFIG REQUIRED)

file(GLOB_RECURSE ENGINE_SOURCES "engine/*.cpp" "engine/*.c")
file(GLOB_RECURSE GAME_SOURCES "game/*.cpp" "game/*.c")

add_executable(main "main.cpp" ${ENGINE_SOURCES} ${GAME_SOURCES})
target_link_libraries(main PRIVATE GLEW::GLEW glfw robin_hood::robin_hood Boost::container yaml-cpp)
target_link_libraries(main PRIVATE stb)
target_compile_features(main PRIVATE cxx_std_20)
