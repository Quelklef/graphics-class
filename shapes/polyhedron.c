#ifndef polyhedron_c_INCLUDED
#define polyhedron_c_INCLUDED

// The Polyhedron data type, which is a collection of polygongons

#include <float.h>

#include "v3.c"
#include "polygon.c"
#include "../matrix.c"

#include "../util/dyn.c"
DYN_INIT(Polyhedron, Polygon*)

void Polyhedron_print(const Polyhedron *polyhedron) {
  printf("POLYHEDRON [\n");
  for (int polygon_idx = 0; polygon_idx < polyhedron->length; polygon_idx++) {
    Polygon_print(Polyhedron_get(polyhedron, polygon_idx));
  }
  printf("] POLYHEDRON\n");
}

void Polyhedron_transform(Polyhedron *polyhedron, const _Mat transformation) {
  for (int polygon_idx = 0; polygon_idx < polyhedron->length; polygon_idx++) {
    Polygon *polygon = Polyhedron_get(polyhedron, polygon_idx);
    Polygon_transform(polygon, transformation);
  }
}

void Polyhedron_destroy(Polyhedron *polyhedron) {
  for (int i = 0; i < polyhedron->length; i++) {
    Polygon_destroy(Polyhedron_get(polyhedron, i));
  }
  Dyn_destroy(polyhedron);
}

