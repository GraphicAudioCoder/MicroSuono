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
  src/nodes/MixerNode.cpp
  src/nodes/SliderNode.cpp
  src/nodes/SineOscillatorNode.cpp
  src/MicroScript/Token.cpp
  src/MicroScript/Lexer.cpp
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

add_executable(demo7_mixer_vectors examples/demo7_mixer_vectors.cpp)
target_link_libraries(demo7_mixer_vectors MicroSuonoLib)

add_executable(demo8_beating examples/demo8_beating.cpp)
target_link_libraries(demo8_beating MicroSuonoLib)

add_executable(demo7_record_wav examples/demo7_record_wav.cpp)
target_link_libraries(demo7_record_wav MicroSuonoLib)

add_executable(demo9_dynamic_connections examples/demo9_dynamic_connections.cpp)
target_link_libraries(demo9_dynamic_connections MicroSuonoLib)

add_executable(demo10_live_patching examples/demo10_live_patching.cpp)
target_link_libraries(demo10_live_patching MicroSuonoLib)

add_executable(demo11_slider_control examples/demo11_slider_control.cpp)
target_link_libraries(demo11_slider_control MicroSuonoLib)

add_executable(demo_param_types examples/demo_param_types.cpp)
target_link_libraries(demo_param_types MicroSuonoLib)

# Test executables
add_executable(test_lexer examples/test_lexer.cpp)
target_link_libraries(test_lexer MicroSuonoLib)

add_executable(test_node_refactoring examples/test_node_refactoring.cpp)
target_link_libraries(test_node_refactoring MicroSuonoLib)
