cmake_minimum_required(VERSION 3.12.0)

project(Luch_${PLATFORM} VERSION 0.1 LANGUAGES CXX)
set(CXX_STANDARD 17)

set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)

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

        add_compile_options(/W4 /WX)
    endif()
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

if(USE_METAL AND NOT APPLE)
    message(FATAL_ERROR "Metal is only for macOS")
endif()

if((PLATFORM EQUAL "macOS") AND NOT APPLE)
    message(FATAL_ERROR "Trying to create macOS project not on macOS")
endif()

if((PLATFORM EQUAL "iOS") AND NOT APPLE)
    message(FATAL_ERROR "Trying to create iOS project not on macOS")
endif()

if(APPLE)
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++17")
    macro (set_xcode_property TARGET XCODE_PROPERTY XCODE_VALUE)
        set_property (TARGET ${TARGET} PROPERTY XCODE_ATTRIBUTE_${XCODE_PROPERTY} ${XCODE_VALUE})
    endmacro (set_xcode_property)
endif()

set(LUCH_PLATFORM ${PLATFORM})

if(USE_METAL)
    set(LUCH_USE_METAL true)
    add_compile_definitions(LUCH_USE_METAL)
endif()

if(USE_VULKAN)
    set(LUCH_USE_VULKAN true)
    add_compile_definitions(LUCH_USE_VULKAN)
endif()

if(PLATFORM STREQUAL "win32")
    find_package(SDL2 REQUIRED)
    set(LUCH_PLATFORM_WIN32 true)
    add_compile_definitions(LUCH_PLATFORM_WIN32)
endif()

if(PLATFORM STREQUAL "macOS")
    find_package(SDL2 REQUIRED)
    set(LUCH_PLATFORM_APPLE true)
    set(LUCH_PLATFORM_MACOS true)
    add_compile_definitions(LUCH_PLATFORM_MACOS)
endif()

if(LUCH_PLATFORM_APPLE)
    add_compile_definitions(LUCH_PLATFORM_APPLE)
endif()

if(PLATFORM STREQUAL "iOS")
    set(SDL2_INCLUDE_DIRS ${LIB_DIR}/SDL_iOS/include/SDL2/)
    set(SDL2_LIBRARIES -L${LIB_DIR}/SDL_iOS/lib/ -lSDL2 -lSDL2main)
    set(LUCH_PLATFORM_APPLE true)
    set(LUCH_PLATFORM_IOS true)
    add_compile_definitions(LUCH_PLATFORM_IOS)
endif()

# TODO Check depth for consistency
add_compile_definitions(GLM_FORCE_DEPTH_ZERO_TO_ONE)

# Theese libs are always used
include_directories(BEFORE SYSTEM
    ${LIB_DIR}/stb/
    ${LIB_DIR}/glm/
    ${LIB_DIR}/json/src/
    ${SDL2_INCLUDE_DIRS}
)

