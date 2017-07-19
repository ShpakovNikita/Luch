cmake_minimum_required(VERSION 3.9.0)

project(Husky VERSION 0.1 LANGUAGES CXX)
set(CXX_STANDARD 17)

add_subdirectory(lib/glfw)
find_package(Vulkan REQUIRED)

add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)

include_directories(AFTER ${Vulkan_INCLUDE_DIR})

file(GLOB_RECURSE ENGINE_CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/Husky/*.cpp)
file(GLOB_RECURSE ENGINE_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/Husky/*.*)

file(GLOB_RECURSE SAMPLE_CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/Sample/*.cpp)
file(GLOB_RECURSE SAMPLE_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/Sample/*.h)

add_library(HuskyEngine ${ENGINE_CPP_FILES} ${ENGINE_HEADER_FILES})
add_executable(HuskyEngineSample ${SAMPLE_CPP_FILES} ${SAMPLE_HEADER_FILES})

target_link_libraries(HuskyEngine ${Vulkan_LIBRARIES})
target_link_libraries(HuskyEngineSample HuskyEngine)

