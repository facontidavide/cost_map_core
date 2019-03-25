
project(cost_map_core)

include_directories(
  include
  ${EIGEN3_INCLUDE_DIR}
)

############################################################################
# Sources
##############################################################################

file(GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.cpp)
file(GLOB ITERATOR_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/iterators/*.cpp)
file(GLOB OPERATOR_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/operators/*.cpp)

##############################################################################
# Library
##############################################################################

add_library(${PROJECT_NAME} ${SOURCES} ${ITERATOR_SOURCES} ${OPERATOR_SOURCES})
target_link_libraries(${PROJECT_NAME} ${catkin_LIBRARIES})

