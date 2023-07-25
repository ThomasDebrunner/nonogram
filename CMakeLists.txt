cmake_minimum_required(VERSION 3.21)
project(nonogram)

set(CMAKE_CXX_STANDARD 14)

add_executable(nonogram main.cpp DotNonParser.h)
