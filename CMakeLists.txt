#Config

cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1 -mxsave -finput-charset=utf-8 -Wl,--unresolved-symbols=ignore-in-object-files")

option(USE_AVX512 "Enable experimental AVX-512 support" OFF)

build_command(b2, cxxstd=17)

add_definitions(-DBOOST_ERROR_CODE_HEADER_ONLY)
add_definitions(-DBOOST_SYSTEM_NO_DEPRECATED)

SET (CMAKE_CXX_COMPILER "/usr/bin/clang-14")

set(MOC_AVX512_FILES Engine/OcclusionCulling/MaskedOcclusionCullingAVX512.cpp)
set(MOC_AVX2_FILES Engine/OcclusionCulling/MaskedOcclusionCullingAVX2.cpp)
set(MOC_SSE_FILES Engine/OcclusionCulling/MaskedOcclusionCulling.cpp Engine/OcclusionCulling/CullingThreadpool.cpp)
set(MOC_INCLUDE_FILES Engine/OcclusionCulling/MaskedOcclusionCulling.h Engine/OcclusionCulling/CullingThreadpool.h Engine/OcclusionCulling/CompilerSpecific.inl Engine/OcclusionCulling/MaskedOcclusionCullingCommon.inl)
set(MOC_FILES ${MOC_AVX512_FILES} ${MOC_AVX2_FILES} ${MOC_SSE_FILES} ${MOC_INCLUDE_FILES})

SET_SOURCE_FILES_PROPERTIES(${MOC_AVX512_FILES} PROPERTIES COMPILE_FLAGS "/arch:AVX2")
SET_SOURCE_FILES_PROPERTIES(${MOC_AVX2_FILES} PROPERTIES COMPILE_FLAGS "/arch:AVX2")
SET_SOURCE_FILES_PROPERTIES(${MOC_AVX512_FILES} PROPERTIES COMPILE_FLAGS "-mavx2 -mfma -msse4.1")
SET_SOURCE_FILES_PROPERTIES(${MOC_AVX2_FILES} PROPERTIES COMPILE_FLAGS "-mavx2 -mfma -msse4.1")
SET_SOURCE_FILES_PROPERTIES(${MOC_SSE_FILES} PROPERTIES COMPILE_FLAGS "-msse4.1")
SET_SOURCE_FILES_PROPERTIES(${MOC_AVX512_FILES} PROPERTIES COMPILE_FLAGS "-DUSE_AVX512=1 /arch:AVX2")
SET_SOURCE_FILES_PROPERTIES(${MOC_AVX512_FILES} PROPERTIES COMPILE_FLAGS "/arch:AVX2")
SET_SOURCE_FILES_PROPERTIES(${MOC_AVX2_FILES} PROPERTIES COMPILE_FLAGS "/arch:AVX2")
SET_SOURCE_FILES_PROPERTIES(${MOC_AVX512_FILES} PROPERTIES COMPILE_FLAGS "-mavx2 -mfma -msse4.1")
SET_SOURCE_FILES_PROPERTIES(${MOC_AVX2_FILES} PROPERTIES COMPILE_FLAGS "-mavx2 -mfma -msse4.1")
SET_SOURCE_FILES_PROPERTIES(${MOC_SSE_FILES} PROPERTIES COMPILE_FLAGS "-msse4.1")

project(FalcoEngine_bgfx)
add_library(Engine STATIC ${SOURCES})
add_executable(Player ${SOURCES})
add_executable(Editor ${SOURCES})

add_dependencies(Player Engine)
add_dependencies(Editor Engine)
add_dependencies(Editor Player)

set_target_properties(Editor PROPERTIES OUTPUT_NAME "Editor.app")
set_target_properties(Player PROPERTIES OUTPUT_NAME "Player.app")

set(CMAKE_GENERATOR_PLATFORM x64)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath='$ORIGIN/lib'")
set(OpenGL_GL_PREFERENCE GLVND)

#Sources

