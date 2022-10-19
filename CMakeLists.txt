project(cgal_tools)

cmake_minimum_required(VERSION 3.11)

# Fetch CGAL. Do not compile it.
include(FetchContent)
FetchContent_Declare(CGAL
    URL https://github.com/CGAL/cgal/releases/download/v5.3/CGAL-5.3.tar.xz
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
)
Set(FETCHCONTENT_QUIET FALSE) # show progress dialog
FetchContent_Populate(CGAL)   # finish fetching and unpacking before continuing

# Find and use CGAL fetched earlier
set(CGAL_DIR "${FETCHCONTENT_BASE_DIR}/cgal-src")
find_package(CGAL REQUIRED)

# Boost and its components
find_package(Boost REQUIRED)

if(NOT Boost_FOUND)
  message(STATUS "This project requires the Boost library.")
  return()
endif()

find_package(Eigen3 3.2.0 REQUIRED) # 3.2.0 is minimum version
include(CGAL_Eigen3_support)

# Create individual tools
set(program_names "fill_holes;smoothe_mesh;rm_connected_components;simplify_mesh")
foreach(program IN LISTS program_names)

    # Build 
    create_single_source_cgal_program("${CMAKE_CURRENT_SOURCE_DIR}/${program}.cc")
    target_link_libraries(${program} PUBLIC CGAL::Eigen3_support)

    # Install if installation directory was set 
    if (CGAL_TOOLS_INSTALL_DIR)
        install(TARGETS ${program} RUNTIME DESTINATION "${CGAL_TOOLS_INSTALL_DIR}/bin")
    endif()
        
endforeach()



