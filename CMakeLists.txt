cmake_minimum_required(VERSION 3.10)
project(parallel-pattern-matching LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

find_package(PkgConfig REQUIRED)
pkg_check_modules(CLI11 REQUIRED IMPORTED_TARGET CLI11)
pkg_check_modules(TBB REQUIRED IMPORTED_TARGET tbb)

add_subdirectory(src)