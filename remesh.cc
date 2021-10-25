#include <CGAL/Simple_cartesian.h>
#include <CGAL/Advancing_front_surface_reconstruction.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/IO/PLY_reader.h>
#include <CGAL/IO/PLY_writer.h>

#include <algorithm>
#include <iostream>
#include <fstream>

// TODO(oalexan1): This code needs to be ported to CGAL 5.3 like the rest
// of the files here. Also remove the Gflags dependency.

namespace PMP = CGAL::Polygon_mesh_processing;

typedef CGAL::Simple_cartesian<double> K;
typedef K::Point_3  Point_3;
typedef std::array<std::size_t, 3> Facet;
typedef CGAL::Surface_mesh<K::Point_3> Mesh;

struct Perimeter {
  double bound;

  explicit Perimeter(double bound): bound(bound) {}

  template <typename AdvancingFront, typename Cell_handle>
  double operator() (const AdvancingFront& adv, Cell_handle& c,
                     const int& index) const {
    // bound == 0 is better than bound < infinity
    // as it avoids the distance computations
    if (bound == 0) {
      return adv.smallest_radius_delaunay_sphere (c, index);
    }

    // If perimeter > bound, return infinity so that facet is not used
    double d  = 0;
    d = sqrt(squared_distance(c->vertex((index+1)%4)->point(),
                              c->vertex((index+2)%4)->point()));
    if (d > bound) return adv.infinity();
    d += sqrt(squared_distance(c->vertex((index+2)%4)->point(),
                               c->vertex((index+3)%4)->point()));
    if (d > bound) return adv.infinity();
    d += sqrt(squared_distance(c->vertex((index+1)%4)->point(),
                               c->vertex((index+3)%4)->point()));
    if (d > bound) return adv.infinity();
    // Otherwise, return usual priority value: smallest radius of
    // delaunay sphere
    return adv.smallest_radius_delaunay_sphere (c, index);
  }
};

DEFINE_string(input_mesh, "",
              "The input mesh file.");
DEFINE_string(output_mesh, "",
              "The output mesh file.");
DEFINE_double(max_triangle_perimeter, 0.0,
              "All created triangular faces must have perimeter no more than this.");
DEFINE_double(radius_ratio_bound, 5.0,
             "A larger value of this allows for more (but less pretty) triangles.");

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
  if (FLAGS_max_triangle_perimeter <= 0) {
    std::cout << "The maximum triangle perimeter is not specified.\n";
    return 1;
  }
  if (FLAGS_radius_ratio_bound <= 0) {
    std::cout << "The maximum number of edges of a hole to fill "
      "should be set to a positive value.\n";
    return 1;
  }

  std::vector<Point_3> points;
  std::vector< std::vector<std::size_t> > polygons;
  std::vector<CGAL::Color> fcolors;
  std::vector<CGAL::Color> vcolors;

  std::cout << "Reading: " << FLAGS_input_mesh << std::endl;
  std::ifstream in(FLAGS_input_mesh.c_str());
  if (!in || !CGAL::read_PLY(in, points, polygons, fcolors, vcolors) || points.empty()) {
    std::cerr << "Cannot open file: " << FLAGS_input_mesh << std::endl;
    return EXIT_FAILURE;
  }
  std::cerr << "Read: " << points.size() << " points." << std::endl;

  Perimeter perimeter(FLAGS_max_triangle_perimeter);
  double beta = 0.52;

  std::vector<Facet> faces;
  CGAL::advancing_front_surface_reconstruction(points.begin(),
                                               points.end(),
                                               std::back_inserter(faces),
                                               perimeter,
                                               FLAGS_radius_ratio_bound,
                                               beta);

  std::cout << "Writing: " << FLAGS_output_mesh << std::endl;
  std::ofstream out(FLAGS_output_mesh.c_str());
  out.precision(17);
  write_PLY(out, points, faces);

  return 0;
}
