#ifndef model_h_INCLUDED
#define model_h_INCLUDED

// The Model data type, which is a collection of polygons

#include <float.h>

#include "v3.h"
#include "poly.h"
#include "matrix.h"

#include "dyn.h"
DYN_INIT(Model, Poly*)

void Model_print(const Model *model) {
  printf("MODEL [\n");
  for (int poly_idx = 0; poly_idx < model->length; poly_idx++) {
    Poly_print(Model_get(model, poly_idx));
  }
  printf("] MODEL\n");
}

Model *Model_clone(const Model *source) {
  Model *result = Model_new(source->length);
  for (int poly_i = 0; poly_i < source->length; poly_i++) {
    const Poly *poly = Model_get(source, poly_i);
    Poly *clone = Poly_clone(poly);
    Model_append(result, clone);
  }
  return result;
}

void Model_transform(Model *model, const _Mat transformation) {
  for (int poly_idx = 0; poly_idx < model->length; poly_idx++) {
    Poly *poly = Model_get(model, poly_idx);
    Poly_transform(poly, transformation);
  }
}

void Model_destroy(Model *model) {
  for (int i = 0; i < model->length; i++) {
    Poly_destroy(Model_get(model, i));
  }
  Dyn_destroy(model);
}

void Model_bounds_M(
      float *result_min_x, float *result_max_x,
      float *result_min_y, float *result_max_y,
      float *result_min_z, float *result_max_z,
      const Model *model
    ) {

  *result_min_x = +DBL_MAX;
  *result_max_x = -DBL_MAX;
  *result_min_y = +DBL_MAX;
  *result_max_y = -DBL_MAX;
  *result_min_z = +DBL_MAX;
  *result_max_z = -DBL_MAX;

  for (int poly_idx = 0; poly_idx < model->length; poly_idx++) {
    const Poly *poly = Model_get(model, poly_idx);
    for (int point_idx = 0; point_idx < poly->length; point_idx++) {
      const v3 point = Poly_get(poly, point_idx);

      const float x = point[0];
      const float y = point[1];
      const float z = point[2];

           if (x < *result_min_x) *result_min_x = x;
      else if (x > *result_max_x) *result_max_x = x;
           if (y < *result_min_y) *result_min_y = y;
      else if (y > *result_max_y) *result_max_y = y;
           if (z < *result_min_z) *result_min_z = z;
      else if (z > *result_max_z) *result_max_z = z;
    }
  }
}

v3 Model_center(const Model *model) {
  float min_x, max_x, min_y, max_y, min_z, max_z;

  Model_bounds_M(&min_x, &max_x, &min_y, &max_y, &min_z, &max_z, model);

  return (v3) {
    min_x / 2 + max_x / 2,
    min_y / 2 + max_y / 2,
    min_z / 2 + max_z / 2
  };
}

void Model_size_M(float *result_x_size, float *result_y_size, float *result_z_size, const Model *model) {
  float min_x, max_x, min_y, max_y, min_z, max_z;
  Model_bounds_M(&min_x, &max_x, &min_y, &max_y, &min_z, &max_z, model);
  *result_x_size = max_x - min_x;
  *result_y_size = max_y - min_y;
  *result_z_size = max_z - min_z;
}

void Model_move_to(Model *model, const v3 target) {
  v3 model_center = Model_center(model);

  const _Mat translation = Mat_translate_v(-model_center + target);
  Model_transform(model, translation);
}

void nicely_place_model(Model *model) {
  // Move the model to somewhere nice

  float min_x, max_x, min_y, max_y, min_z, max_z;
  Model_bounds_M(&min_x, &max_x, &min_y, &max_y, &min_z, &max_z, model);
  const float width = max_z - min_z;

  // We choose that "somewhere nice" means that x=y=0 and the closest z value is at some z
  const float desired_z = 15;
  v3 desired_location = (v3) { 0, 0, width + desired_z };
  Model_move_to(model, desired_location);
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

  int model_poly_count;
  fscanf(file, "%d", &model_poly_count);

  Model *model = Model_new(model_poly_count);

  for (int poly_idx = 0; poly_idx < model_poly_count; poly_idx++) {

    int poly_point_count;
    fscanf(file, "%d", &poly_point_count);

    Poly *poly = Poly_new(poly_point_count);

    for (int point_idx = 0; point_idx < poly_point_count; point_idx++) {
      int crossref_idx;
      fscanf(file, "%d", &crossref_idx);

      if (crossref_idx > number_of_indexed_points) {
        printf("Internal error in load_model.\n");
        exit(1);
      }

      v3 point = {
        (float) xs[crossref_idx],
        (float) ys[crossref_idx],
        (float) zs[crossref_idx]
      };
      Poly_append(poly, point);
    }

    Model_append(model, poly);
  }

  // TODO: this should not be in this location
  nicely_place_model(model);

  fclose(file);

  return model;

}

