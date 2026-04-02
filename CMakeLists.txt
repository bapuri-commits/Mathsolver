cmake_minimum_required(VERSION 3.20)
project(MathSolver VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/utf-8)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(MathSolver src/main.cpp "src/Matrix.hpp")

# Catch2 테스트 설정
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.4
)
FetchContent_MakeAvailable(Catch2)

enable_testing()
add_executable(MathSolverTests ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_matrix.cpp)
target_include_directories(MathSolverTests PRIVATE src)
target_link_libraries(MathSolverTests PRIVATE Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(MathSolverTests)
