#ifndef locus_c_INCLUDED
#define locus_c_INCLUDED

/*
 * The Locus data type, which is a 3d collection of points.
 */

#include "../matrix.c"
#include "v3.c"
#include "polyhedron.c"

typedef struct {
  v3 color;
  v3 point;
} ColoredPoint;

#include "../util/dyn.c"
DYN_INIT(Locus, ColoredPoint)

void Locus_transform(Locus *locus, const _Mat transformation) {
  for (int i = 0; i < locus->length; i++) {
    ColoredPoint clp = Locus_get(locus, i);
    clp.point = v3_transform(clp.point, transformation);
    Locus_set(locus, i, clp);
  }
}

void Locus_destroy(Locus *locus) {
  Dyn_destroy(locus);
}

Locus *Locus_from_parametric(
  // Paramatric definition of the shape
  v3 (*f)(float t, float s),
  // Parallel parametric function giving colors
  v3 (*color_f)(float t, float s),

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
      const v3 color = color_f(t, s);

      ColoredPoint clp = { .point = point, .color = color };
      Locus_append(locus, clp);
    }
  }

  return locus;

}

Locus *Locus_from_points(const int point_count, const v3 rgb, ...) {
  va_list args;
  va_start(args, rgb);

  Locus *locus = Locus_new(point_count);

  for (int i = 0; i < point_count; i++) {
    const v3 point = va_arg(args, v3);
    ColoredPoint clp = { .point = point, .color = rgb };
    Locus_append(locus, clp);
  }

  va_end(args);
  return locus;
}

void Locus_bounds_M(v3 *lows, v3 *highs, const Locus *locus) {

#ifdef DEBUG
  if (locus->length == 0) {
    printf("cannot find bounds of empty locus\n");
    exit(1);
  }
#endif

  if (locus->length == 1) {
    const v3 point = Locus_get(locus, 0).point;
    *lows = point;
    *highs = point;
    return;
  }

  *lows  = (v3) { +DBL_MAX, +DBL_MAX, +DBL_MAX };
  *highs = (v3) { -DBL_MAX, -DBL_MAX, -DBL_MAX };

  for (int i = 0; i < locus->length; i++) {
    const v3 point = Locus_get(locus, i).point;

    const float x = point[0];
    const float y = point[1];
    const float z = point[2];

         if (x < (*lows )[0]) (*lows )[0] = x;
    else if (x > (*highs)[0]) (*highs)[0] = x;
         if (y < (*lows )[1]) (*lows )[1] = y;
    else if (y > (*highs)[1]) (*highs)[1] = y;
         if (z < (*lows )[2]) (*lows )[2] = z;
    else if (z > (*highs)[2]) (*highs)[2] = z;
  }
}

#endif // locus_c_INCLUDED

