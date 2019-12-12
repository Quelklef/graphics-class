#ifndef poly_h_INCLUDED
#define poly_h_INCLUDED

// Polygon data structure

#include <float.h>

#include "globals.h"
#include "pointvec.h"

typedef struct Poly {
  Point *points[ENOUGH];
  int point_count;
} Poly;

void Poly_init(Poly *poly) {
  poly->point_count = 0;
}

Poly *Poly_new() {
  Poly *poly = malloc(sizeof(Poly));
  Poly_init(poly);
  return poly;
}

void Poly_print(const Poly* poly) {
  printf("POLY [\n");
  for (int point_idx = 0; point_idx < poly->point_count; point_idx++) {
    PointVec_print(poly->points[point_idx]);
  }
  printf("] POLY\n");
}

void Poly_add_point(Poly *poly, Point *point) {
  /* Assumes that the added point is coplanar; does not verify */
  poly->points[poly->point_count] = point;
  poly->point_count++;
}

Poly *Poly_clone(const Poly *source) {
  Poly *poly = Poly_new();

  for (int point_idx = 0; point_idx < source->point_count; point_idx++) {
    Poly_add_point(poly, PointVec_clone(source->points[point_idx]));
  }

  return poly;
}

void Poly_clear(Poly *poly) {
  for (int point_idx = 0; point_idx < poly->point_count; point_idx++) {
    PointVec_destroy(poly->points[point_idx]);
  }
  poly->point_count = 0;
}

void Poly_destroy(Poly *poly) {
  Poly_clear(poly);
  free(poly);
}

void Poly_calc_center_M(Point *result, const Poly *poly) {
  double min_x = DBL_MAX;
  double max_x = DBL_MIN;
  double min_y = DBL_MAX;
  double max_y = DBL_MIN;
  double min_z = DBL_MAX;
  double max_z = DBL_MIN;

  for (int point_idx = 0; point_idx < poly->point_count; point_idx++) {
    const Point *point = poly->points[point_idx];
    const double x = point->x;
    const double y = point->y;
    const double z = point->z;

    if (x < min_x) min_x = x;
    if (x > max_x) max_x = x;
    if (y < min_y) min_y = y;
    if (y > max_y) max_y = y;
    if (z < min_z) min_z = z;
    if (z > max_z) max_z = z;
  }

  result->x = min_x / 2 + max_x / 2;
  result->y = min_y / 2 + max_y / 2;
  result->z = min_z / 2 + max_z / 2;
}

void Poly_xs_M(double *xs, const Poly *poly) {
  for (int i = 0; i < poly->point_count; i++) xs[i] = poly->points[i]->x;
}

void Poly_ys_M(double *xs, const Poly *poly) {
  for (int i = 0; i < poly->point_count; i++) xs[i] = poly->points[i]->y;
}

void Poly_zs_M(double *xs, const Poly *poly) {
  for (int i = 0; i < poly->point_count; i++) xs[i] = poly->points[i]->z;
}

void Poly_transform(Poly *poly, const double transformation[4][4]) {
  // This just does matrix multiplication

  double xs[poly->point_count];
  Poly_xs_M(xs, poly);

  double ys[poly->point_count];
  Poly_ys_M(ys, poly);

  double zs[poly->point_count];
  Poly_zs_M(zs, poly);

  for (int i = 0; i < poly->point_count; i++) {
    poly->points[i]->x = transformation[0][0] * xs[i]
                       + transformation[0][1] * ys[i]
                       + transformation[0][2] * zs[i]
                       + transformation[0][3] * 1;

    poly->points[i]->y = transformation[1][0] * xs[i]
                       + transformation[1][1] * ys[i]
                       + transformation[1][2] * zs[i]
                       + transformation[1][3] * 1;

    poly->points[i]->z = transformation[2][0] * xs[i]
                       + transformation[2][1] * ys[i]
                       + transformation[2][2] * zs[i]
                       + transformation[2][3] * 1;
  }
}

void Poly_scale_relative(Poly *poly, const double ratio) {
  Point poly_center;
  Poly_calc_center_M(&poly_center, poly);

  _Mat scale;
  Mat_scaling_M(scale, ratio, ratio, ratio);

  _Mat to_origin;
  Mat_translation_M(to_origin, -poly_center.x, -poly_center.y, -poly_center.z);
  Mat_mult_right(scale, to_origin);

  _Mat from_origin;
  Mat_translation_M(from_origin, +poly_center.x, +poly_center.y, +poly_center.z);
  Mat_mult_left(scale, from_origin);

  Poly_transform(poly, scale);
}

#endif // poly_h_INCLUDED
