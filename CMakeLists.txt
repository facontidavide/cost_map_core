##############################################################################
# Cmake
##############################################################################

cmake_minimum_required(VERSION 2.8.3)
project(grid_map)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

## Define Eigen addons.
include(cmake/grid_map_core-extras.cmake)
find_package(Eigen3 REQUIRED)

set(CMAKE_AUTOMOC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets)

set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS} -fPIC")

include_directories(
  include
  ${Qt5Widgets_INCLUDE_DIRS}
  ${EIGEN3_INCLUDE_DIR}
)

## Declare a cpp library
add_library(grid_map
   src/GridMap.cpp
   src/GridMapMath.cpp
   src/SubmapGeometry.cpp
   src/BufferRegion.cpp
   src/Polygon.cpp
   src/iterators/GridMapIterator.cpp
   src/iterators/SubmapIterator.cpp
   src/iterators/CircleIterator.cpp
   src/iterators/EllipseIterator.cpp
   src/iterators/SpiralIterator.cpp
   src/iterators/PolygonIterator.cpp
   src/iterators/LineIterator.cpp
   src/iterators/SlidingWindowIterator.cpp
   src/operators/Inflation.cpp

   include/grid_map/visualization/qt_display.hpp
   src/visualization/qt_display.cpp
)

add_executable(gridmap_sandbox example/gridmap_sandbox.cpp)
target_link_libraries(gridmap_sandbox Qt5::Widgets grid_map)



