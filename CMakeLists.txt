cmake_minimum_required(VERSION 3.17...3.20)
project(Polygon_mesh_processing_Examples)

# CGAL and its components
find_package(CGAL REQUIRED)

# Boost and its components
find_package(Boost REQUIRED)

if(NOT Boost_FOUND)
  message(STATUS "This project requires the Boost library, and will not be compiled.")
  return()
endif()

find_package(OpenMesh QUIET)
if(OpenMesh_FOUND)
  include(UseOpenMesh)
else()
  message(STATUS "Examples that use OpenMesh will not be compiled.")
endif()

find_package(Eigen3 3.2.0) # requires 3.2.0 or greater
include(CGAL_Eigen3_support)

# Creating entries for all main programs

if(TARGET CGAL::Eigen3_support)
  create_single_source_cgal_program("fill_holes.cc")
  target_link_libraries(fill_holes PUBLIC CGAL::Eigen3_support)

  create_single_source_cgal_program("smoothe_mesh.cc")
  target_link_libraries(smoothe_mesh PUBLIC CGAL::Eigen3_support)
  
  create_single_source_cgal_program("rm_connected_components.cc")
  target_link_libraries(rm_connected_components PUBLIC CGAL::Eigen3_support)

  create_single_source_cgal_program("simplify_mesh.cc")
  target_link_libraries(simplify_mesh PUBLIC CGAL::Eigen3_support)
  
endif()

