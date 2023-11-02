cmake_minimum_required(VERSION 3.22)
project(HappyNumbersCalculatorCPP)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")

add_executable(HappyNumbersCalculatorCPP HnCalculator.cpp)
