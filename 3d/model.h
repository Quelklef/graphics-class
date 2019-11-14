#ifndef model_h_INCLUDED
#define model_h_INCLUDED

// The Model data type, which is a collection of polygons

#include <float.h>

#include "pointvec.h"
#include "poly.h"
#include "matrix.h"

typedef struct {
  Poly *polys[ENOUGH];
  int poly_count;
} Model;

void Model_init(Model *model) {
  model->poly_count = 0;
}

Model *Model_new() {
  Model *model = malloc(sizeof(Model));
  Model_init(model);
  return model;
}

void Model_print(const Model *model) {
  printf("MODEL [\n");
  for (int poly_idx = 0; poly_idx < model->poly_count; poly_idx++) {
    Poly_print(model->polys[poly_idx]);
  }
  printf("] MODEL\n");
}

void Model_add_poly(Model *model, Poly *poly) {
  model->polys[model->poly_count] = poly;
  model->poly_count++;
}

void Model_transform(Model *model, const _Mat transformation) {
  for (int poly_idx = 0; poly_idx < model->poly_count; poly_idx++) {
    Poly *poly = model->polys[poly_idx];
    Poly_transform(poly, transformation);
  }
}

void Model_center_M(Point *result, const Model *model) {
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
      const Point *point = poly->points[point_idx];
      double x = point->x;
      double y = point->y;
      double z = point->z;

      if (x < min_x) min_x = x;
      if (x > max_x) max_x = x;
      if (y < min_y) min_y = y;
      if (y > max_y) max_y = y;
      if (z < min_z) min_z = z;
      if (z > max_z) max_z = z;
    }
  }

  result->x = min_x / 2 + max_x / 2;
  result->y = min_y / 2 + max_y / 2;
  result->z = min_z / 2 + max_z / 2;
}

void nicely_place_model(Model *model) {
  // Move the model to somewhere nice
  Point model_center;
  Model_center_M(&model_center, model);

  double min_z = DBL_MAX;
  for (int poly_idx = 0; poly_idx < model->poly_count; poly_idx++) {
    const Poly *poly = model->polys[poly_idx];
    for (int point_idx = 0; point_idx < poly->point_count; point_idx++) {
      const Point *point = poly->points[point_idx];
      const double z = point->z;
      if (z < min_z) min_z = z;
    }
  }

  // We choose that "somewhere nice" means that x=y=0 and the closest z value is at some z
  const desired_z = 15;
  _Mat to_nice;
  Mat_translation_M(to_nice, -model_center.x, -model_center.y, -min_z + desired_z);

  Model_transform(model, to_nice);
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

      if (crossref_idx > number_of_indexed_points) {
        printf("Internal error in load_model.\n");
        exit(1);
      }

      Point *point = PointVec_new(xs[crossref_idx], ys[crossref_idx], zs[crossref_idx]);
      Poly_add_point(poly, point);
    }

    Model_add_poly(model, poly);
  }

  nicely_place_model(model);

  return model;

}

#endif // model_h_INCLUDED
