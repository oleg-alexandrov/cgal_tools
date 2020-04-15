# cgal_tools
This repository contains simple tools that use CGAL to remesh, smoothe, and fill holes in meshes. They can be used under the same terms as CGAL itself, which is usually some mix of GPL and LGPL. Anything in this repository not strictly derived from CGAL (which does not amount to much) can be used under the Apache II license. 

To use, first build CGAL.

  mkdir -p $HOME/projects
  cd $HOME/projects
  git clone https://github.com/CGAL/cgal.git
  cd cgal
  TODO(oalexan1): Later versions of CGAL will likely work too
  git checkout cf816d2 

Then clone and build this repository.
 
  cd $HOME/projects
  git clone 
 
  mkdir -p $HOME/freeflyer_build/native/cgal_build
  cd $HOME/freeflyer_build/native/cgal_build

  cmake $HOME/freeflyer/dense_map/mesh             \
    -DCMAKE_BUILD_TYPE=Release                     \
    -DCMAKE_MODULE_PATH=$HOME/freeflyer/cmake      \
    -DCGAL_DIR=$HOME/projects/cgal                 \
    -DEIGEN3_INCLUDE_DIR=/usr/include/eigen3

  make -j 10

Above we assume a system that has Boost, Eigen3, MPFR, GMP, and
GFlags.

Operations:

 - Remeshing. (This is very slow and should be avoided.)
 
   ~/freeflyer_build/native/cgal_build/remesh           \
      -input_mesh in_mesh.ply -output_mesh out_mesh.ply \
      -max_triangle_perimeter 0.3 -radius_ratio_bound 5

 - Hole-filling:

   ~/freeflyer_build/native/cgal_build/fill_holes       \
      -input_mesh in_mesh.ply -output_mesh out_mesh.ply \
      -max_hole_diameter 0.4 -max_num_hole_edges 1000

 - Mesh smoothing:

  ~/freeflyer_build/native/cgal_build/smoothe_mesh     \
    -input_mesh in_mesh.ply -output_mesh out_mesh.ply  \
    -num_iterations 1 -smoothing_time 0.0001

It is very strongly recommended to do these procedures first on small
meshes to get a feel for how they work.

