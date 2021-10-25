#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>

#include <CGAL/Polygon_mesh_processing/smooth_shape.h>
#include <CGAL/Polygon_mesh_processing/IO/polygon_mesh_io.h>

#include <iostream>
#include <fstream>

typedef CGAL::Exact_predicates_inexact_constructions_kernel   K;
typedef CGAL::Surface_mesh<K::Point_3>                        Mesh;

namespace PMP = CGAL::Polygon_mesh_processing;

int main(int argc, char* argv[]) {

  if (argc < 6) {
    std::cout << "Usage: " << argv[0]
              << " num_iterations smoothing_time smoothe_boundary input.ply output.ply\n";
    return 1;
  }
  
  int num_iterations      = atoi(argv[1]);
  double smoothing_time   = atof(argv[2]);
  int smoothe_boundary    = atoi(argv[3]);
  const char* input_file  = argv[4];
  const char* output_file = argv[5];

  std::cout << "Reading mesh:         " << input_file << std::endl;
  std::cout << "Number of iterations: " << num_iterations << "\n";
  std::cout << "Smoothing time:       " << smoothing_time << "\n";
  std::cout << "Smoothe boundary:     " << smoothe_boundary << "\n";

  Mesh mesh;
  if(!PMP::IO::read_polygon_mesh(input_file, mesh))
  {
    std::cerr << "Invalid input." << std::endl;
    return 1;
  }

  std::set<Mesh::Vertex_index> constrained_vertices;
  if (!smoothe_boundary) {
    for(Mesh::Vertex_index v : vertices(mesh)) {
        if(is_border(v, mesh))
          constrained_vertices.insert(v);
      }
  }
  
  std::cout << "Constraining: " << constrained_vertices.size() << " border vertices." << std::endl;
  
  CGAL::Boolean_property_map<std::set<Mesh::Vertex_index> > vcmap(constrained_vertices);
  
  //std::cout << "Smoothing shape... (" << num_iterations << " iterations)" << std::endl;
  PMP::smooth_shape(mesh, smoothing_time, PMP::parameters::number_of_iterations(num_iterations)
                                                .vertex_is_constrained_map(vcmap));

  std::cout << "Writing output mesh: " << output_file << std::endl;
  CGAL::IO::write_PLY(output_file, mesh,
                      CGAL::parameters::stream_precision(17).use_binary_mode(false));

  return 0;
}

