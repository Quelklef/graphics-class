#ifndef locus_h_INCLUDED
#define locus_h_INCLUDED

/*
 * The Locus data type, which is a 3d collection of points.
 */

#include "v3.h"
#include "matrix.h"
#include "poly.h"

#include "dyn.H"
DYN_INIT(Locus, v3)

void Locus_transform(Locus *locus, const _Mat transformation) {
  for (int i = 0; i < Locus_length(locus); i++) {
    const v3 point = Locus_get(locus, i);
    const v3 transformed = v3_transform(point, transformation);
    Locus_set(locus, i, transformed);
  }
}

void Locus_destroy(Locus *locus) {
  Dyn_destroy(locus);
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

#endif // locus_h_INCLUDED