Model *Model_from_parametric(
  v3 (*f)(float t, float s),
  const float t0,
  const float tf,
  const int t_count,
  const int do_t_wrapping,
  const float s0,
  const float sf,
  const int s_count,
  const int do_s_wrapping
) {

  const float dt = (tf - t0) / t_count;
  const float ds = (sf - s0) / s_count;

  // Point is a 2d array of v3
  // Major axis is t_idx, minor is s_idx
  // This array is almost always too large for the stack,
  // so we we have to dynamically allocate it.
  v3 *points = malloc(t_count * s_count * sizeof(v3));
#define point_at(i, j) (*(points + (size_t) (i) * (size_t) s_count + (size_t) (j)))
// use of size_t as per https://stackoverflow.com/a/3463745/4608364

  if (points == NULL) {
    printf("malloc failed");
    exit(1);
  }

  for (int t_idx = 0; t_idx < t_count; t_idx++) {
    for (int s_idx = 0; s_idx < s_count; s_idx++) {
      const float t = t0 + t_idx * dt;
      const float s = s0 + s_idx * ds;

      const v3 v = f(t, s);
      point_at(t_idx, s_idx) = v;
    }
  }

  // Each quadruplet of adjacent items becomes a polygon

  Model *model = Model_new(t_count * s_count);
  // t_count * s_count is the length if wrapping in both directions.
  // Thus it acts as an upper bound for all cases,
  // with maximum error t_count + s_count + 1, which is pretty low

  for (int t_idx = 0; t_idx < t_count - 1; t_idx++) {
    for (int s_idx = 0; s_idx < s_count - 1; s_idx++) {
      const v3 top_left     = point_at(t_idx    , s_idx    );
      const v3 top_right    = point_at(t_idx + 1, s_idx    );
      const v3 bottom_left  = point_at(t_idx    , s_idx + 1);
      const v3 bottom_right = point_at(t_idx + 1, s_idx + 1);

      Poly *poly = Poly_from_points(4, top_left, top_right, bottom_right, bottom_left);
      Model_append(model, poly);
    }
  }

  if (do_t_wrapping) {
    for (int s_idx = 0; s_idx < s_count - 1; s_idx++ ) {
      const v3 top_left     = point_at(t_count - 1, s_idx    );
      const v3 top_right    = point_at(0          , s_idx    );
      const v3 bottom_left  = point_at(t_count - 1, s_idx + 1);
      const v3 bottom_right = point_at(0          , s_idx + 1);

      Poly *poly = Poly_from_points(4, top_left, top_right, bottom_right, bottom_left);
      Model_append(model, poly);
    }
  }

  if (do_s_wrapping) {
    for (int t_idx = 0; t_idx < t_count - 1; t_idx++) {
      const v3 top_left     = point_at(t_idx    , s_count - 1);
      const v3 top_right    = point_at(t_idx + 1, s_count - 1);
      const v3 bottom_left  = point_at(t_idx    , 0          );
      const v3 bottom_right = point_at(t_idx + 1, 0          );

      Poly *poly = Poly_from_points(4, top_left, top_right, bottom_right, bottom_left);
      Model_append(model, poly);
    }
  }

  if (do_t_wrapping && do_s_wrapping) {
    const v3 top_left     = point_at(t_count - 1, s_count - 1);
    const v3 top_right    = point_at(0          , s_count - 1);
    const v3 bottom_left  = point_at(t_count - 1, 0          );
    const v3 bottom_right = point_at(0          , 0          );

    Poly *poly = Poly_from_points(4, top_left, top_right, bottom_left, bottom_right);
    Model_append(model, poly);
  }

  free(points);
  return model;

}

Model *Model_from_polys(const int poly_count, ...) {
  va_list args;
  va_start(args, poly_count);

  Model *model = Model_new(poly_count);

  for (int i = 0; i < poly_count; i++) {
    Poly *poly = va_arg(args, Poly*);
    Model_append(model, poly);
  }

  va_end(args);

  return model;
}

Model *make_small_model() {
  /* Make a small model. No specified size or shape. Just small. */

  const float scale = 0.1;

  const v3 p0 = scale * (v3) { 0, 0, 0 };
  const v3 p1 = scale * (v3) { 1, 0, 1 };
  const v3 p2 = scale * (v3) { 1, 1, 0 };
  const v3 p3 = scale * (v3) { 0, 1, 1 };

  Poly *poly0 = Poly_from_points(3,     p1, p2, p3);
  Poly *poly1 = Poly_from_points(3, p0,     p2, p3);
  Poly *poly2 = Poly_from_points(3, p0, p1,     p3);
  Poly *poly3 = Poly_from_points(3, p0, p1, p2    );

  Model *model = Model_from_polys(4, poly0, poly1, poly2, poly3);

  // TODO: this should not be in this location
  nicely_place_model(model);

  return model;
}

#endif // model_h_INCLUDED

