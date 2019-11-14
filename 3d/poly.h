#ifndef poly_h_INCLUDED
#define poly_h_INCLUDED

// Polygon data structure

#include <float.h>

#include "globals.h"
#include "pointvec.h"

typedef struct {
  Point *points[ENOUGH];
  int point_count;
} Poly;

Poly *Poly_new() {
  Poly *poly = malloc(sizeof(Poly));
  poly->point_count = 0;
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

void Poly_normal_M(Vec *result, const Poly *poly) {
  // Takes a polygon which MUST have at least 3 points
  // Returns the (length-1) normal veector to the polygon
  if (poly->point_count < 3) {
    printf("ERROR: In plane_normal(): requires point_count >= 3");
    exit(1);
  }

  Vec A;
  PointVec_between_M(&A, poly->points[0], poly->points[1]);
  PointVec_normalize_M(&A, &A);

  Vec B;
  PointVec_between_M(&B, poly->points[0], poly->points[2]);
  PointVec_normalize_M(&B, &B);

  PointVec_cross_M(result, &A, &B);
}

#endif // poly_h_INCLUDED
