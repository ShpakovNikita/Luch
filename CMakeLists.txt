cmake_minimum_required(VERSION 3.9.0)

project(Husky VERSION 0.1 LANGUAGES CXX)
set(CXX_STANDARD 17)

set_property(GLOBAL PROPERTY USE_FOLDERS On) 

if(MSVC)
    if (MSVC_VERSION GREATER_EQUAL "1900")
        include(CheckCXXCompilerFlag)

        CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
        if (_cpp_latest_flag_supported)
            add_compile_options("/std:c++latest")
        endif()

        CHECK_CXX_COMPILER_FLAG("/permissive-" _cpp_non_permissive_supported)
        if (_cpp_non_permissive_supported)
            add_compile_options("/permissive-")
        endif()
    endif()
endif()

find_package(Vulkan REQUIRED)
set(Vulkan_SDK_DIR ${Vulkan_INCLUDE_DIR}/../)


add_definitions(-DGLM_FORCE_DEPTH_ZERO_TO_ONE)

if(WIN32)
    add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
endif()


set(GLSLANG_INCLUDE_DIR ${Vulkan_SDK_DIR}/glslang/)

set(GLSLANG_SEARCH_PATH
    ${Vulkan_SDK_DIR}/glslang/build/glslang/Release/
    ${Vulkan_SDK_DIR}/glslang/build/glslang/OSDependent/Windows/Release/
    ${Vulkan_SDK_DIR}/glslang/build/hlsl/Release/
    ${Vulkan_SDK_DIR}/glslang/build/OGLCompilersDLL/Release/
    ${Vulkan_SDK_DIR}/glslang/build/SPIRV/Release/)

set(GLSLANG_DEBUG_SEARCH_PATH
    ${Vulkan_SDK_DIR}/glslang/build/glslang/Debug/
    ${Vulkan_SDK_DIR}/glslang/build/glslang/OSDependent/Windows/Debug/
    ${Vulkan_SDK_DIR}/glslang/build/hlsl/Debug/
    ${Vulkan_SDK_DIR}/glslang/build/OGLCompilersDLL/Debug/
    ${Vulkan_SDK_DIR}/glslang/build/SPIRV/Debug/)

set (SPIRV_TOOLS_SEARCH_PATH ${Vulkan_SDK_DIR}/spirv-tools/build/source/Release)
set (SPIRV_TOOLS_DEBUG_SEARCH_PATH ${Vulkan_SDK_DIR}/spirv-tools/build/source/Debug)

set (SPIRV_TOOLS_OPT_SEARCH_PATH ${Vulkan_SDK_DIR}/spirv-tools/build/source/opt/Release)
set (SPIRV_TOOLS_OPT_DEBUG_SEARCH_PATH ${Vulkan_SDK_DIR}/spirv-tools/build/source/opt/Debug)

find_library(GLSLANG_LIB NAMES glslang REQUIRED HINTS ${GLSLANG_SEARCH_PATH})
find_library(SPIRV_LIB NAMES SPIRV REQUIRED HINTS ${GLSLANG_SEARCH_PATH})
find_library(OGLCompiler_LIB NAMES OGLCompiler REQUIRED HINTS ${GLSLANG_SEARCH_PATH})
find_library(OSDependent_LIB NAMES OSDependent REQUIRED HINTS ${GLSLANG_SEARCH_PATH})
find_library(HLSL_LIB NAMES HLSL REQUIRED HINTS ${GLSLANG_SEARCH_PATH})
find_library(SPIRV_TOOLS_LIB NAMES SPIRV-Tools REQUIRED HINTS ${SPIRV_TOOLS_SEARCH_PATH})
find_library(SPIRV_TOOLS_OPT_LIB NAMES SPIRV-Tools-opt HINTS ${SPIRV_TOOLS_OPT_SEARCH_PATH})
find_library(SPIRV_REMAPPER_LIB NAMES SPVRemapper REQUIRED HINTS ${GLSLANG_SEARCH_PATH})

