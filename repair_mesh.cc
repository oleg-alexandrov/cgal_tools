#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Polygon_mesh_processing/repair_polygon_soup.h>
#include <CGAL/Polygon_mesh_processing/orient_polygon_soup.h>
#include <CGAL/Polygon_mesh_processing/polygon_soup_to_polygon_mesh.h>
#include <CGAL/IO/PLY_reader.h>
#include <CGAL/IO/PLY_writer.h>

#include <algorithm>
#include <array>
#include <iostream>
#include <vector>
#include <fstream>

#include <gflags/gflags.h>

// A little example to repair a polygon mesh

DEFINE_string(input_mesh, "",
              "The input mesh file.");
DEFINE_string(output_mesh, "",
              "The output mesh file.");

typedef CGAL::Exact_predicates_inexact_constructions_kernel     K;
typedef CGAL::Simple_cartesian<double>                          Kernel;
typedef K::FT                                                   FT;
typedef K::Point_3                                              Point_3;
typedef CGAL::Surface_mesh<Point_3>                             Mesh;
typedef std::array<FT, 3>                                       Custom_point;
typedef std::vector<std::size_t>                                CGAL_Polygon;
namespace PMP = CGAL::Polygon_mesh_processing;
struct Array_traits {
  struct Equal_3
  {
    bool operator()(const Custom_point& p, const Custom_point& q) const {
      return (p == q);
    }
  };
  struct Less_xyz_3
  {
    bool operator()(const Custom_point& p, const Custom_point& q) const {
      return std::lexicographical_compare(p.begin(), p.end(), q.begin(), q.end());
    }
  };
  Equal_3 equal_3_object() const { return Equal_3(); }
  Less_xyz_3 less_xyz_3_object() const { return Less_xyz_3(); }
};

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

  std::cout << "Reading " << FLAGS_input_mesh << std::endl;
  std::ifstream in(FLAGS_input_mesh);

  std::vector<Kernel::Point_3> points;
  std::vector< std::vector<std::size_t> > polygons;

  std::vector<CGAL::Color> fcolors;
  std::vector<CGAL::Color> vcolors;
  if (!in || !CGAL::read_PLY(in, points, polygons, fcolors, vcolors) || points.empty()) {
    std::cerr << "Cannot open file: " << FLAGS_input_mesh << std::endl;
    return EXIT_FAILURE;
  }

  std::vector<CGAL_Polygon> polygons2;
  for (size_t it = 0; it < polygons.size(); it++) {
    CGAL_Polygon p;
    for (size_t it2 = 0; it2 < polygons[it].size(); it2++) {
      p.push_back(polygons[it][it2]);
    }
    polygons2.push_back(p);
  }

  std::vector<std::array<FT, 3> > points2;
  for (size_t it = 0; it < points.size(); it++) {
    Kernel::Point_3 point = points[it];
    points2.push_back(CGAL::make_array<FT>(point[0], point[1], point[2]));
  }
  

  PMP::repair_polygon_soup(points2, polygons2, CGAL::parameters::geom_traits(Array_traits()));
  
  std::cout << "After reparation, the soup has " << points2.size() << " vertices and " << polygons2.size() << " faces" << std::endl;
  
  Mesh mesh;
  PMP::orient_polygon_soup(points2, polygons2);
  PMP::polygon_soup_to_polygon_mesh(points2, polygons2, mesh);
  std::cout << "Mesh has " << num_vertices(mesh) << " vertices and " << num_faces(mesh) << " faces" << std::endl;

  std::cout << "Writing: " << FLAGS_output_mesh << std::endl;
  std::ofstream out(FLAGS_output_mesh);
  out.precision(17);
  CGAL::write_PLY(out, mesh);
  
  return 0;
}

