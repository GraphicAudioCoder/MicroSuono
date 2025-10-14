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

add_executable(MicroSuono
  src/main.cpp 
  src/miniaudio_impl.cpp
  src/core/GraphManager.cpp
  src/audio/AudioEngine.cpp
  src/nodes/SineNode.cpp
  src/nodes/GainNode.cpp
  src/nodes/ThresholdNode.cpp
)
