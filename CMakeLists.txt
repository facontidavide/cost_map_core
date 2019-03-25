##############################################################################
# Cmake
##############################################################################

cmake_minimum_required(VERSION 2.8.3)

set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")

## Define Eigen addons.
include(cmake/grid_map_core-extras.cmake)

## System dependencies are found with CMake's conventions
#find_package(Eigen3 REQUIRED)
# Solution to find Eigen3 with Saucy.
find_package(Eigen3 REQUIRED)



add_subdirectory(grid_map_core)
include_directories(grid_map_core/include)

add_subdirectory(cost_map_core)


