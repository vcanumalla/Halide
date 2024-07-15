cmake_minimum_required(VERSION 3.22)
project(sca)

enable_testing()

# Set up language settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

# Find Halide
find_package(Halide REQUIRED)
find_package(OpenMP)

# Generator
add_halide_generator(sca.generator SOURCES halide_sca_generator.cpp)

# Filters
add_halide_library(halide_sca FROM sca.generator)

# Main executable
add_executable(sca_test test.cpp)
target_compile_options(sca_test PRIVATE $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-O2>)
target_link_libraries(sca_test
                      PRIVATE
                      Halide::Tools
                      halide_sca
                      $<TARGET_NAME_IF_EXISTS:OpenMP::OpenMP_CXX>)

# Test that the app actually works!
add_test(NAME sca_app COMMAND sca_test)
set_tests_properties(sca_app PROPERTIES
                     LABELS sca
                     PASS_REGULAR_EXPRESSION "Success!"
                     SKIP_REGULAR_EXPRESSION "\\[SKIP\\]")
