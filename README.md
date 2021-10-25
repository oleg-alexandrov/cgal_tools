# cgal_tools

This repository contains simple tools that use CGAL to remesh, smooth,
and fill holes in meshes. They can be used under the same terms as
CGAL itself, which is usually some mix of GPL and LGPL. Anything in
this repository not strictly derived from CGAL (which does not amount
to much) can be used under the Apache II license.

## Prerequisites

We assume a system that has Boost, Eigen3, MPFR, and GMP. These can be
installed with conda, if necessary.

# Fetch GDAL. It is header-only so it need not be built.

```
   mkdir -p $HOME/projects
   cd $HOME/projects
   
   # Note that CGAL version 5.3 is used (latest as of Fall 2021). 
   # CGAL can break APIs between versions.

   wget https://github.com/CGAL/cgal/releases/download/v5.3/CGAL-5.3.tar.xz
   tar xfv CGAL-5.3.tar.xz

```

# Build this repository

```
  cd $HOME/projects
  git clone https://github.com/oleg-alexandrov/cgal_tools.git 
  cd cgal_tools 

  cmake . -DCMAKE_BUILD_TYPE=Release -DCGAL_DIR:PATH=$HOME/projects/CGAL-5.3
  make -j 10
```

# Using the tools

See the actual invocations of these tools in geometry_mapper.py.

Mesh smoothing:

```
  ~/projects/cgal_tools/smoothe_mesh <num_iterations>  \
    <smoothing_time> <smoothe_boundary (1/0)>          \
    <input_mesh.ply> <output_mesh.ply>

```

Note that the above command may remove too much of the mesh if being
overly aggressive.

Hole-filling:

```
  ~/projects/cgal_tools/fill_holes               \
      <max_hole_diameter> <max_num_hole_edges>   \
      <input_mesh.ply> <output_mesh.ply>
```

Remove small connected components from the mesh:

```
  ~/projects/cgal_tools/rm_connected_components            \
    <num_components_to_keep> <num_min_faces_in_component>  \
    <input_mesh.ply> <output_mesh.ply>
```

Mesh simplification:

```
   ~/projects/cgal_tools/simplify_mesh_path <edge_keep_ratio> \
    <input_mesh.ply> <output_mesh.ply>
```

Remeshing (this is very slow and should be avoided. It needs porting
to CGAL 5.3, imitating the above tools, and added to the
CMakeLists.txt file):
 
```
  ~/projects/cgal_tools/remesh                     \
    <max_triangle_perimeter> <radius_ratio_bound>  \
    <input_mesh.ply> <output_mesh.ply> 
```

It is very strongly recommended to do these procedures first on small
meshes to get a feel for how they work.

