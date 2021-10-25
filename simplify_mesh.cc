#include <CGAL/Simple_cartesian.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Surface_mesh_simplification/edge_collapse.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Count_ratio_stop_predicate.h>
#include <CGAL/Polygon_mesh_processing/IO/polygon_mesh_io.h>
#include <CGAL/IO/PLY.h>

#include <chrono>
#include <fstream>
#include <iostream>

typedef CGAL::Simple_cartesian<double>               Kernel;
typedef Kernel::Point_3                              Point_3;
typedef CGAL::Surface_mesh<Point_3>                  Mesh;

namespace SMS = CGAL::Surface_mesh_simplification;
namespace PMP = CGAL::Polygon_mesh_processing;

int main(int argc, char** argv) {

  if (argc < 4) {
    std::cout << "Usage: " << argv[0]
              << " edge_keep_ratio input.ply output.ply\n";
    return 1;
  }

  //Simply the mesh keeping only this fraction of the original edges.
  double edge_keep_ratio  = atof(argv[1]);
  
  const char* input_file  = argv[2];
  const char* output_file = argv[3];

  std::cout << "Edge keep ratio: " << edge_keep_ratio << "\n";
  std::cout << "Reading mesh:       " << input_file << "\n";

  Mesh mesh;
  if(!PMP::IO::read_polygon_mesh(input_file, mesh)) {
    std::cerr << "Invalid input." << std::endl;
    return 1;
  }
  
  if (!CGAL::is_triangle_mesh(mesh)){
    std::cerr << "Input geometry is not triangulated." << std::endl;
    return EXIT_FAILURE;
  }
  
  std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
  // In this example, the simplification stops when the number of undirected edges
  // drops below 10% of the initial count
  SMS::Count_ratio_stop_predicate<Mesh> stop(edge_keep_ratio);
  int r = SMS::edge_collapse(mesh, stop);
  std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::now();

  std::cout << "Edges removed: " << r << ".\n"
            << "Edges left: " << mesh.number_of_edges() << ".\n";
  //d::cout << "Time elapsed: "
  //        << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count()
  //        << "ms" << std::endl;

  // Without this the mesh will be invalid
  mesh.collect_garbage();

  std::cout << "Writing output mesh: " << output_file << std::endl;
  CGAL::IO::write_PLY(output_file, mesh,
                      CGAL::parameters::stream_precision(17).use_binary_mode(false));
  return EXIT_SUCCESS;
}