set(include_dirs ${PROJECT_BINARY_DIR}
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Boost>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Engine>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Mono/include>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Bgfx/bx/include>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Bgfx/bimg/include>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Bgfx/bgfx/include>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Bgfx/bgfx/3rdparty/dear-imgui>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/LibZip/include>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/OpenAL/include>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/codecs/audio/ogg>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/codecs/video/ffmpeg/include>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/SDL>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Bullet/include>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Engine/Navigation/DebugUtils/Include>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Engine/Navigation/Detour/Include>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Engine/Navigation/DetourCrowd/Include>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Engine/Navigation/DetourTileCache/Include>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Engine/Navigation/Recast/Include>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/FreeType/include>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Assimp/include>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Carve/include>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Bgfx/bgfx/3rdparty>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Bgfx/bgfx/3rdparty/fcpp>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Bgfx/bgfx/3rdparty/glsl-optimizer/src/glsl>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Bgfx/bgfx/3rdparty/glslang>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Bgfx/bgfx/3rdparty/glslang/StandAlone>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Bgfx/bgfx/3rdparty/glslang/glslang/Public>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Bgfx/bgfx/3rdparty/glslang/glslang/include>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Bgfx/bgfx/3rdparty/spirv-cross>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Bgfx/bgfx/3rdparty/spirv-cross/include>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Bgfx/bgfx/3rdparty/spirv-tools/include>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Bgfx/bgfx/3rdparty/webgpu/include>
                    )

find_package(OpenGL REQUIRED)

set(include_libs "${CMAKE_CURRENT_SOURCE_DIR}/Linux/lib/libstdc++.so.6.0.30"
                    "${CMAKE_CURRENT_SOURCE_DIR}/Linux/lib/libm.so.6"
                    "${CMAKE_CURRENT_SOURCE_DIR}/Linux/lib/libc.so.6"
                    "${CMAKE_CURRENT_SOURCE_DIR}/Linux/lib/libz.so.1.2.11"
                    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/video/ffmpeg/lib/libavcodec.so.58"
                    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/video/ffmpeg/lib/libavdevice.so.58"
                    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/video/ffmpeg/lib/libavfilter.so.7"
                    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/video/ffmpeg/lib/libavformat.so.58"
                    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/video/ffmpeg/lib/libavutil.so.56"
                    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/video/ffmpeg/lib/libswresample.so.3"
                    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/video/ffmpeg/lib/libswscale.so.5"
                    "${CMAKE_CURRENT_SOURCE_DIR}/Boost/lib/linux/libboost_regex.so.1.74.0"
                    "${CMAKE_CURRENT_SOURCE_DIR}/Boost/lib/linux/libboost_serialization.so.1.74.0"
                    "${CMAKE_CURRENT_SOURCE_DIR}/Boost/lib/linux/libboost_system.so.1.74.0"
                    "${CMAKE_CURRENT_SOURCE_DIR}/Boost/lib/linux/libboost_filesystem.so.1.74.0"
                    "${CMAKE_CURRENT_SOURCE_DIR}/Boost/lib/linux/libboost_iostreams.so.1.74.0"
                    "${CMAKE_CURRENT_SOURCE_DIR}/Boost/lib/linux/libboost_thread.so.1.74.0"
                    "${CMAKE_CURRENT_SOURCE_DIR}/FreeType/lib/libfreetype.so.6.18.1"
                    ${OPENGL_LIBRARIES}
                    "${CMAKE_CURRENT_SOURCE_DIR}/SDL/lib/libSDL2.so"
                    "${CMAKE_CURRENT_SOURCE_DIR}/SDL/lib/libSDL2main.a"
                    "${CMAKE_CURRENT_SOURCE_DIR}/Assimp/lib/libassimp.so.5"
                    "${CMAKE_CURRENT_SOURCE_DIR}/LibZip/lib/libzip.so"
                    "${CMAKE_CURRENT_SOURCE_DIR}/FreeImage/lib/libfreeimage.so.3"
                    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/audio/ogg/lib/libvorbis.so"
                    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/audio/ogg/lib/libvorbisfile.so"
                    "${CMAKE_CURRENT_SOURCE_DIR}/codecs/audio/ogg/lib/libogg.so"
                    "${CMAKE_CURRENT_SOURCE_DIR}/Bgfx/bgfx/.build/linux64_gcc/bin/libbxRelease.a"
                    "${CMAKE_CURRENT_SOURCE_DIR}/Bgfx/bgfx/.build/linux64_gcc/bin/libbimgRelease.a"
                    "${CMAKE_CURRENT_SOURCE_DIR}/Bgfx/bgfx/.build/linux64_gcc/bin/libfcppRelease.a"
                    "${CMAKE_CURRENT_SOURCE_DIR}/Bgfx/bgfx/.build/linux64_gcc/bin/libglslangRelease.a"
                    "${CMAKE_CURRENT_SOURCE_DIR}/Bgfx/bgfx/.build/linux64_gcc/bin/libglsl-optimizerRelease.a"
                    "${CMAKE_CURRENT_SOURCE_DIR}/Bgfx/bgfx/.build/linux64_gcc/bin/libspirv-crossRelease.a"
                    "${CMAKE_CURRENT_SOURCE_DIR}/Bgfx/bgfx/.build/linux64_gcc/bin/libspirv-optRelease.a"
                    "${CMAKE_CURRENT_SOURCE_DIR}/Bgfx/bgfx/.build/linux64_gcc/bin/libbgfx-shared-libRelease.so"
                    "${CMAKE_CURRENT_SOURCE_DIR}/Bgfx/bgfx/.build/linux64_gcc/bin/libshadercRelease.so"
                    "${CMAKE_CURRENT_SOURCE_DIR}/Bullet/lib/libBullet3Common.so.3.24"
                    "${CMAKE_CURRENT_SOURCE_DIR}/Bullet/lib/libBulletCollision.so.3.24"
                    "${CMAKE_CURRENT_SOURCE_DIR}/Bullet/lib/libBulletDynamics.so.3.24"
                    "${CMAKE_CURRENT_SOURCE_DIR}/Bullet/lib/libBulletInverseDynamics.so.3.24"
                    "${CMAKE_CURRENT_SOURCE_DIR}/Bullet/lib/libBulletSoftBody.so.3.24"
                    "${CMAKE_CURRENT_SOURCE_DIR}/Bullet/lib/libLinearMath.so.3.24"
                    "${CMAKE_CURRENT_SOURCE_DIR}/OpenAL/lib/x64/libopenal.so.1.22.0"
                    "${CMAKE_CURRENT_SOURCE_DIR}/Carve/lib/libcarve.so.1.4"
                    "${CMAKE_CURRENT_SOURCE_DIR}/Mono/lib/libmonosgen-2.0.so"
                    "${CMAKE_CURRENT_SOURCE_DIR}/build/libEngine.a"
                    )

