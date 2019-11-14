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

void Poly_display(const Poly *poly) {
  double xs[poly->point_count];
  double ys[poly->point_count];
  for (int point_idx = 0; point_idx < poly->point_count; point_idx++) {
    const Point *p = poly->points[point_idx];
    pixel_coords_M(&xs[point_idx], &ys[point_idx], p);
  }

  G_rgb(1, 0, 0);
  G_fill_polygon(xs, ys, poly->point_count);

  G_rgb(1, 1, 1);
  for (int point_idx = 0; point_idx < poly->point_count; point_idx++) {
    const Point *p0 = poly->points[point_idx];
    const Point *pf = poly->points[(point_idx + 1) % poly->point_count];
    display_line(p0, pf);
  }
}

int comparator(const void *_poly0, const void *_poly1) {
  const Poly *poly0 = (const Poly *) _poly0;
  const Poly *poly1 = (const Poly *) _poly1;

  Point center0;
  Point center1;

  Poly_calc_center_M(&center0, poly0);
  Poly_calc_center_M(&center1, poly1);

  const double dist0 = PointVec_mag(&center0);
  const double dist1 = PointVec_mag(&center1);

  return dist1 - dist0;
}

void Model_display(Model *model) {
  qsort((void *) model->polys, model->poly_count, sizeof(Poly *), comparator);

  for (int poly_idx = 0; poly_idx < model->poly_count; poly_idx++) {
    Poly *poly = model->polys[poly_idx];
    Poly_display(poly);
  }
}


#endif // display_h_INCLUDED

