cmake_minimum_required (VERSION 3.0)
project (emp-zk-set)
set(NAME "emp-zk-set")

find_path(CMAKE_FOLDER NAMES cmake/emp-tool-config.cmake)
include(${CMAKE_FOLDER}/cmake/emp-base.cmake)

find_package(emp-zk REQUIRED)
include_directories(${EMP-ZK_INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -ggdb -Wno-ignored-attributes")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -Wno-ignored-attributes")

install(DIRECTORY emp-zk-set DESTINATION include/)
install(FILES cmake/emp-zk-set-config.cmake DESTINATION cmake/)

# Installation

macro (add_test _name)
   add_executable(${_name} "test/${_name}.cpp")
   target_link_libraries(${_name} ${EMP-ZK_LIBRARIES})
endmacro()

add_test(svole)
add_test(set_equality)
add_test(set_utils)
add_test(set_subset)
add_test(set_disjoint)
