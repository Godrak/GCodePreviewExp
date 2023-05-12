project(ProjectZero)

set(CMAKE_CXX_STANDARD 17)

cmake_minimum_required(VERSION 3.8.4)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(glm REQUIRED)

add_executable(Zero zero.cpp misc/sky.c)
add_custom_target(Shaders ALL)
add_custom_command(TARGET Shaders PRE_BUILD COMMAND 
	cp -r ${CMAKE_CURRENT_SOURCE_DIR}/*_shaders ${CMAKE_BINARY_DIR} | :  )
target_link_libraries(Zero glfw epoxy)

target_compile_options(Zero PRIVATE -Wno-trigraphs)
