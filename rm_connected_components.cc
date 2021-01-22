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
#include <CGAL/IO/PLY_reader.h>
#include <CGAL/IO/PLY_writer.h>
#include <CGAL/Polygon_mesh_processing/orient_polygon_soup.h>
#include <CGAL/Polygon_mesh_processing/polygon_soup_to_polygon_mesh.h>
#include <CGAL/Polygon_mesh_processing/repair_polygon_soup.h>

#include <iostream>
#include <fstream>
#include <map>

#include <gflags/gflags.h>

DEFINE_string(input_mesh, "",
              "The input mesh file.");
DEFINE_string(output_mesh, "",
              "The output mesh file.");

DEFINE_int32(num_min_faces_in_component, -1,
             "Keep only connected mesh components with at least this many faces.");

DEFINE_int32(num_components_to_keep, -1,
             "How many of the largest connected components of the mesh to keep. Being too aggressive here can result in a mesh with missing parts.");

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
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  if (FLAGS_input_mesh == "" || FLAGS_output_mesh == "") {
    std::cout << "The input and/or output mesh was not specified." << std::endl;
    return 1;
  }
  
  std::cout << "Reading: " << FLAGS_input_mesh << std::endl;
  std::ifstream in(FLAGS_input_mesh);

  // Read the mesh
  std::vector<Kernel::Point_3> points;
  std::vector< std::vector<std::size_t> > polygons;
  std::vector<CGAL::Color> fcolors;
  std::vector<CGAL::Color> vcolors;
  if (!in || !CGAL::read_PLY(in, points, polygons, fcolors, vcolors) || points.empty()) {
    std::cout << "Cannot open file: " << FLAGS_input_mesh << std::endl;
    return EXIT_FAILURE;
  }

  // Clean up the mesh
  Mesh mesh;
  PMP::repair_polygon_soup(points, polygons);
  PMP::orient_polygon_soup(points, polygons);
  PMP::polygon_soup_to_polygon_mesh(points, polygons, mesh);
  
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
  if (FLAGS_num_min_faces_in_component > 0) {
    PMP::keep_large_connected_components
      (mesh, FLAGS_num_min_faces_in_component,
       PMP::parameters::edge_is_constrained_map(Constraint<Mesh>(mesh, bound)));
    
    // Without this the mesh will be invalid
    mesh.collect_garbage();
  }
  
  if (FLAGS_num_components_to_keep > 0) {
    PMP::keep_largest_connected_components
      (mesh, FLAGS_num_components_to_keep,
       PMP::parameters::edge_is_constrained_map(Constraint<Mesh>(mesh, bound)));

    // Without this the mesh will be invalid
    mesh.collect_garbage();
  }
  
  // Save the processed mesh
  std::cout << "Writing: " << FLAGS_output_mesh << std::endl;
  std::ofstream out(FLAGS_output_mesh);
  out.precision(17);
  CGAL::write_PLY(out, mesh);
  
  return 0;
}