target_include_directories(Engine PUBLIC ${include_dirs})
target_include_directories(Player PUBLIC ${include_dirs})
target_include_directories(Editor PUBLIC ${include_dirs})

file(GLOB_RECURSE SRC_FILES
     "Engine/*.h"
     "Engine/*.cpp"
     "Engine/*.c"
     "Bgfx/bgfx/3rdparty/dear-imgui/*.h"
     "Bgfx/bgfx/3rdparty/dear-imgui/*.cpp"
     "Bgfx/bgfx/3rdparty/meshoptimizer/src/*.h"
     "Bgfx/bgfx/3rdparty/meshoptimizer/src/*.cpp"
     "codecs/textures/*.h"
     "codecs/textures/*.c"
     "codecs/textures/*.cpp"
     )

file(GLOB_RECURSE PLAYER_SRC_FILES
     "Player/*.h"
     "Player/*.cpp"
)

file(GLOB_RECURSE EDITOR_SRC_FILES
     "Editor/*.h"
     "Editor/*.cpp"
)

#Sources

target_sources(Engine PUBLIC ${SRC_FILES})
target_sources(Player PUBLIC ${PLAYER_SRC_FILES})
target_sources(Editor PUBLIC ${EDITOR_SRC_FILES})

#Libs

target_link_libraries(Player PUBLIC ${include_libs})
target_link_libraries(Editor PUBLIC ${include_libs})
