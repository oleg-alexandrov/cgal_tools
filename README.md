# cgal_tools

This repository contains simple tools that use CGAL to remesh, smooth, and fill holes in meshes. They can be used under the same terms as CGAL itself, which is usually some mix of GPL and LGPL. Anything in this repository not strictly derived from CGAL (which does not amount to much) can be used under the Apache II license. 

## Prerequisites

We assume a system that has Boost, Eigen3, MPFR, GMP, and GFlags.

# Build GDAL

```
   mkdir -p $HOME/projects
   cd $HOME/projects
   git clone https://github.com/CGAL/cgal.git
   cd cgal
   # Later versions of CGAL will likely work too
   git checkout cf816d2 
```

# Build this repository

```
  cd $HOME/projects
  git clone https://github.com/oleg-alexandrov/cgal_tools.git 
  cd cgal_tools 
  mkdir -p build
  cd build

  cmake .. -DCMAKE_BUILD_TYPE=Release -DCGAL_DIR=$HOME/projects/cgal
  make -j 10
```

# Using the tools

 - Remeshing. (This is very slow and should be avoided.)
 
```
  ~/projects/cgal_tools/build/remesh                    \
      -input_mesh in_mesh.ply -output_mesh out_mesh.ply \
      -max_triangle_perimeter 0.3 -radius_ratio_bound 5
```

- Hole-filling:

```
  ~/projects/cgal_tools/build/fill_holes                \
      -input_mesh in_mesh.ply -output_mesh out_mesh.ply \
      -max_hole_diameter 0.4 -max_num_hole_edges 1000
```
 - Mesh smoothing:

```
  ~/projects/cgal_tools/build/smoothe_mesh             \
    -input_mesh in_mesh.ply -output_mesh out_mesh.ply  \
    -num_iterations 1 -smoothing_time 0.0001
```

Remove small connected components from the mesh:


```
  ~/projects/cgal_tools/build/rm_connected_components        \
    -input_mesh in_mesh.ply -output_mesh out_mesh.ply        \
    -num_components_to_keep 5 -num_min_faces_in_component 100
```

Note that the above command may remove too much of the mesh if being
overly aggressive.

It is very strongly recommended to do these procedures first on small
meshes to get a feel for how they work.

