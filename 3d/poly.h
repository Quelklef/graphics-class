#ifndef poly_h_INCLUDED
#define poly_h_INCLUDED

#include <stdlib.h>
#include <math.h>

#include "M3d_mat_tools.h"

#define ENOUGH 5000

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
      int x_idx, y_idx, z_idx;
      fscanf(file, "%d", &x_idx);
      fscanf(file, "%d", &y_idx);
      fscanf(file, "%d", &z_idx);

      Poly_add_point(poly, xs[x_idx], ys[y_idx], zs[z_idx]);
    }

    Model_add_poly(model, poly);
  }

  return model;

}

#define half_angle (M_PI / 4)

void display_point(
      const double x, const double y, const double z,
      const double screen_width, const double screen_height
    ) {

  const double t = 1 / z;
  const double x_prime = t * x;
  const double y_prime = t * y;

  const double H = tan(half_angle);
  const double pixel_x = (screen_width  / 2) / H * x_prime + (screen_width  / 2);
  const double pixel_y = (screen_height / 2) / H * y_prime + (screen_height / 2);

  printf("%lf, %lf, %lf -> %lf, %lf\n", x, y, z, pixel_x, pixel_y);
  G_fill_rectangle(pixel_x - 2, pixel_y - 2, 4, 4);
}

void Model_display(const Model *model, const double screen_width, const double screen_height) {
  G_rgb(1, 0, 0);
  for (int poly_idx = 0; poly_idx < model->poly_count; poly_idx++) {
    Poly *poly = model->polys[poly_idx];
    for (int point_idx = 0; point_idx < poly->point_count; point_idx++) {
      display_point(
        poly->xs[point_idx], poly->ys[point_idx], poly->zs[point_idx],
        screen_width, screen_height
      );
    }
  }
}

#endif // poly_h_INCLUDED

