# CMakeList.txt : CMake project for Space, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(TARGET_NAME Space)
project (Space CXX C)

# For Cpp_Properties
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Sets the output dirs
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})

# For Debug
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall")
set(PROJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Space") 

message(${PROJECT_DIR})

set(SPACE_LIB_DIRECTORY ${PROJECT_DIR}/Libs)
set(SPACE_INCLUDE_DIRECTORY ${PROJECT_DIR}/Include/)
set(SPACE_SOURCE_DIRECTORY ${PROJECT_DIR}/Src/)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -LC:/VulkanSDK/1.3.275.0/Lib/") 

# include_dirs Func used due to issuesx with target_include_directories from VS CODE
function(include_dirs)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${PROJECT_DIR}/${includes}" PARENT_SCOPE) 
endfunction()

set(includes "Include/")
include_dirs()
set(includes "Src/Window/")
include_dirs()
set(includes "Src/Input/")
include_dirs()
set(includes "Src/Events/")
include_dirs()
set(includes "Src/Renderer/")
include_dirs()
set(includes "Src/Renderer/Vulkan/")
include_dirs()

set(includes "Libs/GLFW/include/")
include_dirs()
set(includes "Libs/GLFW/src/")
include_dirs()
set(includes "Libs/glm/glm/")
include_dirs()

add_subdirectory(Space/Libs)

set(SPACE_VULKAN_SRC_DIR "Space/Src/Renderer/Vulkan")

# Add source to this project's executable.
add_executable (${TARGET_NAME} Space/Space.cpp ${SPACE_VULKAN_SRC_DIR}/Buffers.cpp ${SPACE_VULKAN_SRC_DIR}/VulkanImplementations.cpp ${SPACE_VULKAN_SRC_DIR}/VulkanApi.cpp Space/Include/UUID.cpp Space/Src/Window/WindowsWindow.cpp Space/Src/Window/Window.cpp Space/Src/Window/WindowStack.cpp Space/Src/Input/Input.cpp Space/Src/Renderer/Renderer.cpp)
target_precompile_headers(${TARGET_NAME} PUBLIC Space/Include/PCH.h)

message("")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(Space PRIVATE SP_DEBUG)
	message("Build Configuration: Debug")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
	target_compile_definitions(Space PRIVATE SP_RELEASE)
	message("Build Configuration: Release")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Dist")
	target_compile_definitions(Space PRIVATE SP_DIST)
	message("Build Configuration: Dist")
endif()

# Links The Libraries
message("")
target_link_libraries(${TARGET_NAME} glfw vulkan-1 opengl32 gdi32 user32 kernel32 stdc++)
