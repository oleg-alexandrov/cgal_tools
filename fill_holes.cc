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

// This code uses CGAL that is licensed under the GPL. No good
// geometry library under a different license exists, as VCG is also
// GPL. This implements a standalone tool, not connect to the rest of
// Astrobee.

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/boost/graph/iterator.h>
#include <CGAL/Polygon_mesh_processing/triangulate_hole.h>
#include <CGAL/Bbox_3.h>
#include <CGAL/Polygon_mesh_processing/bbox.h>
#include <CGAL/IO/PLY_reader.h>
#include <CGAL/IO/PLY_writer.h>

#include <gflags/gflags.h>

#include <iostream>
#include <fstream>
#include <vector>

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef Kernel::Point_3                                     Point;
typedef CGAL::Surface_mesh<Point>                           Mesh;
typedef boost::graph_traits<Mesh>::halfedge_descriptor      halfedge_descriptor;
typedef boost::graph_traits<Mesh>::face_descriptor          face_descriptor;
typedef boost::graph_traits<Mesh>::vertex_descriptor        vertex_descriptor;

bool is_small_hole(halfedge_descriptor h, Mesh & mesh,
                   std::set<Point> & examined_points,
                   double max_hole_diam, int max_num_hole_edges) {
  int num_hole_edges = 0;
  auto cvpm = CGAL::get_const_property_map(CGAL::vertex_point, mesh);
  CGAL::Halfedge_around_face_circulator<Mesh> circ(h, mesh), done(circ);
  CGAL::Bbox_3 hole_bbox;

  do {
    Point p = get(cvpm, target(*circ, mesh));

    if (examined_points.find(p) != examined_points.end())
      return false;
    examined_points.insert(p);

    hole_bbox += CGAL::Bbox_3(p.x(), p.y(), p.z(), p.x(), p.y(), p.z());
    num_hole_edges++;

    // Exit early, to avoid unnecessary traversal of large holes
    if (num_hole_edges > max_num_hole_edges) return false;
    if (hole_bbox.xmax() - hole_bbox.xmin() > max_hole_diam) return false;
    if (hole_bbox.ymax() - hole_bbox.ymin() > max_hole_diam) return false;
    if (hole_bbox.zmax() - hole_bbox.zmin() > max_hole_diam) return false;
  } while (++circ != done);

  return true;
}

DEFINE_string(input_mesh, "",
              "The input mesh file.");
DEFINE_string(output_mesh, "",
              "The output mesh file.");
DEFINE_double(max_hole_diameter, 0.0,
              "The diameter (in x, y, and z) of the largest hole to fill.");
DEFINE_int32(max_num_hole_edges, 0,
             "Close holes which have no more than this many edges.");

int main(int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  // Sanity checks
  if (FLAGS_input_mesh == "") {
    std::cout << "The input mesh file is not specified.\n";
    return 1;
  }
  if (FLAGS_output_mesh == "") {
    std::cout << "The output mesh file is not specified.\n";
    return 1;
  }
  if (FLAGS_max_hole_diameter <= 0) {
    std::cout << "The maximum diameter of a hole to fill "
      "should be set to a positive value.\n";
    return 1;
  }
  if (FLAGS_max_num_hole_edges <= 0) {
    std::cout << "The maximum number of edges of a hole to fill "
      "should be set to a positive value.\n";
    return 1;
  }

  std::cout << "Reading: " << FLAGS_input_mesh << std::endl;
  std::ifstream input(FLAGS_input_mesh.c_str());
  Mesh mesh;
  std::string comments;
  CGAL::read_ply(input, mesh, comments);

  std::cout << "Max hole diameter: " << FLAGS_max_hole_diameter << std::endl;
  std::cout << "Max num hole edges: " << FLAGS_max_num_hole_edges << std::endl;

  // Incrementally fill the holes
  unsigned int nb_holes = 0;

  // Avoid examining a hole we studied before using another half edge
  // to it.
  std::set<Point> examined_points;

  for (halfedge_descriptor h : halfedges(mesh)) {
    if (is_border(h, mesh)) {
      // Skip large holes that are hard to fill
      if (!is_small_hole(h, mesh, examined_points,
                     FLAGS_max_hole_diameter, FLAGS_max_num_hole_edges))
        continue;

      std::vector<face_descriptor>  patch_facets;
      std::vector<vertex_descriptor> patch_vertices;
      bool success = std::get<0>(CGAL::Polygon_mesh_processing::
                                 triangulate_refine_and_fair_hole
                                 (mesh, h,
                                  std::back_inserter(patch_facets),
                                  std::back_inserter(patch_vertices),
                                  CGAL::Polygon_mesh_processing::parameters
                                  ::vertex_point_map(get(CGAL::vertex_point, mesh)).
                                  geom_traits(Kernel())) );
      nb_holes++;

      // std::cout << "* Number of facets in constructed patch: "
      // << patch_facets.size() << std::endl;
      // std::cout << "  Number of vertices in constructed patch: "
      // << patch_vertices.size() << std::endl;
      // std::cout << "  Is fairing successful: " << success << std::endl;
    }
  }

  std::cout << "Number of holes that have been closed: " << nb_holes << "\n";

  std::cout << "Writing: " << FLAGS_output_mesh << std::endl;
  std::ofstream out(FLAGS_output_mesh.c_str());
  out.precision(17);
  CGAL::write_PLY(out, mesh);
  out.close();

  return 0;
}

