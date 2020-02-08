#ifndef polygon_h_INCLUDED
#define polygon_h_INCLUDED

// Polygongon data structure

#include <float.h>

#include "globals.c"
#include "v3.c"

#include "dyn.c"
DYN_INIT(Polygon, v3);

void Polygon_print(const Polygon* polygon) {
  printf("POLY [\n");
  for (int point_idx = 0; point_idx < polygon->length; point_idx++) {
    printf("\t");
    v3_print(Polygon_get(polygon, point_idx));
    printf("\n");
  }
  printf("] POLY\n");
}

Polygon *Polygon_clone(const Polygon *source) {
  Polygon *polygon = Polygon_new(source->length);

  for (int point_idx = 0; point_idx < source->length; point_idx++) {
    Polygon_append(polygon, Polygon_get(source, point_idx));
  }

  return polygon;
}

void Polygon_destroy(Polygon *polygon) {
  Dyn_destroy(polygon);
}

v3 Polygon_center(const Polygon *polygon) {
  float min_x = DBL_MAX;
  float max_x = DBL_MIN;
  float min_y = DBL_MAX;
  float max_y = DBL_MIN;
  float min_z = DBL_MAX;
  float max_z = DBL_MIN;

  for (int point_idx = 0; point_idx < polygon->length; point_idx++) {
    const v3 point = Polygon_get(polygon, point_idx);
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

void Polygon_xs_M(float *xs, const Polygon *polygon) {
  for (int i = 0; i < polygon->length; i++) xs[i] = Polygon_get(polygon, i)[0];
}

void Polygon_ys_M(float *ys, const Polygon *polygon) {
  for (int i = 0; i < polygon->length; i++) ys[i] = Polygon_get(polygon, i)[1];
}

void Polygon_zs_M(float *zs, const Polygon *polygon) {
  for (int i = 0; i < polygon->length; i++) zs[i] = Polygon_get(polygon, i)[2];
}

void Polygon_transform(Polygon *polygon, const float transformation[4][4]) {
  // This just does matrix multiplication
  for (int i = 0; i < polygon->length; i++) {
    Polygon_set(polygon, i, v3_transform(Polygon_get(polygon, i), transformation));
  }
}

void Polygon_scale_relative(Polygon *polygon, const float ratio) {
  const v3 polygon_center = Polygon_center(polygon);

  _Mat scale = Mat_dilate(ratio, ratio, ratio);
  _Mat to_origin = Mat_translate_v(-polygon_center);
  _Mat from_origin = Mat_translate_v(+polygon_center);

  _Mat composed;
  Mat_chain_M(
    composed, 3,
    to_origin,
    scale,
    from_origin
  );

  Polygon_transform(polygon, composed);
}

Polygon *Polygon_from_parametric(
  float (*x)(float t),
  float (*y)(float t),
  float (*z)(float t),
  float t0,
  float tf,
  float step
) {

  const size_t count = (tf - t0) / step;
  Polygon *polygon = Polygon_new(count);

  for (float t = t0; t <= tf; t += step) {
    v3 point = { x(t), y(t), z(t) };
    Polygon_append(polygon, point);
  }

  return polygon;

}

Polygon *Polygon_from_points(const int point_count, ...) {
  Polygon *polygon = Polygon_new(point_count);

  va_list args;
  va_start(args, point_count);

  for (int i = 0; i < point_count; i++) {
    v3 point = va_arg(args, v3);
    Polygon_append(polygon, point);
  }

  va_end(args);

  return polygon;
}

#endif // polygon_h_INCLUDED
