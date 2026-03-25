cmake_minimum_required(VERSION 3.20)
project(MathSolver VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MSVC UTF-8 인코딩 강제
if(MSVC)
    add_compile_options(/utf-8)
endif()

# 빌드 결과물 경로
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
