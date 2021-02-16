#include <CGAL/Simple_cartesian.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Surface_mesh_simplification/edge_collapse.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Count_ratio_stop_predicate.h>
#include <chrono>
#include <fstream>
#include <iostream>

#include <gflags/gflags.h>

#include <CGAL/IO/PLY_reader.h>
#include <CGAL/IO/PLY_writer.h>

DEFINE_string(input_mesh, "",
              "The input mesh file.");
DEFINE_string(output_mesh, "",
              "The output mesh file.");
DEFINE_double(edge_keep_ratio, 0.2,
              "Stop when the number of edges is this this fraction of the original.");

typedef CGAL::Simple_cartesian<double>               Kernel;
typedef Kernel::Point_3                              Point_3;
typedef CGAL::Surface_mesh<Point_3>                  Surface_mesh;

namespace SMS = CGAL::Surface_mesh_simplification;

int main(int argc, char** argv) {

  gflags::ParseCommandLineFlags(&argc, &argv, true);

  Surface_mesh surface_mesh;
  std::string comments;
  std::ifstream is(FLAGS_input_mesh.c_str());
  std::cout << "Reading: " << FLAGS_input_mesh << std::endl;
  if (!is || !read_ply(is, surface_mesh, comments)) {
    std::cerr << "Failed to read input mesh: " << FLAGS_input_mesh << std::endl;
    return EXIT_FAILURE;
  }

  if (!CGAL::is_triangle_mesh(surface_mesh)){
    std::cerr << "Input geometry is not triangulated." << std::endl;
    return EXIT_FAILURE;
  }
  
  std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
  // In this example, the simplification stops when the number of undirected edges
  // drops below 10% of the initial count
  SMS::Count_ratio_stop_predicate<Surface_mesh> stop(FLAGS_edge_keep_ratio);
  int r = SMS::edge_collapse(surface_mesh, stop);
  std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::now();

  std::cout << "\nFinished!\n" << r << " edges removed.\n"
            << surface_mesh.number_of_edges() << " final edges.\n";
  std::cout << "Time elapsed: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count()
            << "ms" << std::endl;

  // Without this the mesh will be invalid
  surface_mesh.collect_garbage();
  
  std::ofstream os(FLAGS_output_mesh.c_str());
  std::cout << "Writing: " << FLAGS_output_mesh << std::endl;
  os.precision(17);
  if (!write_ply(os, surface_mesh)) {
    std::cerr << "Could not write mesh to " << FLAGS_output_mesh << std::endl;
    return EXIT_FAILURE;
  }
  
  return EXIT_SUCCESS;
}

  
