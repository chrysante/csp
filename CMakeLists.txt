cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)

project(
  csp
  VERSION 0.0.1
  DESCRIPTION "RTTI library"
  LANGUAGES CXX)

add_library(csp INTERFACE)

target_include_directories(csp
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(PROJECT_IS_TOP_LEVEL) 
  # Add the source file if we are top level to make it show up in the IDE when developing
  target_sources(csp INTERFACE include/csp.hpp)
endif()

if(NOT PROJECT_IS_TOP_LEVEL)
  return()
endif()

add_executable(csp-test test/csp.t.cpp)

target_link_libraries(csp-test csp)

add_executable(animals-example
	examples/animals/animals.cpp
	examples/animals/animals.hpp)

target_link_libraries(animals-example csp)

add_executable(interpreter-example
	examples/interpreter/ast.hpp
	examples/interpreter/interpreter.cpp
	examples/interpreter/interpreter.hpp
	examples/interpreter/parser.cpp
	examples/interpreter/parser.hpp
	examples/interpreter/terminal.cpp
	examples/interpreter/terminal.hpp
	examples/interpreter/utils.cpp
	examples/interpreter/utils.hpp
	examples/interpreter/main.cpp)

target_include_directories(interpreter-example
	PRIVATE
	examples/interpreter)

target_link_libraries(interpreter-example csp)

add_executable(compiler-errors
               examples/compiler-errors/compiler-errors.cpp)

target_link_libraries(compiler-errors csp)

# Benchmark

include(FetchContent)
FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.6.1 
)
set(BENCHMARK_ENABLE_TESTING OFF)
set(BENCHMARK_USE_BUNDLED_GTEST OFF)
set(BENCHMARK_DOWNLOAD_DEPENDENCIES ON)
FetchContent_MakeAvailable(googlebenchmark)

add_executable(csp-benchmark)
target_sources(csp-benchmark PRIVATE benchmark/animals.hpp benchmark/main.cpp)
target_link_libraries(csp-benchmark csp benchmark::benchmark)
