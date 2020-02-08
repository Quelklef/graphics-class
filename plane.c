#ifndef plane_c_INCLUDED
#define plane_c_INCLUDED

#include "misc.c"
#include "polygon.c"
#include "line.c"
#include "v3.c"

// We store a plane as a point and a normal
typedef struct Plane {
  v3 p0;
  v3 normal;
} Plane;

void Plane_from_point_normal(Plane *result, v3 p0, v3 normal) {
  if (v3_mag(normal) != 1) {
    printf("Error in Plane_from_point_normal: normal vector must have magnitude 1!\n");
    exit(1);
  }

  result->p0 = p0;
  result->normal = normal;
}

void Plane_from_points(Plane *result, v3 a, v3 b, v3 c) {
  v3 normal = v3_normalize(v3_cross(b - a, c - a));
  result->p0 = a;
  result->normal = normal;
}

void Plane_from_polygon(Plane *result, const Polygon *polygon) {
  if (polygon->length < 3) {
    printf("Error in Plane_from_polygon: polygon requires >= 3 points!\n");
    exit(1);
  }

  Plane_from_points(result, Polygon_get(polygon, 0), Polygon_get(polygon, 1), Polygon_get(polygon, 2));
}

v3 Polygon_normal(const Polygon *polygon) {
  Plane plane;
  Plane_from_polygon(&plane, polygon);
  return plane.normal;
}

int Plane_side_of(const Plane *plane, const v3 point) {
  // Return the side of the plane that the point is on
  // Return 1 for one side, -1 for the other, and 0 if the points is on the plane.

  v3 diff = point - plane->p0;
  return sgn(v3_dot(plane->normal, diff));
}

int Plane_intersect_line_M(v3 *result, const Plane *plane, const Line *line) {
  // Return whether or not an intersection was foudn

  v3 movement = Line_vector(line);

  // Check if line parallel to plane
  const float denom = v3_dot(plane->normal, movement);
  if (denom == 0) return 0;

  v3 diff = plane->p0 - line->p0;
  const float t = v3_dot(plane->normal, diff) / denom;

  movement *= t;

  *result = line->p0 + movement;
  return 1;
}

#endif // plane_c_INCLUDED

