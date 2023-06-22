project(GCodePreviewExp)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.8.4)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

option(WIN "WIN" OFF)
if(WIN)
	set(BUILD_FOR_WIN TRUE)
	set(BUILD_SHARED_LIBS OFF)
	find_package (glm REQUIRED PATHS glm/cmake/glm NO_DEFAULT_PATH)
	set(WININCL glm glfw/include)
	set(LIBLINK ${CMAKE_CURRENT_SOURCE_DIR}/glfw/lib-mingw-w64/libglfw3.a 
				${CMAKE_CURRENT_SOURCE_DIR}/glfw/lib-mingw-w64/libglfw3dll.a 
				opengl32 
				gdi32
				-lwinpthread
		)
elseif(APPLE)
	set(WININCL glm glfw/include)
	find_package (TBB REQUIRED)
	set(LIBLINK TBB::tbb
		${CMAKE_CURRENT_SOURCE_DIR}/glfw/lib-osx/libglfw3.a 
		"-framework Cocoa"
		"-framework IOKit"
		"-framework CoreVideo"
	)
else()
	set(WININCL glm glfw/include)
	find_package (TBB REQUIRED)
	set(LIBLINK glfw TBB::tbb)
endif()

file(GLOB_RECURSE GLAD_SOURCES "glad/src/glad.c" "glad/*.h")
add_library(glad ${GLAD_SOURCES})
target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/glad/include)

file(GLOB IMGUI_SOURCES "imgui/*.cpp" "imgui/*.h")
add_library(imgui ${IMGUI_SOURCES})
target_link_libraries(imgui PUBLIC glad)
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/imgui ${WININCL})

add_definitions(-DUSE_HARDCODED_PATHS="")

add_executable(GCdeView main.cpp)
target_link_libraries(GCdeView PUBLIC glad imgui ${LIBLINK})
target_include_directories(GCdeView PUBLIC ${WININCL})
if(WIN)
	target_link_options(GCdeView PUBLIC -static-libgcc -static-libstdc++ -static -Bstatic -lpthread)
endif()

add_custom_target(Shaders ALL)
add_custom_command(TARGET Shaders PRE_BUILD COMMAND 
	cp -r ${CMAKE_CURRENT_SOURCE_DIR}/*_shaders ${CMAKE_BINARY_DIR} | :  )
