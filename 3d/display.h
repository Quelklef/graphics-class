#ifndef display_h_INCLUDED
#define display_h_INCLUDED

// Functions relating to displaying stuff

#include "poly.h"
#include "model.h"
#include "vec3.h"

void pixel_coords(
      const double x, const double y, const double z,
      double *pixel_x, double *pixel_y
    ) {
  /* Find the pixel coordinates on the screen of a given
   * (x, y, z) point. */

  const double x_bar = x / z;
  const double y_bar = y / z;

  // Scale with respect to only width OR height, because
  // scaling with respect to both will deform the object
  // by stretching it.
  const double minor = fmin(screen_width, screen_height);

  const double H = tan(half_angle);
  const double x_bar_bar = x_bar / H * (minor / 2);
  const double y_bar_bar = y_bar / H * (minor / 2);

  *pixel_x = x_bar_bar + minor / 2;
  *pixel_y = y_bar_bar + minor / 2;
}

void display_line(
      const double x0, const double y0, const double z0,
      const double xf, const double yf, const double zf
    ) {

  double pixel_x0, pixel_y0, pixel_xf, pixel_yf;
  pixel_coords(x0, y0, z0, &pixel_x0, &pixel_y0);
  pixel_coords(xf, yf, zf, &pixel_xf, &pixel_yf);

  G_line(pixel_x0, pixel_y0, pixel_xf, pixel_yf);
}

void display_vec(const double x0, const double y0, const double z0, const Vec3 *A) {
  display_line(x0, y0, z0, x0 + A->x, y0 + A->y, z0 + A->z);
}

int shouldnt_display(const Poly *poly) {
  // Implements backface elimination
  const Vec3 *T = Vec3_between(0, 0, 0, poly->xs[0], poly->ys[0], poly->zs[0]);
  const Vec3 *N = plane_normal(poly->xs, poly->ys, poly->zs, poly->point_count);

  return Vec3_dot(T, N) < 0;
}

void Model_display(const Model *model) {
  G_rgb(1, 0, 0);
  for (int poly_idx = 0; poly_idx < model->poly_count; poly_idx++) {
    Poly *poly = model->polys[poly_idx];

    if (shouldnt_display(poly)) continue;

    for (int point_idx = 0; point_idx < poly->point_count; point_idx++) {
      display_line(
        poly->xs[point_idx],
        poly->ys[point_idx],
        poly->zs[point_idx],

        poly->xs[(point_idx + 1) % poly->point_count],
        poly->ys[(point_idx + 1) % poly->point_count],
        poly->zs[(point_idx + 1) % poly->point_count]
      );
    }
  }
}


#endif // display_h_INCLUDED

