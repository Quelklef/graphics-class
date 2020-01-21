#ifndef model_h_INCLUDED
#define model_h_INCLUDED

// The Model data type, which is a collection of polygons

#include <float.h>

#include "pointvec.h"
#include "poly.h"
#include "matrix.h"

typedef struct Model {
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

void Model_destroy(Model *model) {
  for (int poly_idx = 0; poly_idx < model->poly_count; poly_idx++) {
    Poly_destroy(model->polys[poly_idx]);
  }
  free(model);
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

Model *Model_clone(const Model *source) {
  Model *result = Model_new();
  for (int poly_i = 0; poly_i < source->poly_count; poly_i++) {
    const Poly *poly = source->polys[poly_i];
    Poly *clone = Poly_clone(poly);
    Model_add_poly(result, clone);
  }
  return result;
}

void Model_transform(Model *model, const _Mat transformation) {
  for (int poly_idx = 0; poly_idx < model->poly_count; poly_idx++) {
    Poly *poly = model->polys[poly_idx];
    Poly_transform(poly, transformation);
  }
}

void Model_bounds_M(
      double *result_min_x, double *result_max_x,
      double *result_min_y, double *result_max_y,
      double *result_min_z, double *result_max_z,
      const Model *model
    ) {

  *result_min_x = +DBL_MAX;
  *result_max_x = -DBL_MAX;
  *result_min_y = +DBL_MAX;
  *result_max_y = -DBL_MAX;
  *result_min_z = +DBL_MAX;
  *result_max_z = -DBL_MAX;

  for (int poly_idx = 0; poly_idx < model->poly_count; poly_idx++) {
    const Poly *poly = model->polys[poly_idx];
    for (int point_idx = 0; point_idx < poly->point_count; point_idx++) {
      const Point *point = poly->points[point_idx];
      double x = point->x;
      double y = point->y;
      double z = point->z;

      if (x < *result_min_x) *result_min_x = x;
      if (x > *result_max_x) *result_max_x = x;
      if (y < *result_min_y) *result_min_y = y;
      if (y > *result_max_y) *result_max_y = y;
      if (z < *result_min_z) *result_min_z = z;
      if (z > *result_max_z) *result_max_z = z;
    }
  }
}

void Model_center_M(Point *result, const Model *model) {
  double min_x, max_x, min_y, max_y, min_z, max_z;

  Model_bounds_M(&min_x, &max_x, &min_y, &max_y, &min_z, &max_z, model);

  result->x = min_x / 2 + max_x / 2;
  result->y = min_y / 2 + max_y / 2;
  result->z = min_z / 2 + max_z / 2;
}

void Model_size_M(double *result_x_size, double *result_y_size, double *result_z_size, const Model *model) {
  double min_x, max_x, min_y, max_y, min_z, max_z;
  Model_bounds_M(&min_x, &max_x, &min_y, &max_y, &min_z, &max_z, model);
  *result_x_size = max_x - min_x;
  *result_y_size = max_y - min_y;
  *result_z_size = max_z - min_z;
}

void Model_move_to(Model *model, const Point *target) {
  Point model_center;
  Model_center_M(&model_center, model);

  _Mat translation;
  Mat_translation_M(
    translation,
    -model_center.x + target->x,
    -model_center.y + target->y,
    -model_center.z + target->z
  );

  Model_transform(model, translation);
}

void nicely_place_model(Model *model) {
  // Move the model to somewhere nice

  double min_x, max_x, min_y, max_y, min_z, max_z;
  Model_bounds_M(&min_x, &max_x, &min_y, &max_y, &min_z, &max_z, model);
  const double width = max_z - min_z;

  // We choose that "somewhere nice" means that x=y=0 and the closest z value is at some z
  const double desired_z = 15;
  Point desired_location;
  PointVec_init(&desired_location, 0, 0, width + desired_z);
  Model_move_to(model, &desired_location);
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

  fclose(file);

  return model;

}

Model *make_small_model() {
  /* Make a small model. No specified shape. Just small. */

  Point *p0 = PointVec_new(0, 0, 0);
  Point *p1 = PointVec_new(1, 0, 1);
  Point *p2 = PointVec_new(1, 1, 0);
  Point *p3 = PointVec_new(0, 1, 1);

  const double scale = 0.1;
  PointVec_scale(p0, scale);
  PointVec_scale(p1, scale);
  PointVec_scale(p2, scale);
  PointVec_scale(p3, scale);

  Poly *poly0 = Poly_new();
  Poly_add_point(poly0, PointVec_clone(p1));
  Poly_add_point(poly0, PointVec_clone(p2));
  Poly_add_point(poly0, PointVec_clone(p3));

  Poly *poly1 = Poly_new();
  Poly_add_point(poly1, PointVec_clone(p0));
  Poly_add_point(poly1, PointVec_clone(p2));
  Poly_add_point(poly1, PointVec_clone(p3));

  Poly *poly2 = Poly_new();
  Poly_add_point(poly2, PointVec_clone(p0));
  Poly_add_point(poly2, PointVec_clone(p1));
  Poly_add_point(poly2, PointVec_clone(p3));

  Poly *poly3 = Poly_new();
  Poly_add_point(poly3, PointVec_clone(p0));
  Poly_add_point(poly3, PointVec_clone(p1));
  Poly_add_point(poly3, PointVec_clone(p2));

  PointVec_destroy(p0);
  PointVec_destroy(p1);
  PointVec_destroy(p2);
  PointVec_destroy(p3);

  Model *model = Model_new();
  Model_add_poly(model, poly0);
  Model_add_poly(model, poly1);
  Model_add_poly(model, poly2);
  Model_add_poly(model, poly3);

  nicely_place_model(model);

  return model;
}

#endif // model_h_INCLUDED
