#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>

#include <CGAL/Polygon_mesh_processing/triangulate_hole.h>
#include <CGAL/Polygon_mesh_processing/border.h>
#include <CGAL/Polygon_mesh_processing/IO/polygon_mesh_io.h>

#include <CGAL/IO/PLY.h>

#include <boost/lexical_cast.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <set>

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef Kernel::Point_3                                     Point;
typedef CGAL::Surface_mesh<Point>                           Mesh;

typedef boost::graph_traits<Mesh>::vertex_descriptor        vertex_descriptor;
typedef boost::graph_traits<Mesh>::halfedge_descriptor      halfedge_descriptor;
typedef boost::graph_traits<Mesh>::face_descriptor          face_descriptor;

namespace PMP = CGAL::Polygon_mesh_processing;

bool is_small_hole(halfedge_descriptor h, Mesh & mesh,
                   double max_hole_diam, int max_num_hole_edges)
{
  int num_hole_edges = 0;
  CGAL::Bbox_3 hole_bbox;
  for (halfedge_descriptor hc : CGAL::halfedges_around_face(h, mesh))
  {
    const Point& p = mesh.point(target(hc, mesh));

    hole_bbox += p.bbox();
    ++num_hole_edges;

    // Exit early, to avoid unnecessary traversal of large holes
    if (num_hole_edges > max_num_hole_edges) return false;
    if (hole_bbox.xmax() - hole_bbox.xmin() > max_hole_diam) return false;
    if (hole_bbox.ymax() - hole_bbox.ymin() > max_hole_diam) return false;
    if (hole_bbox.zmax() - hole_bbox.zmin() > max_hole_diam) return false;
  }

  return true;
}

// Incrementally fill the holes that are no larger than given diameter
// and with no more than a given number of edges (if specified).

int main(int argc, char* argv[]) {

  if (argc < 5) {
    std::cout << "Usage: " << argv[0]
              << " max_hole_diameter max_num_hole_edges input.ply output.ply\n";
    return 1;
  }
  
  double max_hole_diam    = atof(argv[1]);
  int max_num_hole_edges  = atoi(argv[2]);
  const char* input_file  = argv[3];
  const char* output_file = argv[4];

  std::cout << "Reading mesh:       " << input_file << std::endl;
  std::cout << "Max num hole edges: " << max_num_hole_edges << "\n";
  std::cout << "Max hole diameter:  " << max_hole_diam << "\n";
  
  Mesh mesh;
  if(!PMP::IO::read_polygon_mesh(input_file, mesh)) {
    std::cerr << "Invalid input." << std::endl;
    return 1;
  }

  unsigned int nb_holes = 0;
  std::vector<halfedge_descriptor> border_cycles;

  // collect one halfedge per boundary cycle
  PMP::extract_boundary_cycles(mesh, std::back_inserter(border_cycles));

  for(halfedge_descriptor h : border_cycles)
  {
    if(max_hole_diam > 0 && max_num_hole_edges > 0 &&
       !is_small_hole(h, mesh, max_hole_diam, max_num_hole_edges))
      continue;

    std::vector<face_descriptor>  patch_facets;
    std::vector<vertex_descriptor> patch_vertices;
    bool success =
      std::get<0>(PMP::triangulate_refine_and_fair_hole(mesh,
                                                        h,
                                                        std::back_inserter(patch_facets),
                                                        std::back_inserter(patch_vertices)));
    
    //std::cout << "* Number of facets in constructed patch: " << patch_facets.size() << std::endl;
    //std::cout << "  Number of vertices in constructed patch: " << patch_vertices.size() << std::endl;
    //std::cout << "  Is fairing successful: " << success << std::endl;
    ++nb_holes;
  }

  //std::cout << std::endl;
  //std::cout << nb_holes << " holes have been filled" << std::endl;

  std::cout << "Writing output mesh: " << output_file << std::endl;
  CGAL::IO::write_PLY(output_file, mesh,
                      CGAL::parameters::stream_precision(17).use_binary_mode(false));

  return 0;
}