void Polyhedron_bounds_M(v3 *lows, v3 *highs, const Polyhedron *polyhedron) {
#ifdef DEBUG
  if (polyhedron->length == 0) {
    printf("cannot find bounds of empty polyhedron\n");
    exit(1);
  }
#endif

  *lows  = (v3) { +DBL_MAX, +DBL_MAX, +DBL_MAX };
  *highs = (v3) { -DBL_MAX, -DBL_MAX, -DBL_MAX };

  for (int polygon_idx = 0; polygon_idx < polyhedron->length; polygon_idx++) {
    const Polygon *polygon = Polyhedron_get(polyhedron, polygon_idx);
    for (int point_idx = 0; point_idx < polygon->length; point_idx++) {
      const v3 point = Polygon_get(polygon, point_idx);

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
}

Polyhedron *load_polyhedron(const char *filename) {

  FILE *file = fopen(filename, "r");

  if (file == NULL) {
    printf("Cannot open file %s", filename);
    exit(1);
  }

  int number_of_indexed_points;
  fscanf(file, "%d", &number_of_indexed_points);

  double xs[number_of_indexed_points];
  double ys[number_of_indexed_points];
  double zs[number_of_indexed_points];

  for (int point_idx = 0; point_idx < number_of_indexed_points; point_idx++) {
    fscanf(file, "%lf", &xs[point_idx]);
    fscanf(file, "%lf", &ys[point_idx]);
    fscanf(file, "%lf", &zs[point_idx]);
  }

  int polyhedron_polygon_count;
  fscanf(file, "%d", &polyhedron_polygon_count);

  Polyhedron *polyhedron = Polyhedron_new(polyhedron_polygon_count);

  for (int polygon_idx = 0; polygon_idx < polyhedron_polygon_count; polygon_idx++) {

    int polygon_point_count;
    fscanf(file, "%d", &polygon_point_count);

    Polygon *polygon = Polygon_new(polygon_point_count);

    for (int point_idx = 0; point_idx < polygon_point_count; point_idx++) {
      int crossref_idx;
      fscanf(file, "%d", &crossref_idx);

#ifdef DEBUG
      if (crossref_idx > number_of_indexed_points) {
        printf("Internal error in load_polyhedron.\n");
        exit(1);
      }
#endif

      v3 point = {
        (float) xs[crossref_idx],
        (float) ys[crossref_idx],
        (float) zs[crossref_idx]
      };
      Polygon_append(polygon, point);
    }

    Polyhedron_append(polyhedron, polygon);
  }

  fclose(file);
  return polyhedron;

}

Polyhedron *Polyhedron_from_parametric(
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

  const float dt = (tf - t0) / t_count;
  const float ds = (sf - s0) / s_count;

  // Point is a 2d array of v3
  // Major axis is t_idx, minor is s_idx
  // This array is almost always too large for the stack,
  // so we we have to dynamically allocate it.
  v3 *points = malloc(t_count * s_count * sizeof(v3));
#define point_at(i, j) (*(points + (size_t) (i) * (size_t) s_count + (size_t) (j)))
// use of size_t as per https://stackoverflow.com/a/3463745/4608364

#ifdef DEBUG
  if (points == NULL) {
    printf("malloc failed");
    exit(1);
  }
#endif

  for (int t_idx = 0; t_idx < t_count; t_idx++) {
    for (int s_idx = 0; s_idx < s_count; s_idx++) {
      const float t = t0 + t_idx * dt;
      const float s = s0 + s_idx * ds;

      const v3 v = f(t, s);
      point_at(t_idx, s_idx) = v;
    }
  }

  // Each quadruplet of adjacent items becomes a polygongon

  Polyhedron *polyhedron = Polyhedron_new(t_count * s_count);
  // t_count * s_count is the length if wrapping in both directions.
  // Thus it acts as an upper bound for all cases,
  // with maximum error t_count + s_count + 1, which is pretty low

  for (int t_idx = 0; t_idx < t_count - 1; t_idx++) {
    for (int s_idx = 0; s_idx < s_count - 1; s_idx++) {
      const v3 top_left     = point_at(t_idx    , s_idx    );
      const v3 top_right    = point_at(t_idx + 1, s_idx    );
      const v3 bottom_left  = point_at(t_idx    , s_idx + 1);
      const v3 bottom_right = point_at(t_idx + 1, s_idx + 1);

      Polygon *polygon = Polygon_from_points(4, top_left, top_right, bottom_right, bottom_left);
      Polyhedron_append(polyhedron, polygon);
    }
  }

  if (do_t_wrapping) {
    for (int s_idx = 0; s_idx < s_count - 1; s_idx++ ) {
      const v3 top_left     = point_at(t_count - 1, s_idx    );
      const v3 top_right    = point_at(0          , s_idx    );
      const v3 bottom_left  = point_at(t_count - 1, s_idx + 1);
      const v3 bottom_right = point_at(0          , s_idx + 1);

      Polygon *polygon = Polygon_from_points(4, top_left, top_right, bottom_right, bottom_left);
      Polyhedron_append(polyhedron, polygon);
    }
  }

  if (do_s_wrapping) {
    for (int t_idx = 0; t_idx < t_count - 1; t_idx++) {
      const v3 top_left     = point_at(t_idx    , s_count - 1);
      const v3 top_right    = point_at(t_idx + 1, s_count - 1);
      const v3 bottom_left  = point_at(t_idx    , 0          );
      const v3 bottom_right = point_at(t_idx + 1, 0          );

      Polygon *polygon = Polygon_from_points(4, top_left, top_right, bottom_right, bottom_left);
      Polyhedron_append(polyhedron, polygon);
    }
  }

  if (do_t_wrapping && do_s_wrapping) {
    const v3 top_left     = point_at(t_count - 1, s_count - 1);
    const v3 top_right    = point_at(0          , s_count - 1);
    const v3 bottom_left  = point_at(t_count - 1, 0          );
    const v3 bottom_right = point_at(0          , 0          );

    Polygon *polygon = Polygon_from_points(4, top_left, top_right, bottom_left, bottom_right);
    Polyhedron_append(polyhedron, polygon);
  }

  free(points);
  return polyhedron;

}

Polyhedron *Polyhedron_from_polygons(const int polygon_count, ...) {
  va_list args;
  va_start(args, polygon_count);

  Polyhedron *polyhedron = Polyhedron_new(polygon_count);

  for (int i = 0; i < polygon_count; i++) {
    Polygon *polygon = va_arg(args, Polygon*);
    Polyhedron_append(polyhedron, polygon);
  }

  va_end(args);

  return polyhedron;
}

#endif // polyhedron_c_INCLUDED

