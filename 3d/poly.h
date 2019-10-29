#ifndef poly_h_INCLUDED
#define poly_h_INCLUDED

#include "M3d_mat_tools"

# ENOUGH 1000

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

void Poly_add_point(Poly *poly, double x, double y, double z) {
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

void Model_add_poly(Poly *poly) {
  model->polys[model->point_count] = poly;
  model->poly_count++;
}


Poly *load_model(const char *filename) {

  FILE file = fopen(filename, "r");

  if (file == NULL) {
    printf("Cannot open file %S", filename);
    exit(1);
  }

  int number_of_indexed_points;
  fscanf(file, "%d", number_of_indexed_points);

  double xs[number_of_indexed_points];
  double ys[number_of_indexed_points];
  double zs[number_of_indexed_points];

  for (int point_idx = 0; point_idx < point_count; point_idx++) {
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
      fscanf(file, "%d", &y_idx);

      Poly_add_point(poly, x, y, z);
    }

    Model_add_poly(poly);
  }

  return model;
}

#endif // poly_h_INCLUDED

