#ifndef lattice_c_INCLUDED
#define lattice_c_INCLUDED

// The Lattice datatype, which is, essentially,
// the image of a 2d square lattice under some
// transformation R2 -> R3

#include "../matrix.c"
#include "v3.c"
#include "polyhedron.c"

typedef struct {
  v3 color;
  v3 position;
} ColoredPoint;

#include "../util/dyn.c"
DYN_INIT(LatticePoints, ColoredPoint)

typedef struct {
  LatticePoints *points;
  int width;
  int height;
} Lattice;

Lattice *Lattice_new(const int width, const int height) {
  Lattice *lattice = malloc(sizeof(Lattice));
  lattice->points = LatticePoints_new(width * height);
  lattice->width = width;
  lattice->height = height;
  return lattice;
}

ColoredPoint Lattice_get(const Lattice *lattice, const int x, const int y) {
  const int idx = lattice->width * y + x;
  return LatticePoints_get(lattice->points, idx);
}

void Lattice_set(const Lattice *lattice, const int x, const int y, const ColoredPoint clp) {
  const int idx = lattice->width * y + x;
  return LatticePoints_set(lattice->points, idx, clp);
}

void Lattice_transform(Lattice *lattice, const _Mat transformation) {
  for (int i = 0; i < lattice->points->length; i++) {
    ColoredPoint clp = LatticePoints_get(lattice->points, i);
    clp.position = v3_transform(clp.position, transformation);
    LatticePoints_set(lattice->points, i, clp);
  }
}

void Lattice_destroy(Lattice *lattice) {
  Dyn_destroy(lattice->points);
  free(lattice);
}

Lattice *Lattice_from_parametric(
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

  Lattice *lattice = Lattice_new(t_count, s_count);
  lattice->points->length = t_count * s_count;

  const float dt = (tf - t0) / t_count;
  const float ds = (sf - s0) / s_count;

  for (int t_idx = 0; t_idx < t_count; t_idx++) {
    for (int s_idx = 0; s_idx < s_count; s_idx++) {
      const float t = t0 + t_idx * dt;
      const float s = s0 + s_idx * ds;

      const v3 point = f(t, s);
      const v3 color = color_f(t, s);

      ColoredPoint clp = { .position = point, .color = color };
      Lattice_set(lattice, t_idx, s_idx, clp);
    }
  }

  return lattice;

}

void Lattice_bounds_M(v3 *lows, v3 *highs, const Lattice *lattice) {

#ifdef DEBUG
  if (lattice->points->length == 0) {
    printf("cannot find bounds of empty lattice\n");
    exit(1);
  }
#endif

  if (lattice->points->length == 1) {
    const v3 point = LatticePoints_get(lattice->points, 0).position;
    *lows = point;
    *highs = point;
    return;
  }

  *lows  = (v3) { +DBL_MAX, +DBL_MAX, +DBL_MAX };
  *highs = (v3) { -DBL_MAX, -DBL_MAX, -DBL_MAX };

  for (int i = 0; i < lattice->points->length; i++) {
    const v3 point = LatticePoints_get(lattice->points, i).position;

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

#endif // lattice_c_INCLUDED

