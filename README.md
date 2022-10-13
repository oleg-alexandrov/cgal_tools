This repository contains simple tools that use CGAL to remesh, smooth,
and fill holes in meshes. They can be used under the same terms as
CGAL itself, which is usually some mix of GPL and LGPL. Anything in
this repository not strictly derived from CGAL (which does not amount
to much) can be used under the Apache II license.

# Prerequisites

We assume a system that has Boost, Eigen3, MPFR, and GMP. These can be
installed with conda, if necessary, for example, as:

    conda create -n cgal -c conda-forge gmp=6.2.1 mpfr=4.1.0    \
      boost=1.68.0 eigen=3.4.0 cmake=3.15.5 gxx_linux-64=11.2.0 
    conda activate cgal

Type:

    which cmake

and verify that it is found in the just-created environment.

# Fetch GDAL

CGAL is header-only so it need not be built.

Note that CGAL version 5.3 is used (latest as of Fall 2021). 
CGAL can break APIs between versions.

    mkdir -p $HOME/projects
    cd $HOME/projects

    wget https://github.com/CGAL/cgal/releases/download/v5.3/CGAL-5.3.tar.xz
    tar xfv CGAL-5.3.tar.xz

# Build this repository

    cd $HOME/projects
    git clone https://github.com/oleg-alexandrov/cgal_tools.git 
    cd cgal_tools 
    cmake . -DCMAKE_BUILD_TYPE=Release        \
      -DCGAL_DIR:PATH=$HOME/projects/CGAL-5.3 \
      -DCMAKE_CXX_COMPILER=$HOME/miniconda3/envs/cgal/bin/x86_64-conda_cos6-linux-gnu-g++
    make -j 10

# Using the tools

Mesh smoothing:

    num_iter=1; smoothing_time=0.00005; smoothe_boundary=1
    ~/projects/cgal_tools/smoothe_mesh          \
    $num_iter $smoothing_time $smoothe_boundary \
      <input_mesh.ply> <output_mesh.ply>

Note that the above command may remove too much of the mesh if being
overly aggressive with parameters.

Hole-filling:

    max_hole_diameter=0.4
    max_num_hole_edges=1000
    ~/projects/cgal_tools/fill_holes         \
      $max_hole_diameter $max_num_hole_edges \
      <input_mesh.ply> <output_mesh.ply>

Remove small connected components from the mesh:

    num_min_faces_in_component=1000
    num_components_to_keep=1
    ~/projects/cgal_tools/rm_connected_components \
      $num_min_faces_in_component                 \
      $num_components_to_keep                     \
      <input_mesh.ply> <output_mesh.ply>

Mesh simplification:

    edge_keep_ratio=0.2
    ~/projects/cgal_tools/simplify_mesh_path $edge_keep_ratio \
      <input_mesh.ply> <output_mesh.ply>

Remeshing (this is very slow and should be avoided). This tool is
broken for now. It needs porting to CGAL 5.3, imitating the above
tools, and added to the CMakeLists.txt file.
 
    ~/projects/cgal_tools/remesh                     \
      <max_triangle_perimeter> <radius_ratio_bound>  \
      <input_mesh.ply> <output_mesh.ply> 

It is very strongly recommended to do these procedures first on small
meshes to get a feel for how they work.

