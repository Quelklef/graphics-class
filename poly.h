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

// A 'clip', i.e., mask, must be a simply polygon
typedef SimplePoly Clip;

typedef struct {
  SimplePoly *simples[ENOUGH];
  int simple_count;

  // Each polygon also gets a matrix that will rotate
  // it a small amount
  double rotation_mat[3][3];
} Poly;

Poly polys[ENOUGH];
int poly_count;

void SimplePoly_add_point(SimplePoly *simple, double x, double y) {
  simple->xs[simple->point_count] = x;
  simple->ys[simple->point_count] = y;
  simple->point_count++;
}

void promote(SimplePoly *simple, Poly *poly) {
  // Promote a simply polygon to a complex polygon
  // Does not calculate rotation matrix
  poly->simple_count = 1;
  poly->simples[0] = simple;
}

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
const int click_bar_height = 20;

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

void draw_click_bar() {
  G_rgb(1, 0, 0);
  G_fill_rectangle(0, 0, click_bar_width, click_bar_height);
}

void draw_bounding_box() {
  G_rgb(1, 0, 0);
  G_fill_rectangle(0, 0, screen_width, 1);
  G_fill_rectangle(screen_width - 1, 0, 1, screen_height);
  G_fill_rectangle(0, screen_height - 1, screen_width, 1);
  G_fill_rectangle(0, 0, 1, screen_height);
}

void click_and_save_point(double *x, double *y) {
  G_rgb(1, 0, 0);

  G_display_image();
  double click[2];
  G_wait_click(click);

  *x = click[0];
  *y = click[1];

  G_fill_rectangle(*x - 2, *y - 2, 4, 4);
}

SimplePoly *click_and_save_simple() {
  /* Allow the user to click several points, and return the resultant
   * polygon. Assumes that the user has outlined a simple polygon. */

  G_rgb(1, 0, 0);

  SimplePoly *simple = malloc(sizeof(Clip));
  simple->point_count = 0;

  while (1) {
    double x, y;
    click_and_save_point(&x, &y);

    if (x < click_bar_width && y < click_bar_height) {
      break;
    } else {
      simple->xs[simple->point_count] = x;
      simple->ys[simple->point_count] = y;

      if (simple->point_count > 0) {
        G_line(simple->xs[simple->point_count - 1], simple->ys[simple->point_count - 1],
               x, y);
      }

      simple->point_count++;
    }
  }

  return simple;
}

int side(double x0, double y0, double xf, double yf, double x, double y) {
  /* Considers the line from (x0, y0) to (xf, yf), extended infinitely.
   * Returns either -1 or 1 based on which side of the line (x, y) is on.
   * The only guaratee is that two points which are on the same side of
   * the line will get the same result, and points on opposite sides will
   * get different results.
   * If the point is on the line, return 1. */

   double v = (xf - x0) * (y - y0) - (yf - y0) * (x - x0);
   return v < 0 ? -1 : 1;
}

int simple_convex_contains(SimplePoly *simple, double x, double y) {
  // Return if a simple convex polygon contains a point
  double simple_center_x, simple_center_y;

  Poly promoted;
  promote(simple, &promoted);
  calc_poly_center(&simple_center_x, &simple_center_y, &promoted);

  for (int point_idx = 0; point_idx < simple->point_count; point_idx++) {
    double x0 = simple->xs[point_idx];
    double y0 = simple->ys[point_idx];
    double xf = simple->xs[(point_idx + 1) % simple->point_count];
    double yf = simple->ys[(point_idx + 1) % simple->point_count];

    int center_side = side(x0, y0, xf, yf, simple_center_x, simple_center_y);
    int point_side = side(x0, y0, xf, yf, x, y);

    if (center_side != point_side) return 0;
  }

  return 1;
}

void TEST_simple_convex_contains() {
  init_gfx(1);

  SimplePoly *simple = click_and_save_simple();

  G_rgb(1, 0, 0);
  for (int point_idx = 0; point_idx < simple->point_count; point_idx++) {
    double x0 = simple->xs[point_idx];
    double y0 = simple->ys[point_idx];
    double xf = simple->xs[(point_idx + 1) % simple->point_count];
    double yf = simple->ys[(point_idx + 1) % simple->point_count];
    G_line(x0, y0, xf, yf);
  }

  double center_x, center_y;
  Poly promoted;
  promote(simple, &promoted);
  calc_poly_center(&center_x, &center_y, &promoted);

  G_rgb(0, 0, 1);
  G_fill_rectangle(center_x - 4, center_y - 4, 8, 8);

  while (1) {
    double x, y;
    click_and_save_point(&x, &y);

    if (simple_convex_contains(simple, x, y)) {
      G_rgb(0, 1, 0);
    } else {
      G_rgb(1, 0, 0);
    }

    G_fill_rectangle(x - 2, y - 2, 4, 4);
  }
}

int find_lines_intersection(
    double Ax0, double Ay0, double Axf, double Ayf,
    double Bx0, double By0, double Bxf, double Byf,
    double ts[2], double intersection[2]) {
  // Solve for the intersection of the lines
  // s0 + t0 * d0
  // and
  // s1 + t1 * d1
  // Set ts[0] = solution t0,
  //     ts[1] = solution t1
  // And intersection = the interesection point
  // Return whether or not an intersection was found

  // Did a bunch of math, and got this:

  double denom = (Ayf - Ay0) * (Bxf - Bx0) - (Axf - Ax0) * (Byf - By0);
  if (denom == 0) {
    return 0;
  }

  ts[0] = ((Bxf - Bx0) * (By0 - Ay0) + (Byf - By0) * (Ax0 - Bx0)) / denom;
  ts[1] = ((Axf - Ax0) * (By0 - Ay0) + (Ayf - Ay0) * (Ax0 - Bx0)) / denom;
  intersection[0] = Ax0 + ts[0] * (Axf - Ax0);
  intersection[1] = Ay0 + ts[0] * (Ayf - Ay0);

  return 1;
}

void TEST_find_lines_intersection() {
  init_gfx(0);

  while (1) {
    double startX0, startY0, endX0, endY0,
           startX1, startY1, endX1, endY1;

    click_and_save_point(&startX0, &startY0);
    click_and_save_point(&endX0, &endY0);
    G_line(startX0, startY0, endX0, endY0);

    click_and_save_point(&startX1, &startY1);
    click_and_save_point(&endX1, &endY1);
    G_line(startX1, startY1, endX1, endY1);

    G_rgb(1, 0, 0);

    double ts[2];
    double intersection[2];
    find_lines_intersection(startX0, startY0, endX0, endY0,
                            startX1, startY1, endX1, endY1,
                            ts, intersection);
    G_rgb(0, 1, 0);
    G_fill_rectangle(intersection[0] - 2, intersection[1] - 2, 4, 4);

  }
}

void clip_simple(SimplePoly *simple, Clip *clip, SimplePoly *clipped) {
  // TODO
}

#endif // poly_h_INCLUDED

