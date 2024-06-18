cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)

project(
  fastvis
  VERSION 0.0.1
  DESCRIPTION "RTTI library"
  LANGUAGES CXX)

add_library(fastvis INTERFACE)

target_include_directories(fastvis
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_sources(fastvis INTERFACE include/fastvis.hpp)

if(NOT PROJECT_IS_TOP_LEVEL)
  return()
endif()

add_executable(fastvis-test test/fastvis.t.cpp)

target_link_libraries(fastvis-test fastvis)
