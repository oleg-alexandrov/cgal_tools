# Created by the script cgal_create_cmake_script
# This is the CMake script for compiling a CGAL application.

cmake_minimum_required(VERSION 3.0)

project(cgal_tools)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Eigen3 REQUIRED)
if (EIGEN3_FOUND)
    message(STATUS "Eigen3 found.")
else()
    message(FATAL_ERROR "Eigen3 not found.")
endif()

include_directories(${EIGEN3_INCLUDE_DIR})
set(BUILD_SHARED_LIBS ON)

find_package(Gflags REQUIRED)
set(CGAL_3RD_PARTY_LIBRARIES ${GFLAGS_LIBRARIES})

find_package(CGAL REQUIRED OPTIONAL_COMPONENTS Core)

set(EIGEN3_USE_FILE "UseEigen3")
include( ${EIGEN3_USE_FILE} )

create_single_source_cgal_program("remesh.cc")
create_single_source_cgal_program("smoothe_mesh.cc")
create_single_source_cgal_program("fill_holes.cc")


