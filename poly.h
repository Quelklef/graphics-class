#ifndef poly_h_INCLUDED
#define poly_h_INCLUDED

#include "M2d_mat_tools.c"

#define screen_width 800
#define screen_height 800

#define ENOUGH 200

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





const int click_bar_width = screen_width;
const int click_bar_height = 10;

void init_gfx(int do_click_bar) {
  G_init_graphics(screen_width, screen_height);

  G_rgb(0, 0, 0);
  G_clear();

  if (do_click_bar) {
    // Make clickable bottom bar
    G_rgb(1, 0, 0);
    G_fill_rectangle(0, 0, click_bar_width, click_bar_height);
    G_rgb(0, 0, 0);
  }
}

Poly *click_and_save_simple() {
  /* Allow the user to click several points, and return the resultant
   * polygon. Assumes that the user has outlined a simple polygon. */

  SimplePoly *simple = malloc(sizeof(SimplePoly));
  simple->point_count = 0;

  while (1) {
    double click[2];
    G_wait_click(click);

    if (click[0] < click_bar_width && click[1] < click_bar_height) {
      break;
    } else {
      simple->xs[simple->point_count] = click[0];
      simple->ys[simple->point_count] = click[1];
      simple->point_count++;

      if (simple->point_count > 1) {
        G_line(simple->xs[simple->point_count - 1], simple->ys[simple->point_count - 1],
               click[0], click[1]);
      }
    }
  }

  Poly* poly = malloc(sizeof(Poly));
  poly->simples[0] = simple;
  poly->simple_count = 1;

  return poly;
}

#endif // poly_h_INCLUDED

