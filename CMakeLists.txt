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

if(APPLE)
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++17")
endif()

find_package(Vulkan REQUIRED)
set(Vulkan_SDK_DIR ${Vulkan_INCLUDE_DIR}/../)
set(GLSLANG_DIR ${Vulkan_SDK_DIR}/glslang/)

add_definitions(-DGLM_FORCE_DEPTH_ZERO_TO_ONE)

if(WIN32)
    add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
elseif(APPLE)
    add_definitions(-DVK_USE_PLATFORM_MACOS_MVK)
endif()

if(WIN32)
    set(GLSLANG_SEARCH_PATH
        ${GLSLANG_DIR}/build/glslang/Release/
        ${GLSLANG_DIR}/build/glslang/OSDependent/Windows/Release/
        ${GLSLANG_DIR}/build/hlsl/Release/
        ${GLSLANG_DIR}/build/OGLCompilersDLL/Release/
        ${GLSLANG_DIR}/build/SPIRV/Release/)

    set(GLSLANG_DEBUG_SEARCH_PATH
        ${GLSLANG_DIR}/build/glslang/Debug/
        ${GLSLANG_DIR}/build/glslang/OSDependent/Windows/Debug/
        ${GLSLANG_DIR}/build/hlsl/Debug/
        ${GLSLANG_DIR}/build/OGLCompilersDLL/Debug/
        ${GLSLANG_DIR}/build/SPIRV/Debug/)

    set (SPIRV_TOOLS_SEARCH_PATH ${Vulkan_SDK_DIR}/spirv-tools/build/source/Release/)
    set (SPIRV_TOOLS_DEBUG_SEARCH_PATH ${Vulkan_SDK_DIR}/spirv-tools/build/source/Debug/)

    set (SPIRV_TOOLS_OPT_SEARCH_PATH ${Vulkan_SDK_DIR}/spirv-tools/build/source/opt/Release/)
    set (SPIRV_TOOLS_OPT_DEBUG_SEARCH_PATH ${Vulkan_SDK_DIR}/spirv-tools/build/source/opt/Debug/)
elseif(APPLE)
    set (GLSLANG_SEARCH_PATH ${Vulkan_SDK}/lib/)
    set (SPIRV_TOOLS_SEARCH_PATH ${Vulkan_SDK}/lib/)
    set (SPIRV_TOOLS_OPT_SEARCH_PATH ${Vulkan_SDK}/lib/)
endif()

find_library(GLSLANG_LIB NAMES glslang REQUIRED HINTS ${GLSLANG_SEARCH_PATH})
find_library(SPIRV_LIB NAMES SPIRV REQUIRED HINTS ${GLSLANG_SEARCH_PATH})
find_library(OGLCompiler_LIB NAMES OGLCompiler REQUIRED HINTS ${GLSLANG_SEARCH_PATH})
find_library(OSDependent_LIB NAMES OSDependent REQUIRED HINTS ${GLSLANG_SEARCH_PATH})
find_library(HLSL_LIB NAMES HLSL HINTS ${GLSLANG_SEARCH_PATH})
find_library(SPIRV_TOOLS_LIB NAMES SPIRV-Tools HINTS ${SPIRV_TOOLS_SEARCH_PATH})
find_library(SPIRV_TOOLS_OPT_LIB NAMES SPIRV-Tools-opt HINTS ${SPIRV_TOOLS_OPT_SEARCH_PATH})
find_library(SPIRV_REMAPPER_LIB NAMES SPVRemapper HINTS ${GLSLANG_SEARCH_PATH})

if(WIN32)
    find_library(GLSLANG_DLIB NAMES glslangd REQUIRED HINTS ${GLSLANG_DEBUG_SEARCH_PATH})
    find_library(SPIRV_DLIB NAMES SPIRVd REQUIRED HINTS ${GLSLANG_DEBUG_SEARCH_PATH})
    find_library(OGLCompiler_DLIB NAMES OGLCompilerd REQUIRED HINTS ${GLSLANG_DEBUG_SEARCH_PATH})
    find_library(OSDependent_DLIB NAMES OSDependentd REQUIRED HINTS ${GLSLANG_DEBUG_SEARCH_PATH})
    find_library(HLSL_DLIB NAMES HLSLd REQUIRED HINTS ${GLSLANG_DEBUG_SEARCH_PATH})
    find_library(SPIRV_TOOLS_DLIB NAMES SPIRV-Tools REQUIRED HINTS ${SPIRV_TOOLS_DEBUG_SEARCH_PATH})
    find_library(SPIRV_TOOLS_OPT_DLIB NAMES SPIRV-Tools-opt HINTS ${SPIRV_TOOLS_OPT_DEBUG_SEARCH_PATH})
    find_library(SPIRV_REMAPPER_DLIB NAMES SPVRemapperd REQUIRED HINTS ${GLSLANG_DEBUG_SEARCH_PATH})
endif()

