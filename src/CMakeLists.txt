project(GCodePreviewExp)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.8.4)

find_package (glm REQUIRED PATHS glm/cmake NO_DEFAULT_PATH)

file(GLOB_RECURSE GLAD_SOURCES "glad/src/glad.c" "glad/*.h")
add_library(glad ${GLAD_SOURCES})
target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/glad/include)

file(GLOB IMGUI_SOURCES "imgui/*.cpp" "imgui/*.h")
add_library(imgui ${IMGUI_SOURCES})
target_link_libraries(imgui PUBLIC glad)
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/imgui)


add_executable(GCdeView main.cpp)
target_link_libraries(GCdeView PUBLIC glfw glad imgui)
target_compile_options(GCdeView PRIVATE -g)

add_custom_target(Shaders ALL)
add_custom_command(TARGET Shaders PRE_BUILD COMMAND 
	cp -r ${CMAKE_CURRENT_SOURCE_DIR}/*_shaders ${CMAKE_BINARY_DIR} | :  )
