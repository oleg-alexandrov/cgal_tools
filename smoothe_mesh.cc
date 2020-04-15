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

#define CGAL_PMP_SMOOTHING_VERBOSE

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_mesh_processing/bbox.h>
#include <CGAL/Polygon_mesh_processing/border.h>
#include <CGAL/Polygon_mesh_processing/orient_polygon_soup.h>
#include <CGAL/Polygon_mesh_processing/polygon_soup_to_polygon_mesh.h>
#include <CGAL/Polygon_mesh_processing/repair_polygon_soup.h>
#include <CGAL/Polygon_mesh_processing/smooth_shape.h>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/IO/PLY_reader.h>
#include <CGAL/IO/PLY_writer.h>

#include <gflags/gflags.h>

#include <fstream>
#include <iostream>
#include <vector>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_3 Point_3;
typedef CGAL::Surface_mesh<K::Point_3> Mesh;
typedef std::vector<std::size_t> Polygon;

namespace PMP = CGAL::Polygon_mesh_processing;

DEFINE_string(input_mesh, "",
              "The input mesh file.");
DEFINE_string(output_mesh, "",
              "The output mesh file.");
DEFINE_int32(num_iterations, 1,
             "How many iterations to do when smoothing the mesh.");
DEFINE_double(smoothing_time, 0.0001,
              "A larger smooth time will result in a smoother mesh.");

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
  if (FLAGS_num_iterations <= 0) {
    std::cout << "The number of smoothing iterations must be positive.\n";
    return 1;
  }
  if (FLAGS_smoothing_time <= 0) {
    std::cout << "The smoothing time must be positive.\n";
    return 1;
  }

  std::cout << "Reading " << FLAGS_input_mesh << std::endl;
  std::ifstream in(FLAGS_input_mesh);

  std::cout << "Number of iterations: " << FLAGS_num_iterations << "\n";
  std::cout << "Smoothing time:       " << FLAGS_smoothing_time << "\n";

  std::vector<Kernel::Point_3> points;
  std::vector< std::vector<std::size_t> > polygons;
  std::vector<CGAL::Color> fcolors;
  std::vector<CGAL::Color> vcolors;
  std::cout << "Reading: " << FLAGS_input_mesh << std::endl;
  if (!in || !CGAL::read_PLY(in, points, polygons, fcolors, vcolors) || points.empty()) {
    std::cerr << "Cannot open file: " << FLAGS_input_mesh << std::endl;
    return EXIT_FAILURE;
  }

  // Clean up the mesh before smoothing it
  Mesh mesh;
  PMP::repair_polygon_soup(points, polygons);
  PMP::orient_polygon_soup(points, polygons);
  PMP::polygon_soup_to_polygon_mesh(points, polygons, mesh);

  // Constrain boundary
  std::set<Mesh::Vertex_index> constrained_vertices;
  for (Mesh::Vertex_index v : vertices(mesh)) {
    if (is_border(v, mesh))
      constrained_vertices.insert(v);
  }

  std::cout << "Input mesh bounding box: "
            << CGAL::Polygon_mesh_processing::bbox(mesh) << "\n";

  std::cout << "Constraining: " << constrained_vertices.size()
            << " border vertices" << std::endl;
  CGAL::Boolean_property_map<std::set<Mesh::Vertex_index> > vcmap(constrained_vertices);
  PMP::smooth_shape(mesh, FLAGS_smoothing_time,
                    PMP::parameters::number_of_iterations(FLAGS_num_iterations)
                    .vertex_is_constrained_map(vcmap));

  std::cout << "Output mesh bounding box: "
            << CGAL::Polygon_mesh_processing::bbox(mesh) << "\n";

  std::cout << "Writing: " << FLAGS_output_mesh << std::endl;
  std::ofstream out(FLAGS_output_mesh);
  out.precision(17);
  CGAL::write_PLY(out, mesh);

  return EXIT_SUCCESS;
}
