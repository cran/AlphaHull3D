#include "alphahull3d.h"

// [[Rcpp::export]]
Rcpp::NumericMatrix FAS_cpp(Rcpp::NumericMatrix pts, double alpha, bool volume) {
  // make list of points
  const int npoints = pts.ncol();
  std::list<Point3> points;
  for(int i = 0; i < npoints; i++) {
    const Rcpp::NumericVector pt_i = pts(Rcpp::_, i);
    points.push_back(Point3(pt_i(0), pt_i(1), pt_i(2)));
  }
  // compute alpha shape
  Fixed_alpha_shape_3 as(points.begin(), points.end(), alpha);
  // get the facets
  std::list<Facet> facets;
  as.get_alpha_shape_facets(std::back_inserter(facets),
                            Fixed_alpha_shape_3::REGULAR);
  as.get_alpha_shape_facets(std::back_inserter(facets),
                            Fixed_alpha_shape_3::SINGULAR);
  const int nfacets = facets.size();
  // output - facet indices are 3i, 3i+1, 3i+2
  Rcpp::NumericMatrix Vertices(3, 3 * nfacets);
  std::list<Facet>::iterator it_facet;
  int i = 0;
  for(it_facet = facets.begin(); it_facet != facets.end(); it_facet++) {
    Facet facet = *it_facet;
    // to have a consistent orientation, always consider an exterior cell
    if(as.classify(facet.first) != Fixed_alpha_shape_3::EXTERIOR) {
      facet = as.mirror_facet(facet);
    }
    int indices[3] = {
      (facet.second + 1) % 4,
      (facet.second + 2) % 4,
      (facet.second + 3) % 4,
    };
    // needed to get a consistent orientation
    if(facet.second % 2 == 0 ){
      std::swap(indices[0], indices[1]);
    }
    const Point3 v1 = facet.first->vertex(indices[0])->point();
    const Point3 v2 = facet.first->vertex(indices[1])->point();
    const Point3 v3 = facet.first->vertex(indices[2])->point();
    const Rcpp::NumericVector V1 = {v1.x(), v1.y(), v1.z()};
    const Rcpp::NumericVector V2 = {v2.x(), v2.y(), v2.z()};
    const Rcpp::NumericVector V3 = {v3.x(), v3.y(), v3.z()};
    Vertices(Rcpp::_, 3*i)   = V1;
    Vertices(Rcpp::_, 3*i+1) = V2;
    Vertices(Rcpp::_, 3*i+2) = V3;
    i++;
  }
  // volume of the tetrahedra
  if(volume) {
    double vol = 0.0;
    std::list<Cell_handle> cells;
    as.get_alpha_shape_cells(std::back_inserter(cells),
                             Fixed_alpha_shape_3::EXTERIOR);
    as.get_alpha_shape_cells(std::back_inserter(cells),
                             Fixed_alpha_shape_3::INTERIOR);
    std::list<Cell_handle>::iterator it_cell;
    for(it_cell = cells.begin(); it_cell != cells.end(); it_cell++) {
      Cell_handle cell = *it_cell;
      Tetrahedron t(
          cell->vertex(0)->point(),
          cell->vertex(1)->point(),
          cell->vertex(2)->point(),
          cell->vertex(3)->point()
      );
      vol += fabs(t.volume());
    }
    Vertices.attr("volume") = vol;
  }
  return Vertices;
}