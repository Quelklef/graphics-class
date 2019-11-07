#ifndef model_h_INCLUDED
#define model_h_INCLUDED

// THe Model data type, which is a collection of polygons

#include <float.h>

#include "poly.h"
#include "matrix.h"

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

#endif // model_h_INCLUDED
