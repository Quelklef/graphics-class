#ifndef poly_h_INCLUDED
#define poly_h_INCLUDED

#include <stdlib.h>
#include <math.h>
#include <float.h>

#include <FPT.h>
#include "M3d_mat_tools.h"

#define ENOUGH 5000

#define screen_width 800
#define screen_height 800

typedef struct {
  double xs[ENOUGH];
  double ys[ENOUGH];
  double zs[ENOUGH];
  int point_count;
} Poly;

Poly *Poly_new() {
  Poly *poly = malloc(sizeof(Poly));
  poly->point_count = 0;
  return poly;
}

void Poly_add_point(Poly *poly, const double x, const double y, const double z) {
  poly->xs[poly->point_count] = x;
  poly->ys[poly->point_count] = y;
  poly->zs[poly->point_count] = z;
  poly->point_count++;
}

void Poly_calc_center(const Poly *poly, double *center_x, double *center_y, double *center_z) {
  double min_x = DBL_MAX;
  double max_x = DBL_MIN;
  double min_y = DBL_MAX;
  double max_y = DBL_MIN;
  double min_z = DBL_MAX;
  double max_z = DBL_MIN;

  for (int point_idx = 0; point_idx < poly->point_count; point_idx++) {
    double x = poly->xs[point_idx];
    double y = poly->ys[point_idx];
    double z = poly->zs[point_idx];

    if (x < min_x) min_x = x;
    if (x > max_x) max_x = x;
    if (y < min_y) min_y = y;
    if (y > max_y) max_y = y;
    if (z < min_z) min_z = z;
    if (z > max_z) max_z = z;
  }

  *center_x = min_x / 2 + max_x / 2;
  *center_y = min_y / 2 + max_y / 2;
  *center_z = min_z / 2 + max_z / 2;
}

typedef struct {
  Poly *polys[ENOUGH];
  int poly_count;
} Model;

Model *Model_new() {
  Model *model = malloc(sizeof(Model));
  model->poly_count = 0;
  return model;
}

void Model_add_poly(Model *model, Poly *poly) {
  model->polys[model->poly_count] = poly;
  model->poly_count++;
}

void Model_transform(Model *model, const double transformation[4][4]) {
  for (int poly_idx = 0; poly_idx < model->poly_count; poly_idx++) {
    Poly *poly = model->polys[poly_idx];
    M3d_mat_mult_points(
      poly->xs, poly->ys, poly->zs,
      transformation,
      poly->xs, poly->ys, poly->zs,
      poly->point_count
    );
  }
}

void Model_calc_center(const Model *model, double *center_x, double *center_y, double *center_z) {
  /* Calculates the center of the bounding box */
  double min_x = DBL_MAX;
  double max_x = DBL_MIN;
  double min_y = DBL_MAX;
  double max_y = DBL_MIN;
  double min_z = DBL_MAX;
  double max_z = DBL_MIN;

  for (int poly_idx = 0; poly_idx < model->poly_count; poly_idx++) {
    Poly *poly = model->polys[poly_idx];
    for (int point_idx = 0; point_idx < poly->point_count; point_idx++) {
      double x = poly->xs[point_idx];
      double y = poly->ys[point_idx];
      double z = poly->zs[point_idx];

      if (x < min_x) min_x = x;
      if (x > max_x) max_x = x;
      if (y < min_y) min_y = y;
      if (y > max_y) max_y = y;
      if (z < min_z) min_z = z;
      if (z > max_z) max_z = z;
    }
  }

  *center_x = min_x / 2 + max_x / 2;
  *center_y = min_y / 2 + max_y / 2;
  *center_z = min_z / 2 + max_z / 2;
}


Model *load_model(const char *filename) {

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

  Model *model = Model_new();

  int model_poly_count;
  fscanf(file, "%d", &model_poly_count);

  for (int poly_idx = 0; poly_idx < model_poly_count; poly_idx++) {
    Poly *poly = Poly_new();

    int poly_point_count;
    fscanf(file, "%d", &poly_point_count);

    for (int point_idx = 0; point_idx < poly_point_count; point_idx++) {
      int crossref_idx;
      fscanf(file, "%d", &crossref_idx);
      Poly_add_point(poly, xs[crossref_idx], ys[crossref_idx], zs[crossref_idx]);
    }

    Model_add_poly(model, poly);
  }

  return model;

}

typedef struct {
  double x;
  double y;
  double z;
} Vec3;

Vec3 *Vec3_new(double x, double y, double z) {
  Vec3 *v = malloc(sizeof(Vec3));
  v->x = x;
  v->y = y;
  v->z = z;
  return v;
}

Vec3 *Vec3_between(const double x0, const double y0, const double z0,
                  const double xf, const double yf, const double zf) {
  return Vec3_new(xf - x0, yf - y0, zf - z0);
}

double Vec3_dot(const Vec3 *A, const Vec3 *B) {
  return A->x * B->x + A->y * B->y + A->z * B->z;
}

Vec3 *Vec3_cross(const Vec3 *A, const Vec3 *B) {
  return Vec3_new(+(A->y * B->z - B->y * A->z),
                  -(A->x * B->z - B->x * A->z),
                  +(A->x * B->y - B->x * A->y));
}

double Vec3_mag(const Vec3 *A) {
  return sqrt(pow(A->x, 2) + pow(A->y, 2) + pow(A->z, 2));
}

Vec3 *Vec3_scale(const Vec3 *A, double c) {
  return Vec3_new(A->x * c, A->y * c, A->z * c);
}

Vec3 *Vec3_normalize(const Vec3 *A) {
  return Vec3_scale(A, 1 / Vec3_mag(A));
}

Vec3 *Vec3_neg(const Vec3 *A) {
  return Vec3_scale(A, -1);
}

#define half_angle (M_PI / 4)

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

Vec3 *plane_normal(const double *xs, const double *ys, const double *zs, const int point_count) {
  // Takes a polygon which MUST have at least 3 points
  // Returns the (length-1) normal veector to the polygon
  if (point_count < 3) {
    printf("ERROR: In plane_normal(): requires point_count >= 3");
    exit(1);
  }
  const Vec3 *A = Vec3_normalize(Vec3_between(xs[0], ys[0], zs[0], xs[1], ys[1], zs[1]));
  const Vec3 *B = Vec3_normalize(Vec3_between(xs[0], ys[0], zs[0], xs[2], ys[2], zs[2]));
  const Vec3 *C = Vec3_neg(Vec3_cross(A, B));
  return C;
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

#endif // poly_h_INCLUDED
