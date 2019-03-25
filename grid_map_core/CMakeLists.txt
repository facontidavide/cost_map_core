cmake_minimum_required(VERSION 2.8.3)
project(grid_map_core)

include_directories(
  include
  ${EIGEN3_INCLUDE_DIR}
)

## Declare a cpp library
add_library(${PROJECT_NAME}
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
)

