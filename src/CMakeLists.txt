cmake_minimum_required(VERSION 3.16)

project(NestdaqUnpacker LANGUAGES CXX)

add_library(NestdaqUnpacker STATIC
  NodeUnpacker.cc
  LeafProcessor.cc
  LeafDecoder.cc
  NodeHeaderDecoder.cc
)

target_compile_features(NestdaqUnpacker PUBLIC cxx_std_17)

target_include_directories(NestdaqUnpacker
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)
