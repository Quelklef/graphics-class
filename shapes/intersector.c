#ifndef intersecor_c_INCLUDED
#define intersecor_c_INCLUDED

#include "line.c"

/*

The Intersector type inverts the normal flow
of a typical shape.

Instead of starting with the shape definition
which tells us where all the points are, the
idea behind intersecor is that we /ask/ it about
where its points are and it tells us.

*/

typedef struct {
  // In object space
  int (*intersect)(v3 *result, Line *line);

  // bounding box
  v3 min_corner;
  v3 max_corner;

  // object space to world space
  _Mat transformation;
} Intersector;


Intersector *Intersector_new(
  int (*intersect)(v3 *result, Line *line),
  v3 min_corner,
  v3 max_corner
) {
  Intersector *intersecor = malloc(sizeof(Intersector));

  intersecor->intersect = intersect;
  intersecor->min_corner = min_corner;
  intersecor->max_corner = max_corner;

  const _Mat id = Mat_identity();
  Mat_clone_M(intersecor->transformation, id);

  return intersecor;
}

void Intersector_destroy(Intersector *intersecor) {
  free(intersecor);
}

void Intersector_transform(Intersector *intersector, const _Mat transformation) {
  Mat_mult_M(intersector->transformation, intersector->transformation, transformation);
}

int Intersector_intersect(v3 *result, Intersector *intersector, const Line *line) {
  _Mat inverse;
  Mat_inv_M(inverse, intersector->transformation);

  Line clone;
  memcpy(&clone, line, sizeof(Line));
  Line_transform(&clone, inverse);

  v3 intersection;
  const int got_intersection = intersector->intersect(&intersection, &clone);
  if (!got_intersection) return 0;

  *result = v3_transform(intersection, intersector->transformation);
  return 1;
}

void Intersector_bounds_M(v3 *lows, v3 *highs, const Intersector *intersector) {
  *lows  = v3_transform(intersector->min_corner, intersector->transformation);
  *highs = v3_transform(intersector->max_corner, intersector->transformation);
}


#endif // intersecor_c_INCLUDED
