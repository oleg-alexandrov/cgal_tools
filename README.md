This repository contains tools that use CGAL to smoothe, fill holes,
and remove connected components in meshes in .ply format, and also to
simplify them. They can be used under the same terms as CGAL itself,
which is usually some mix of GPL and LGPL. Anything in this repository
not strictly derived from CGAL (which does not amount to much) can be
used under the Apache II license.

# Prerequisites

We assume a system that has Boost, Eigen3, MPFR, GMP, cmake, and a C++
compiler. These can be installed with conda, if necessary, for
example, as:

    conda create -n cgal -c conda-forge gmp=6.2.1 mpfr=4.1.0    \
      boost=1.68.0 eigen=3.4.0 cmake=3.15.5 gxx_linux-64=11.2.0 
    conda activate cgal

Type:

    which cmake

and verify that it is found in the just-created environment.

# Build this repository

This will fetch CGAL-5.3, but not build it, as it is header-only. 

    git clone https://github.com/oleg-alexandrov/cgal_tools.git 
    cd cgal_tools
    mkdir -p build
    cd build 
    cmake .. -DCMAKE_BUILD_TYPE=Release       \
      -DCMAKE_CXX_COMPILER=$HOME/miniconda3/envs/cgal/bin/x86_64-conda_cos6-linux-gnu-g++
    make -j 10

If desired to install these tools to a certain location, pass to the
cmake invocation above:

    -DCGAL_TOOLS_INSTALL_DIR=/your/install/dir

In that case, the compiled programs will be copied to the `bin`
subdirectory of the installation path when running `make install`.

# Using the tools

Mesh smoothing:

    num_iter=1; smoothing_time=0.00005; smoothe_boundary=1
    cgal_tools/build/smoothe_mesh               \
      $num_iter $smoothing_time $smoothe_boundary \
      <input_mesh.ply> <output_mesh.ply>

Note that the above command may remove too much of the mesh if being
overly aggressive with parameters.

Hole-filling:

    max_hole_diameter=0.4
    max_num_hole_edges=1000
    cgal_tools/build/fill_holes              \
      $max_hole_diameter $max_num_hole_edges \
      <input_mesh.ply> <output_mesh.ply>

Remove small connected components from the mesh:

    num_min_faces_in_component=1000
    num_components_to_keep=1
    cgal_tools/build/rm_connected_components \
      $num_min_faces_in_component            \
      $num_components_to_keep                \
      <input_mesh.ply> <output_mesh.ply>

Mesh simplification:

    edge_keep_ratio=0.2
    cgal_tools/build/simplify_mesh $edge_keep_ratio \
      <input_mesh.ply> <output_mesh.ply>

It is very strongly recommended to first run these tools on small
meshes to get a feel for how they work. Meshlab can be used
to inspect the results.

