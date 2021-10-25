/* Copyright (c) 2017, United States Government, as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 *
 * All rights reserved.
 *
 * The Astrobee platform is licensed under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with the
 * License. You may obtain a copy of the License at
 *
 *     http:  //  www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

// This code uses CGAL that is licensed under the GPL. This implements
// a standalone tool, not connect to the rest of Astrobee.

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Polygon_mesh_processing/connected_components.h>
#include <boost/function_output_iterator.hpp>
#include <boost/property_map/property_map.hpp>
#include <CGAL/IO/PLY.h>
#include <CGAL/Polygon_mesh_processing/IO/polygon_mesh_io.h>

#include <iostream>
#include <fstream>
#include <map>

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef Kernel::Point_3                                     Point;
typedef Kernel::Compare_dihedral_angle_3                    Compare_dihedral_angle_3;
typedef CGAL::Surface_mesh<Point>                           Mesh;

namespace PMP = CGAL::Polygon_mesh_processing;

template <typename G>
struct Constraint : public boost::put_get_helper<bool,Constraint<G> > {
  typedef typename boost::graph_traits<G>::edge_descriptor edge_descriptor;
  typedef boost::readable_property_map_tag      category;
  typedef bool                                  value_type;
  typedef bool                                  reference;
  typedef edge_descriptor                       key_type;
  Constraint()
    :g_(NULL)
  {}
  Constraint(G& g, double bound)
    : g_(&g), bound_(bound)
  {}
  bool operator[](edge_descriptor e) const
  {
    const G& g = *g_;
    return compare_(g.point(source(e, g)),
                    g.point(target(e, g)),
                    g.point(target(next(halfedge(e, g), g), g)),
                    g.point(target(next(opposite(halfedge(e, g), g), g), g)),
                   bound_) == CGAL::SMALLER;
  }
  const G* g_;
  Compare_dihedral_angle_3 compare_;
  double bound_;
};
template <typename PM>
struct Put_true {
  Put_true(const PM pm)
    :pm(pm)
  {}
  template <typename T>
  void operator()(const T& t)
  {
    put(pm, t, true);
  }
  PM pm;
};

int main(int argc, char* argv[]) {

  if (argc < 5) {
    std::cout << "Usage: " << argv[0]
              << "  num_min_faces_in_component num_components_to_keep input.ply output.ply\n";

    return 1;
  }
  
  int num_min_faces_in_component = atoi(argv[1]);
  int num_components_to_keep     = atoi(argv[2]);
  const char* input_file         = argv[3];
  const char* output_file        = argv[4];

  std::cout << "Reading mesh:       " << input_file << std::endl;
  std::cout << "Max num hole edges: " << num_min_faces_in_component << "\n";
  std::cout << "Max hole diameter:  " << num_components_to_keep << "\n";
  


  if (std::string(input_file) == "" || std::string(output_file) == "") {
    std::cout << "The input and/or output mesh was not specified." << std::endl;
    return 1;
  }
  
  std::cout << "Reading: " << input_file << std::endl;

  Mesh mesh;
  if(!PMP::IO::read_polygon_mesh(input_file, mesh)) {
    std::cerr << "Invalid input." << std::endl;
    return 1;
  }
  
  typedef boost::graph_traits<Mesh>::face_descriptor face_descriptor;
  const double bound = std::cos(0.75 * CGAL_PI);
  std::vector<face_descriptor> cc;
  face_descriptor fd = *faces(mesh).first;
  PMP::connected_component(fd,
      mesh,
      std::back_inserter(cc));

  // Instead of writing the faces into a container, you can set a face property to true
  typedef Mesh::Property_map<face_descriptor, bool> F_select_map;
  F_select_map fselect_map =
    mesh.add_property_map<face_descriptor, bool>("f:select", false).first;
  PMP::connected_component(fd,
      mesh,
      boost::make_function_output_iterator(Put_true<F_select_map>(fselect_map)));
  
  Mesh::Property_map<face_descriptor, std::size_t> fccmap =
    mesh.add_property_map<face_descriptor, std::size_t>("f:CC").first;
  std::size_t num = PMP::connected_components(mesh,
      fccmap,
      PMP::parameters::edge_is_constrained_map(Constraint<Mesh>(mesh, bound)));
  std::cout << "The mesh has " << num << " connected components.\n";
  
  typedef std::map<std::size_t/*index of CC*/, unsigned int/*nb*/> Components_size;
  Components_size nb_per_cc;
  for(face_descriptor f : faces(mesh)){
    nb_per_cc[ fccmap[f] ]++;
  }
  
  //for(const Components_size::value_type& cc : nb_per_cc){
  //   std::cout << "\t CC #" << cc.first
  //            << " is made of " << cc.second << " faces" << std::endl;
  // }
  
  // Keep only components with at least this many faces
  if (num_min_faces_in_component > 0) {
    PMP::keep_large_connected_components
      (mesh, num_min_faces_in_component,
       PMP::parameters::edge_is_constrained_map(Constraint<Mesh>(mesh, bound)));
    
    // Without this the mesh will be invalid
    mesh.collect_garbage();
  }
  
  if (num_components_to_keep > 0) {
    PMP::keep_largest_connected_components
      (mesh, num_components_to_keep,
       PMP::parameters::edge_is_constrained_map(Constraint<Mesh>(mesh, bound)));

    // Without this the mesh will be invalid
    mesh.collect_garbage();
  }
  
  std::cout << "Writing output mesh: " << output_file << std::endl;
  CGAL::IO::write_PLY(output_file, mesh,
                      CGAL::parameters::stream_precision(17).use_binary_mode(false));
  
  return 0;
}
