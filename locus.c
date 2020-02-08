#ifndef locus_c_INCLUDED
#define locus_c_INCLUDED

/*
 * The Locus data type, which is a 3d collection of points.
 */

#include "v3.c"
#include "matrix.c"
#include "polyhedron.c"

#include "dyn.c"
DYN_INIT(Locus, v3)

void Locus_transform(Locus *locus, const _Mat transformation) {
  for (int i = 0; i < locus->length; i++) {
    const v3 point = Locus_get(locus, i);
    const v3 transformed = v3_transform(point, transformation);
    Locus_set(locus, i, transformed);
  }
}

void Locus_destroy(Locus *locus) {
  Dyn_destroy(locus);
}

Locus *Locus_clone(Locus *locus) {
  Locus *clone = Locus_new(locus->length);
  for (int i = 0; i < locus->length; i++) {
    Locus_append(clone, Locus_get(locus, i));
  }
  return clone;
}

Locus *Locus_from_parametric(
  v3 (*f)(float t, float s),

  const float t0,
  const float tf,
  const int t_count,
  const int do_t_wrapping,

  const float s0,
  const float sf,
  const int s_count,
  const int do_s_wrapping
) {

  Locus *locus = Locus_new(t_count * s_count);

  const float dt = (tf - t0) / t_count;
  const float ds = (sf - s0) / s_count;

  for (int t_idx = 0; t_idx < t_count; t_idx++) {
    for (int s_idx = 0; s_idx < s_count; s_idx++) {
      const float t = t0 + t_idx * dt;
      const float s = s0 + s_idx * ds;
      const v3 point = f(t, s);
      Locus_append(locus, point);
    }
  }

  return locus;

}

Locus *Locus_from_points(const int point_count, ...) {
  va_list args;
  va_start(args, point_count);

  Locus *locus = Locus_new(point_count);

  for (int i = 0; i < point_count; i++) {
    const v3 point = va_arg(args, v3);
    Locus_append(locus, point);
  }

  va_end(args);
  return locus;
}

void Locus_bounds_M(
      float *result_min_x, float *result_max_x,
      float *result_min_y, float *result_max_y,
      float *result_min_z, float *result_max_z,
      const Locus *locus
    ) {

  if (locus->length == 0) {
    printf("cannot find bounds of empty locus\n");
    exit(1);
  }

  if (locus->length == 1) {
    const v3 point = Locus_get(locus, 0);
    *result_min_x = point[0];
    *result_max_x = point[0];
    *result_min_y = point[1];
    *result_max_y = point[1];
    *result_min_z = point[2];
    *result_max_z = point[2];
    return;
  }

  *result_min_x = +DBL_MAX;
  *result_max_x = -DBL_MAX;
  *result_min_y = +DBL_MAX;
  *result_max_y = -DBL_MAX;
  *result_min_z = +DBL_MAX;
  *result_max_z = -DBL_MAX;

  for (int i = 0; i < locus->length; i++) {
    const v3 point = Locus_get(locus, i);

    const float x = point[0];
    const float y = point[1];
    const float z = point[2];

         if (x < *result_min_x) *result_min_x = x;
    else if (x > *result_max_x) *result_max_x = x;
         if (y < *result_min_y) *result_min_y = y;
    else if (y > *result_max_y) *result_max_y = y;
         if (z < *result_min_z) *result_min_z = z;
    else if (z > *result_max_z) *result_max_z = z;
  }
}

#endif // locus_c_INCLUDED

