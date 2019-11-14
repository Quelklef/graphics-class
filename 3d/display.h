#ifndef display_h_INCLUDED
#define display_h_INCLUDED

// Functions relating to displaying stuff

#include <math.h>

#include "poly.h"
#include "model.h"
#include "pointvec.h"

void pixel_coords_M(double *result_x, double *result_y, const Point *point) {
  /* Find the pixel coordinates on the screen of a given
   * (x, y, z) point. */

  const double x_bar = point->x / point->z;
  const double y_bar = point->y / point->z;

  // Scale with respect to only width OR height, because
  // scaling with respect to both will deform the object
  // by stretching it.
  const double minor = fmin(screen_width, screen_height);

  const double H = tan(half_angle);
  const double x_bar_bar = x_bar / H * (minor / 2);
  const double y_bar_bar = y_bar / H * (minor / 2);

  *result_x = x_bar_bar + minor / 2;
  *result_y = y_bar_bar + minor / 2;
}

void display_line(const Point *p0, const Point *pf) {
  double pixel_x0, pixel_y0, pixel_xf, pixel_yf;
  pixel_coords_M(&pixel_x0, &pixel_y0, p0);
  pixel_coords_M(&pixel_xf, &pixel_yf, pf);

  G_line(pixel_x0, pixel_y0, pixel_xf, pixel_yf);
}

int shouldnt_display(const Poly *poly) {
  // Implements backface elimination
  Point origin;
  PointVec_init(&origin, 0, 0, 0);

  Vec T;
  PointVec_between_M(&T, &origin, poly->points[0]);

  Vec N;
  Poly_normal_M(&N, poly);

  return backface_elimination_sign * PointVec_dot(&T, &N) < 0;
}

int comparator(const void *_p0, const void *_p1) {
  const Point *p0 = (const Point *) _p0;
  const Point *p1 = (const Point *) _p1;

  const double dist0 = PointVec_mag(p0);
  const double dist1 = PointVec_mag(p1);

  return dist1 - dist0;
}

void Model_display(const Model *model) {
  G_rgb(1, 0, 0);
  for (int poly_idx = 0; poly_idx < model->poly_count; poly_idx++) {
    Poly *poly = model->polys[poly_idx];

    //qsort((void *) poly->points, poly->point_count, sizeof(Point *), comparator);

    if (shouldnt_display(poly)) continue;

    for (int point_idx = 0; point_idx < poly->point_count; point_idx++) {
      const Point *p0 = poly->points[point_idx];
      const Point *pf = poly->points[(point_idx + 1) % poly->point_count];
      display_line(p0, pf);
    }
  }
}


#endif // display_h_INCLUDED
