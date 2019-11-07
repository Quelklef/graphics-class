#ifndef poly_h_INCLUDED
#define poly_h_INCLUDED

// Polygon data structure

#include <float.h>

#include "globals.h"

typedef struct {
  double xs[ENOUGH];
  double ys[ENOUGH];
  double zs[ENOUGH];
  int point_count;
} Poly;

Poly *Poly_new() {
  Poly *poly = malloc(sizeof(Poly));
  poly->point_count = 0;
  return poly;
}

void Poly_add_point(Poly *poly, const double x, const double y, const double z) {
  /* Assumes that the added point is coplanar; does not verify */
  poly->xs[poly->point_count] = x;
  poly->ys[poly->point_count] = y;
  poly->zs[poly->point_count] = z;
  poly->point_count++;
}

void Poly_calc_center(const Poly *poly, double *center_x, double *center_y, double *center_z) {
  double min_x = DBL_MAX;
  double max_x = DBL_MIN;
  double min_y = DBL_MAX;
  double max_y = DBL_MIN;
  double min_z = DBL_MAX;
  double max_z = DBL_MIN;

  for (int point_idx = 0; point_idx < poly->point_count; point_idx++) {
    double x = poly->xs[point_idx];
    double y = poly->ys[point_idx];
    double z = poly->zs[point_idx];

    if (x < min_x) min_x = x;
    if (x > max_x) max_x = x;
    if (y < min_y) min_y = y;
    if (y > max_y) max_y = y;
    if (z < min_z) min_z = z;
    if (z > max_z) max_z = z;
  }

  *center_x = min_x / 2 + max_x / 2;
  *center_y = min_y / 2 + max_y / 2;
  *center_z = min_z / 2 + max_z / 2;
}

#endif // poly_h_INCLUDED
