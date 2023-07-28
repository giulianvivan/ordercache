cmake_minimum_required(VERSION 3.18.4)

project(
  ORDERCACHE_TEST
  VERSION 1.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-pedantic-errors)

set(TARGET_NAME ordercache-test)

add_library(${TARGET_NAME}-lib OrderCache.cpp OrderCache.h)

add_executable(${TARGET_NAME} main.cpp)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(${TARGET_NAME} PRIVATE ${TARGET_NAME}-lib Threads::Threads)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