# Setup vulkan include and library paths
if(USE_VULKAN)
    add_compile_definitions(GLM_FORCE_RIGHT_HANDED)
    find_package(Vulkan REQUIRED)
    set(Vulkan_SDK_DIR ${Vulkan_INCLUDE_DIR}/../)
    set(GLSLANG_DIR ${Vulkan_SDK_DIR}/glslang/)

    if(LUCH_PLATFORM_WIN32)
        add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
    elseif(LUCH_PLATFORM_MACOS)
        add_definitions(-DVK_USE_PLATFORM_MACOS_MVK)
    elseif(LUCH_PLATFORM_IOS)
        add_definitions(-DVK_USE_PLATFORM_IOS_MVK)
    endif()

    if(LUCH_PLATFORM_WIN32)
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
    elseif(LUCH_PLATFORM_APPLE)
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

    if(LUCH_PLATFORM_WIN32)
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
        ${GLSLANG_DIR})

    add_library(glslang STATIC IMPORTED)
    add_library(SPIRV STATIC IMPORTED)
    add_library(OGLCompiler STATIC IMPORTED)
    add_library(OSDependent STATIC IMPORTED)
    add_library(HLSL STATIC IMPORTED)
    add_library(SPIRV-Tools STATIC IMPORTED)
    add_library(SPIRV-Tools-opt STATIC IMPORTED)
    add_library(SPVRemapper STATIC IMPORTED)

    if(LUCH_PLATFORM_WIN32)
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
    elseif(LUCH_PLATFORM_APPLE)
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

    file(GLOB_RECURSE ENGINE_VULKAN_CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/Vulkan/Luch/*.cpp)
    file(GLOB_RECURSE ENGINE_VULKAN_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/Vulkan/Luch/*.h)
endif()

file(GLOB_RECURSE ENGINE_COMMON_CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/common/Luch/*.cpp)
file(GLOB_RECURSE ENGINE_COMMON_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/common/Luch/*.h)

include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/include/common/)

add_library(LuchEngine
    ${ENGINE_COMMON_CPP_FILES}
    ${ENGINE_COMMON_HEADER_FILES})

target_link_libraries(LuchEngine ${SDL2_LIBRARIES})

# Add Luch Engine library

if(LUCH_PLATFORM_APPLE)
    file(GLOB_RECURSE ENGINE_APPLE_CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/${LUCH_PLATFORM}/Luch/*.cpp)
    file(GLOB_RECURSE ENGINE_APPLE_OBJC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/${LUCH_PLATFORM}/Luch/*.m)
    file(GLOB_RECURSE ENGINE_APPLE_OBJCPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/${LUCH_PLATFORM}/Luch/*.mm)
    file(GLOB_RECURSE ENGINE_APPLE_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/${LUCH_PLATFORM}/Luch/*.h)

    include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/include/${LUCH_PLATFORM}/)

    target_sources(LuchEngine
        PRIVATE
        ${ENGINE_APPLE_CPP_FILES}
        ${ENGINE_APPLE_OBJC_FILES}
        ${ENGINE_APPLE_OBJCPP_FILES}
        ${ENGINE_APPLE_HEADER_FILES})
endif()

if(LUCH_PLATFORM_WIN32)
    file(GLOB_RECURSE ENGINE_WIN32_CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/win32/Luch/*.cpp)
    file(GLOB_RECURSE ENGINE_WIN32_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/win32/Luch/*.h)

    include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/include/win32/)

    target_sources(LuchEngine
        ${ENGINE_WIN32_CPP_FILES}
        ${ENGINE_WIN32_HEADER_FILES})
endif()

# Add Metal implementation if it's used
if(USE_METAL)
    add_compile_definitions(GLM_FORCE_LEFT_HANDED)

    file(GLOB_RECURSE MTLPP_OBJCPP_FILES ${LIB_DIR}/mtlpp/src/*.mm)
    file(GLOB_RECURSE MTLPP_HEADER_FILES ${LIB_DIR}/mtlpp/src/*.hpp)
    # Add mtlpp library and includes
    include_directories(AFTER
        ${LIB_DIR}/mtlpp/src)

    add_library(mtlpp
        ${MTLPP_OBJCPP_FILES}
        ${MTLPP_HEADER_FILES})

    target_link_libraries(LuchEngine mtlpp)

    # Add Metal implementation files
    file(GLOB_RECURSE ENGINE_METAL_CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/Metal/Luch/*.cpp)
    file(GLOB_RECURSE ENGINE_METAL_OBJC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/Metal/Luch/*.m)
    file(GLOB_RECURSE ENGINE_METAL_OBJCPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/Metal/Luch/*.mm)
    file(GLOB_RECURSE ENGINE_METAL_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/Metal/Luch/*.h)

    include_directories(AFTER
        ${CMAKE_CURRENT_SOURCE_DIR}/include/Metal/)

    target_sources(LuchEngine
        PRIVATE
        ${ENGINE_METAL_CPP_FILES}
        ${ENGINE_METAL_OBJC_FILES}
        ${ENGINE_METAL_OBJCPP_FILES}
        ${ENGINE_METAL_HEADER_FILES})
endif()

# TODO Add vulkan implementation

# if(USE_VULKAN)
#     target_link_libraries(LuchEngine ${Vulkan_LIBRARIES})
#     target_link_libraries(LuchEngine glslang)
#     target_link_libraries(LuchEngine SPIRV)
#     target_link_libraries(LuchEngine OGLCompiler)
#     target_link_libraries(LuchEngine OSDependent)
#     target_link_libraries(LuchEngine HLSL)
#     target_link_libraries(LuchEngine SPIRV-Tools)
#     target_link_libraries(LuchEngine SPIRV-Tools-opt)
#     target_link_libraries(LuchEngine SPVRemapper)
# endif()

# Add sample
file(GLOB_RECURSE SAMPLE_COMMON_CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/common/Sample/*.cpp)
file(GLOB_RECURSE SAMPLE_COMMON_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/common/Sample/*.h)

add_executable(LuchEngineSample
        ${SAMPLE_COMMON_CPP_FILES}
        ${SAMPLE_COMMON_HEADER_FILES})

# Add resources
set(SAMPLE_RUNTIME_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${PLATFORM}/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>)

set_target_properties(
    LuchEngineSample PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${SAMPLE_RUNTIME_OUTPUT_DIR})

set(SAMPLE_DATA_DIR ${SAMPLE_RUNTIME_OUTPUT_DIR}/Data/)

if(LUCH_PLATFORM_IOS)
    set(SAMPLE_RESOURCES)

    file(GLOB_RECURSE SAMPLE_GLTF2_FILES ${CMAKE_CURRENT_SOURCE_DIR}/res/gltf2/*)
    list(APPEND SAMPLE_RESOURCES ${SAMPLE_GLTF2_FILES})
    if(USE_METAL)
        file(GLOB_RECURSE SAMPLE_SHADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/Metal/Luch/Render/Shaders/*)
        list(APPEND SAMPLE_RESOURCES ${SAMPLE_SHADERS})
    endif()

    set_source_files_properties(
        ${SAMPLE_RESOURCES}
        PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources)
else()
    add_custom_command(
        TARGET LuchEngineSample
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove_directory
        ${SAMPLE_DATA_DIR})

    add_custom_command(
        TARGET LuchEngineSample
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
        ${SAMPLE_DATA_DIR})

    if(USE_METAL)
        add_custom_command(
            TARGET LuchEngineSample
            PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/src/Metal/Luch/Render/Shaders/
            ${SAMPLE_DATA_DIR}/Shaders/)
    endif()

    add_custom_command(
        TARGET LuchEngineSample
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/res/gltf2
        ${SAMPLE_DATA_DIR}/gltf2/)
endif()

if(LUCH_PLATFORM_WIN32)
    file(GLOB_RECURSE SAMPLE_WIN32_CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/win32/Sample/*.cpp)
    file(GLOB_RECURSE SAMPLE_WIN32_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/win32/Sample/*.h)

    target_sources(LuchEngineSample PUBLIC
        ${SAMPLE_WIN32_CPP_FILES}
        ${SAMPLE_WIN32_HEADER_FILES})
endif()

if(LUCH_PLATFORM_APPLE)
    file(GLOB_RECURSE SAMPLE_APPLE_OBJC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/${LUCH_PLATFORM}/Sample/*.m)
    file(GLOB_RECURSE SAMPLE_APPLE_OBJCPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/${LUCH_PLATFORM}/Sample/*.mm)
    file(GLOB_RECURSE SAMPLE_APPLE_CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/${LUCH_PLATFORM}/Sample/*.cpp)
    file(GLOB_RECURSE SAMPLE_APPLE_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/${LUCH_PLATFORM}/Sample/*.h)

    target_sources(LuchEngineSample PRIVATE
        ${SAMPLE_APPLE_CPP_FILES}
        ${SAMPLE_APPLE_OBJC_FILES}
        ${SAMPLE_APPLE_OBJCPP_FILES}
        ${SAMPLE_APPLE_HEADER_FILES})

    if(LUCH_PLATFORM_IOS)
        set_xcode_property(LuchEngineSample CODE_SIGN_IDENTITY "iPhone Developer")
    elseif(LUCH_PLATFORM_MACOS)
        set_xcode_property(LuchEngineSample CODE_SIGN_IDENTITY "Mac Developer")
    endif()

    target_sources(LuchEngineSample PRIVATE
        ${SAMPLE_RESOURCES})

    set_source_files_properties(
        ${SAMPLE_RESOURCES}
        PROPERTIES
        HEADER_FILE_ONLY ON
        XCODE_EXPLICIT_FILE_TYPE "compiled")

    if(DEVELOPMENT_TEAM_ID)
        set_xcode_property(LuchEngineSample DEVELOPMENT_TEAM ${DEVELOPMENT_TEAM_ID})
    endif()
    set_xcode_property(LuchEngineSample ENABLE_BITCODE false)
    set_xcode_property(LuchEngineSample PRODUCT_BUNDLE_IDENTIFIER "es.LuchEngineSample")

    set_target_properties(
        LuchEngineSample
        PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/res/${LUCH_PLATFORM}/Sample/Info.plist.in
        RESOURCE "${SAMPLE_RESOURCES}")

    target_link_libraries(LuchEngineSample ${SDL2_LIBRARIES})

    target_link_libraries(LuchEngineSample "-framework Foundation")
    target_link_libraries(LuchEngineSample "-framework CoreVideo")
    target_link_libraries(LuchEngineSample "-framework QuartzCore")
    target_link_libraries(LuchEngineSample "-framework Metal")

    if(LUCH_PLATFORM_MACOS)
        target_link_libraries(LuchEngineSample "-framework Cocoa")
        target_link_libraries(LuchEngineSample "-framework AppKit")
    endif()

    if(LUCH_PLATFORM_IOS)
        target_link_libraries(LuchEngineSample "-framework OpenGLES")
        target_link_libraries(LuchEngineSample "-framework UIKit")
        target_link_libraries(LuchEngineSample "-framework CoreGraphics")
        target_link_libraries(LuchEngineSample "-framework CoreMotion")
        target_link_libraries(LuchEngineSample "-framework MediaPlayer")
        target_link_libraries(LuchEngineSample "-framework AVFoundation")
    endif()
endif()

target_link_libraries(LuchEngineSample LuchEngine)

if(LUCH_PLATFORM_WIN32)
    set(LUCH_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/_build)
    set(LUCH_OUTPUT_DIRECTORY_DEBUG ${LUCH_OUTPUT_DIRECTORY}/Debug)
    set(LUCH_OUTPUT_DIRECTORY_RELEASE ${LUCH_OUTPUT_DIRECTORY}/Release)
    
    set_target_properties(LuchEngine PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG LUCH_OUTPUT_DIRECTORY_DEBUG)
    set_target_properties(LuchEngineSample PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_RELEASE LUCH_OUTPUT_DIRECTORY_RELEASE)

    set_target_properties(LuchEngineSample PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY ${LUCH_OUTPUT_DIRECTORY}/$(Configuration)) #fuck cmake for making me do this shit
endif()
