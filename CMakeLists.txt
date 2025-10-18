cmake_minimum_required(VERSION 3.10)
project(MicroSuono)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Genera automaticamente compile_commands.json per clangd/LSP
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(
  ${CMAKE_SOURCE_DIR}/include 
  ${CMAKE_SOURCE_DIR}/include/MicroSuono
)

add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}/*
    COMMENT "Cleaning all build files"
)

# Create a static library with all common code
add_library(MicroSuonoLib STATIC
  src/miniaudio_impl.cpp
  src/core/Node.cpp
  src/core/GraphManager.cpp
  src/audio/AudioEngine.cpp
  src/nodes/SineNode.cpp
  src/nodes/GainNode.cpp
  src/nodes/ThresholdNode.cpp
  src/nodes/MonoToStereoNode.cpp
  src/nodes/AudioInputNode.cpp
  src/MicroScript/Token.cpp
)

# Main executable
add_executable(MicroSuono src/main.cpp)
target_link_libraries(MicroSuono MicroSuonoLib)

# Demo executables
add_executable(demo1_direct_stereo examples/demo1_direct_stereo.cpp)
target_link_libraries(demo1_direct_stereo MicroSuonoLib)

add_executable(demo2_mono_to_stereo examples/demo2_mono_to_stereo.cpp)
target_link_libraries(demo2_mono_to_stereo MicroSuonoLib)

add_executable(demo3_audio_input examples/demo3_audio_input.cpp)
target_link_libraries(demo3_audio_input MicroSuonoLib)

add_executable(demo4_tremolo examples/demo4_tremolo.cpp)
target_link_libraries(demo4_tremolo MicroSuonoLib)

add_executable(demo5_summation examples/demo5_summation.cpp)
target_link_libraries(demo5_summation MicroSuonoLib)