include_directories(AFTER SYSTEM
    ${Vulkan_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/stb/
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/glm/
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/json/src/
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
    ${GLSLANG_INCLUDE_DIR}
)

add_library(glslang STATIC IMPORTED)
add_library(SPIRV STATIC IMPORTED)
add_library(OGLCompiler STATIC IMPORTED)
add_library(OSDependent STATIC IMPORTED)
add_library(HLSL STATIC IMPORTED)
add_library(SPIRV-Tools STATIC IMPORTED)
add_library(SPIRV-Tools-opt STATIC IMPORTED)
add_library(SPVRemapper STATIC IMPORTED)

if(WIN32)
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
elseif(APPLE)
    set_target_properties(glslang PROPERTIES
        IMPORTED_LOCATION ${GLSLANG_LIB})
    set_target_properties(SPIRV PROPERTIES
        IMPORTED_LOCATION ${SPIRV_LIB})
    set_target_properties(OGLCompiler PROPERTIES
        IMPORTED_LOCATION ${OGLCompiler_LIB})
    set_target_properties(OSDependent PROPERTIES
        IMPORTED_LOCATION ${OSDependent_LIB})
    set_target_properties(HLSL PROPERTIES
        IMPORTED_LOCATION ${HLSL_LIB})
    set_target_properties(SPIRV-Tools PROPERTIES
        IMPORTED_LOCATION ${SPIRV_TOOLS_LIB})
    set_target_properties(SPIRV-Tools-opt PROPERTIES
        IMPORTED_LOCATION ${SPIRV_TOOLS_OPT_LIB})
    set_target_properties(SPVRemapper PROPERTIES
        IMPORTED_LOCATION ${SPIRV_REMAPPER_LIB})
endif()

file(GLOB_RECURSE ENGINE_CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/Husky/*.cpp)
file(GLOB_RECURSE ENGINE_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/Husky/*.h)

file(GLOB_RECURSE SAMPLE_COMMON_CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/Sample/common/*.cpp)
file(GLOB_RECURSE SAMPLE_COMMON_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/Sample/common/*.h)

add_library(HuskyEngine ${ENGINE_CPP_FILES} ${ENGINE_HEADER_FILES})

if(WIN32)
    file(GLOB_RECURSE SAMPLE_WIN32_CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/Sample/win32/*.cpp)
    file(GLOB_RECURSE SAMPLE_WIN32_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/Sample/win32/*.h)

    add_executable(HuskyEngineSample WIN32
        ${SAMPLE_COMMON_CPP_FILES}
        ${SAMPLE_WIN32_CPP_FILES}
        ${SAMPLE_COMMON_HEADER_FILES}
        ${SAMPLE_WIN32_HEADER_FILES})
elseif(APPLE)
    file(GLOB_RECURSE SAMPLE_MACOS_OBJC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/Sample/macOS/*.mm)
    file(GLOB_RECURSE SAMPLE_MACOS_CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/Sample/macOS/*.cpp)
    file(GLOB_RECURSE SAMPLE_MACOS_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/Sample/macOS/*.h)
    set(SAMPLE_MACOS_RESOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/res/macOS/Main.storyboard)

    add_executable(HuskyEngineSample
        MACOSX_BUNDLE
        ${SAMPLE_COMMON_CPP_FILES}
        ${SAMPLE_MACOS_CPP_FILES}
        ${SAMPLE_MACOS_OBJC_FILES}
        ${SAMPLE_COMMON_HEADER_FILES}
        ${SAMPLE_MACOS_HEADER_FILES}
        ${SAMPLE_MACOS_RESOURCE_FILES})

    set_target_properties(
        HuskyEngineSample
        PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/res/macOS/Info.plist.in
        RESOURCE "${SAMPLE_MACOS_RESOURCE_FILES}")

    target_link_libraries(HuskyEngineSample "-framework Foundation")
    target_link_libraries(HuskyEngineSample "-framework CoreVideo")
    target_link_libraries(HuskyEngineSample "-framework QuartzCore")
    target_link_libraries(HuskyEngineSample "-framework Cocoa")
endif()

target_link_libraries(HuskyEngine ${Vulkan_LIBRARIES})
target_link_libraries(HuskyEngine glslang)
target_link_libraries(HuskyEngine SPIRV)
target_link_libraries(HuskyEngine OGLCompiler)
target_link_libraries(HuskyEngine OSDependent)
target_link_libraries(HuskyEngine HLSL)
target_link_libraries(HuskyEngine SPIRV-Tools)
target_link_libraries(HuskyEngine SPIRV-Tools-opt)
target_link_libraries(HuskyEngine SPVRemapper)

target_link_libraries(HuskyEngineSample HuskyEngine)

if(WIN32)
    set(HUSKY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/_build)
    set(HUSKY_OUTPUT_DIRECTORY_DEBUG ${HUSKY_OUTPUT_DIRECTORY}/Debug)
    set(HUSKY_OUTPUT_DIRECTORY_RELEASE ${HUSKY_OUTPUT_DIRECTORY}/Release)
    
    set_target_properties(HuskyEngine PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG HUSKY_OUTPUT_DIRECTORY_DEBUG)
    set_target_properties(HuskyEngineSample PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_RELEASE HUSKY_OUTPUT_DIRECTORY_RELEASE)

    set_target_properties(HuskyEngineSample PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY ${HUSKY_OUTPUT_DIRECTORY}/$(Configuration)) #fuck cmake for making me do this shit
endif()

# add_custom_command(
#         TARGET HuskyEngine POST_BUILD
#         COMMAND ${CMAKE_COMMAND} -E copy_directory
#                 ${CMAKE_SOURCE_DIR}/src/Husky/Render/Shaders
#                 $<$<CONFIG:debug>:${HUSKY_OUTPUT_DIRECTORY_DEBUG}>$<$<CONFIG:release>:${HUSKY_OUTPUT_DIRECTORY_RELEASE}>/Shaders)