cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(RayTracing)

file(GLOB sources ${PROJECT_SOURCE_DIR}/*.cpp)
file(GLOB headers ${PROJECT_SOURCE_DIR}/*.h)

set(LIBRARIES
	glfw3.lib
	opengl32.lib
	glew32s.lib)

link_directories("libraries/glew-2.1.0/lib/Release/x64")
link_directories("libraries/glfw-3.3.2/lib-vc2019")


add_executable(main Source.cpp ${sources} ${headers})
target_link_libraries(main ${LIBRARIES})

include_directories("libraries/glew-2.1.0/include")
include_directories("libraries/glfw-3.3.2/include")
include_directories("libraries/glm/include")

install(TARGETS main DESTINATION bin)