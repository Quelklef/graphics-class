#ifndef poly_h_INCLUDED
#define poly_h_INCLUDED

#include "M2d_mat_tools.c"

#define ENOUGH 200

const int screen_width = 800;
const int screen_height = 800;

typedef struct {
  double xs[ENOUGH];
  double ys[ENOUGH];
  int point_count;

  double red, green, blue;
} SimplePoly;

typedef struct {
  SimplePoly *simples[ENOUGH];
  int simple_count;

  // Each polygon also gets a matrix that will rotate
  // it a small amount
  double rotation_mat[3][3];
} Poly;

Poly polys[ENOUGH];
int poly_count;

void calc_poly_center(double *center_x, double *center_y, Poly *poly) {
  double min_x = DBL_MAX;
  double max_x = DBL_MIN;
  double min_y = DBL_MAX;
  double max_y = DBL_MIN;

  for (int simple_idx = 0; simple_idx < poly->simple_count; simple_idx++) {
    SimplePoly *simple = poly->simples[simple_idx];

    for (int point_idx = 0; point_idx < poly->simples[simple_idx]->point_count; point_idx++) {
      const int x = simple->xs[point_idx];
      const int y = simple->ys[point_idx];

      if      (x < min_x) min_x = x;
      else if (x > max_x) max_x = x;

      if      (y < min_y) min_y = y;
      else if (y > max_y) max_y = y;
    }
  }

  *center_x = (min_x + max_x) / 2;
  *center_y = (min_y + max_y) / 2;
}

void load_poly(const char *filename, const int poly_idx) {
  FILE *file = fopen(filename, "r");

  if (file == NULL) {
    printf("Can not open file: %s\n", filename);
    exit(1);
  }

  int point_count;
  fscanf(file, "%d", &point_count);

  // Initialize (x, y) arrays to store points
  double xs[point_count];
  double ys[point_count];

  for (int point_idx = 0; point_idx < point_count; point_idx++) {
     fscanf(file, "%lf %lf", &xs[point_idx], &ys[point_idx]);
  }

  Poly poly;
  fscanf(file, "%d", &poly.simple_count);

  for (int simple_idx = 0; simple_idx < poly.simple_count; simple_idx++) {
    SimplePoly *simple = malloc(sizeof(SimplePoly));
    fscanf(file, "%d", &simple->point_count);

    for (int point_idx = 0; point_idx < simple->point_count; point_idx++) {
      int point_crossref_idx;
      fscanf(file, "%d", &point_crossref_idx);

      simple->xs[point_idx] = xs[point_crossref_idx];
      simple->ys[point_idx] = ys[point_crossref_idx];
    }

    poly.simples[simple_idx] = simple;
  }
   
  for (int simple_idx = 0; simple_idx < poly.simple_count; simple_idx++) {
    fscanf(file, "%lf %lf %lf",
           &poly.simples[simple_idx]->red,
           &poly.simples[simple_idx]->green,
           &poly.simples[simple_idx]->blue);
  }


  { // Center the polygon
    double center_x, center_y;
    calc_poly_center(&center_x, &center_y, &poly);

    double translate_to_origin_mat[3][3];
    M2d_make_translation(translate_to_origin_mat, -center_x, -center_y);

    double translate_to_center_mat[3][3];
    M2d_make_translation(translate_to_center_mat, screen_width / 2, screen_height / 2);

    double composed[3][3];
    M2d_make_identity(composed);
    M2d_mat_mult(composed, translate_to_origin_mat, composed);
    M2d_mat_mult(composed, translate_to_center_mat, composed);

    for (int simple_idx = 0; simple_idx < poly.simple_count; simple_idx++) {
      SimplePoly *simple = poly.simples[simple_idx];
      M2d_mat_mult_points(simple->xs, simple->ys,
                          composed,
                          simple->xs, simple->ys, simple->point_count);
    }
  }

  { // Make the rotation matrix
    double center_x, center_y;
    calc_poly_center(&center_x, &center_y, &poly);

    double translate_to_origin_mat[3][3];
    M2d_make_translation(translate_to_origin_mat, -center_x, -center_y);

    double rotation_mat[3][3];
    M2d_make_rotation_degrees(rotation_mat, 1);

    double translate_from_origin_mat[3][3];
    M2d_make_translation(translate_from_origin_mat, center_x, center_y);

    M2d_make_identity(poly.rotation_mat);
    M2d_mat_mult(poly.rotation_mat, translate_to_origin_mat, poly.rotation_mat);
    M2d_mat_mult(poly.rotation_mat, rotation_mat, poly.rotation_mat);
    M2d_mat_mult(poly.rotation_mat, translate_from_origin_mat, poly.rotation_mat);
  }

  polys[poly_idx] = poly;
}

void rotate_poly(Poly *poly) {
  for (int simple_idx = 0; simple_idx < poly->simple_count; simple_idx++) {
    SimplePoly *simple = poly->simples[simple_idx];

    M2d_mat_mult_points(simple->xs, simple->ys,
                        poly->rotation_mat,
                        simple->xs, simple->ys, simple->point_count);
  }
}

#endif // poly_h_INCLUDED