find_library(GLSLANG_DLIB NAMES glslangd REQUIRED HINTS ${GLSLANG_DEBUG_SEARCH_PATH})
find_library(SPIRV_DLIB NAMES SPIRVd REQUIRED HINTS ${GLSLANG_DEBUG_SEARCH_PATH})
find_library(OGLCompiler_DLIB NAMES OGLCompilerd REQUIRED HINTS ${GLSLANG_DEBUG_SEARCH_PATH})
find_library(OSDependent_DLIB NAMES OSDependentd REQUIRED HINTS ${GLSLANG_DEBUG_SEARCH_PATH})
find_library(HLSL_DLIB NAMES HLSLd REQUIRED HINTS ${GLSLANG_DEBUG_SEARCH_PATH})
find_library(SPIRV_TOOLS_DLIB NAMES SPIRV-Tools REQUIRED HINTS ${SPIRV_TOOLS_DEBUG_SEARCH_PATH})
find_library(SPIRV_TOOLS_OPT_DLIB NAMES SPIRV-Tools-opt HINTS ${SPIRV_TOOLS_OPT_DEBUG_SEARCH_PATH})
find_library(SPIRV_REMAPPER_DLIB NAMES SPVRemapperd REQUIRED HINTS ${GLSLANG_DEBUG_SEARCH_PATH})

include_directories(AFTER
    ${Vulkan_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/glm/
	${CMAKE_CURRENT_SOURCE_DIR}/lib/json/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
    ${GLSLANG_INCLUDE_DIR}
)

file(GLOB_RECURSE ENGINE_CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/Husky/*.cpp)
file(GLOB_RECURSE ENGINE_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/Husky/*.h)

file(GLOB_RECURSE SAMPLE_CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/Sample/*.cpp)
file(GLOB_RECURSE SAMPLE_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/Sample/*.h)

add_library(glslang STATIC IMPORTED)
add_library(SPIRV STATIC IMPORTED)
add_library(OGLCompiler STATIC IMPORTED)
add_library(OSDependent STATIC IMPORTED)
add_library(HLSL STATIC IMPORTED)
add_library(SPIRV-Tools STATIC IMPORTED)
add_library(SPIRV-Tools-opt STATIC IMPORTED)
#add_library(Loader STATIC IMPORTED)
add_library(SPVRemapper STATIC IMPORTED)

set_target_properties(glslang PROPERTIES
    IMPORTED_LOCATION ${GLSLANG_LIB}
    IMPORTED_LOCATION_DEBUG ${GLSLANG_DLIB})
set_target_properties(SPIRV PROPERTIES
    IMPORTED_LOCATION ${SPIRV_LIB}
    IMPORTED_LOCATION_DEBUG ${SPIRV_DLIB})
set_target_properties(OGLCompiler PROPERTIES
    IMPORTED_LOCATION ${OGLCompiler_LIB}
    IMPORTED_LOCATION_DEBUG ${OGLCompiler_DLIB})
set_target_properties(OSDependent PROPERTIES
    IMPORTED_LOCATION ${OSDependent_LIB}
    IMPORTED_LOCATION_DEBUG ${OSDependent_DLIB})
set_target_properties(HLSL PROPERTIES
    IMPORTED_LOCATION ${HLSL_LIB}
    IMPORTED_LOCATION_DEBUG ${HLSL_DLIB})
set_target_properties(SPIRV-Tools PROPERTIES
    IMPORTED_LOCATION ${SPIRV_TOOLS_LIB}
    IMPORTED_LOCATION_DEBUG ${SPIRV_TOOLS_DLIB})
set_target_properties(SPIRV-Tools-opt PROPERTIES
    IMPORTED_LOCATION ${SPIRV_TOOLS_OPT_LIB}
    IMPORTED_LOCATION_DEBUG ${SPIRV_TOOLS_OPT_DLIB})
set_target_properties(SPVRemapper PROPERTIES
    IMPORTED_LOCATION ${SPIRV_REMAPPER_LIB}
    IMPORTED_LOCATION_DEBUG ${SPIRV_REMAPPER_DLIB})

add_library(HuskyEngine ${ENGINE_CPP_FILES} ${ENGINE_HEADER_FILES})
add_executable(HuskyEngineSample ${SAMPLE_CPP_FILES} ${SAMPLE_HEADER_FILES})

target_link_libraries(HuskyEngine ${Vulkan_LIBRARIES})
target_link_libraries(HuskyEngine glslang)
target_link_libraries(HuskyEngine SPIRV)
target_link_libraries(HuskyEngine OGLCompiler)
target_link_libraries(HuskyEngine OSDependent)
target_link_libraries(HuskyEngine HLSL)
target_link_libraries(HuskyEngine SPIRV-Tools)
target_link_libraries(HuskyEngine SPIRV-Tools-opt)
#target_link_libraries(HuskyEngine Loader)
target_link_libraries(HuskyEngine SPVRemapper)

target_link_libraries(HuskyEngineSample HuskyEngine)
