project(ProjectZero)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.8.4)

find_package(glm REQUIRED)

add_executable(Zero zero.cpp)
target_link_libraries(Zero PUBLIC glfw epoxy)
target_compile_options(Zero PRIVATE -Wno-trigraphs -g)

add_custom_target(Shaders ALL)
add_custom_command(TARGET Shaders PRE_BUILD COMMAND 
	cp -r ${CMAKE_CURRENT_SOURCE_DIR}/*_shaders ${CMAKE_BINARY_DIR} | :  )
