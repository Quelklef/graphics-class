#ifndef plane_h_INCLUDED
#define plane_h_INCLUDED

#include "misc.h"
#include "pointvec.h"
#include "poly.h"
#include "line.h"

// We store a plane as a point and a normal
typedef struct Plane {
  Point p0;
  Vec normal;
} Plane;

void Plane_from_point_normal(Plane *result, Point *p0, Vec *normal) {
  if (PointVec_magnitude(normal) != 1) {
    printf("Error in Plane_from_point_normal: normal vector must have magnitude 1!\n");
    exit(1);
  }

  result->p0 = *p0;
  result->normal = *normal;
}

void Plane_from_points(Plane *result, Point *a, Point *b, Point *c) {
  Vec v0;
  PointVec_between_M(&v0, a, b);

  Vec v1;
  PointVec_between_M(&v1, a, c);

  Vec normal;
  PointVec_cross_M(&normal, &v0, &v1);
  PointVec_normalize(&normal);

  result->p0 = *a;
  result->normal = normal;
}

void Plane_from_poly(Plane *result, const Poly *poly) {
  if (poly->point_count < 3) {
    printf("Error in Plane_from_poly: poly requires >= 3 points!\n");
    exit(1);
  }

  Plane_from_points(result, poly->points[0], poly->points[1], poly->points[2]);
}

void Poly_normal_M(Vec *result, const Poly *poly) {
  Plane plane;
  Plane_from_poly(&plane, poly);
  *result = plane.normal;
}

int Plane_side_of(const Plane *plane, const Point *point) {
  // Return the side of the plane that the point is on
  // Return 1 for one side, -1 for the other, and 0 if the points is on the plane.

  Vec diff;
  PointVec_subtract_M(&diff, point, &plane->p0);

  return sgn(PointVec_dot(&plane->normal, &diff));
}

int Plane_intersect_line_M(Point *result, const Plane *plane, const Line *line) {
  // Return whether or not an intersection was foudn

  Vec movement;
  Line_vector_M(&movement, line);

  // Check if line parallel to plane
  const double denom = PointVec_dot(&plane->normal, &movement);
  if (denom == 0) return 0;

  Vec diff;
  PointVec_subtract_M(&diff, &plane->p0, &line->p0);
  const double t = PointVec_dot(&plane->normal, &diff) / denom;

  PointVec_scale(&movement, t);

  PointVec_clone_M(result, &line->p0);
  PointVec_add(result, &movement);

  return 1;
}

#endif // plane_h_INCLUDED

