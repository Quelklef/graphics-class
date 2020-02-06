#ifndef poly_h_INCLUDED
#define poly_h_INCLUDED

// Polygon data structure

#include <float.h>

#include "globals.h"
#include "v3.h"

typedef struct Poly {
  v3 points[ENOUGH];
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
    printf("\t");
    v3_print(poly->points[point_idx]);
    printf("\n");
  }
  printf("] POLY\n");
}

void Poly_add_point(Poly *poly, v3 point) {
  /* Assumes that the added point is coplanar; does not verify */
  poly->points[poly->point_count] = point;
  poly->point_count++;
}

Poly *Poly_clone(const Poly *source) {
  Poly *poly = Poly_new();

  for (int point_idx = 0; point_idx < source->point_count; point_idx++) {
    Poly_add_point(poly, source->points[point_idx]);
  }

  return poly;
}

void Poly_clear(Poly *poly) {
  poly->point_count = 0;
}

void Poly_destroy(Poly *poly) {
  Poly_clear(poly);
  free(poly);
}

v3 Poly_center(const Poly *poly) {
  float min_x = DBL_MAX;
  float max_x = DBL_MIN;
  float min_y = DBL_MAX;
  float max_y = DBL_MIN;
  float min_z = DBL_MAX;
  float max_z = DBL_MIN;

  for (int point_idx = 0; point_idx < poly->point_count; point_idx++) {
    const v3 point = poly->points[point_idx];
    const float x = point[0];
    const float y = point[1];
    const float z = point[2];

    if (x < min_x) min_x = x;
    if (x > max_x) max_x = x;
    if (y < min_y) min_y = y;
    if (y > max_y) max_y = y;
    if (z < min_z) min_z = z;
    if (z > max_z) max_z = z;
  }

  return (v3) {
    min_x / 2 + max_x / 2,
    min_y / 2 + max_y / 2,
    min_z / 2 + max_z / 2
  };
}

void Poly_xs_M(float *xs, const Poly *poly) {
  for (int i = 0; i < poly->point_count; i++) xs[i] = poly->points[i][0];
}

void Poly_ys_M(float *ys, const Poly *poly) {
  for (int i = 0; i < poly->point_count; i++) ys[i] = poly->points[i][1];
}

void Poly_zs_M(float *zs, const Poly *poly) {
  for (int i = 0; i < poly->point_count; i++) zs[i] = poly->points[i][2];
}

void Poly_transform(Poly *poly, const float transformation[4][4]) {
  // This just does matrix multiplication
  for (int i = 0; i < poly->point_count; i++) {
    poly->points[i] = v3_transform(poly->points[i], transformation);
  }
}

void Poly_scale_relative(Poly *poly, const float ratio) {
  const v3 poly_center = Poly_center(poly);

  _Mat scale = Mat_dilate(ratio, ratio, ratio);
  _Mat to_origin = Mat_translate_v(-poly_center);
  _Mat from_origin = Mat_translate_v(+poly_center);

  _Mat composed;
  Mat_chain_M(
    composed, 3,
    to_origin,
    scale,
    from_origin
  );

  Poly_transform(poly, composed);
}

Poly *Poly_from_parametric(
  float (*x)(float t),
  float (*y)(float t),
  float (*z)(float t),
  float t0,
  float tf,
  float step
) {

  Poly *poly = Poly_new();

  for (float t = t0; t <= tf; t += step) {
    v3 point = { x(t), y(t), z(t) };
    Poly_add_point(poly, point);
  }

  return poly;

}

Poly *Poly_from_points(const int point_count, ...) {
  Poly *poly = Poly_new();

  va_list args;
  va_start(args, point_count);

  for (int i = 0; i < point_count; i++) {
    v3 point = va_arg(args, v3);
    Poly_add_point(poly, point);
  }

  va_end(args);

  return poly;
}

#endif // poly_h_INCLUDED
