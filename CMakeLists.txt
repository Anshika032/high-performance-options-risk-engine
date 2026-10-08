cmake_minimum_required(VERSION 3.16)
project(high_performance_options_risk_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(pricing_engine src/pricing_engine.cpp)
target_include_directories(pricing_engine PUBLIC include)
target_compile_options(pricing_engine PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
target_link_libraries(pricing_engine PUBLIC Threads::Threads)

find_package(Threads REQUIRED)

add_executable(options_engine src/main.cpp)
target_link_libraries(options_engine PRIVATE pricing_engine)

enable_testing()
add_executable(test_pricing tests/test_pricing.cpp)
target_link_libraries(test_pricing PRIVATE pricing_engine)
add_test(NAME pricing_regression COMMAND test_pricing)

add_executable(benchmark benchmarks/benchmark.cpp)
target_link_libraries(benchmark PRIVATE pricing_engine)
