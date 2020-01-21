#include <stdio.h>
#include <FPT.h>
#include <float.h>

#include "poly.h"

void draw_poly(Poly *poly, Clip *clip) {
  draw_bounding_box();

  for (int simple_idx = 0; simple_idx < poly->simple_count; simple_idx++) {
    SimplePoly *simple = poly->simples[simple_idx];

    SimplePoly clipped;
    clip_simple(simple, clip, &clipped);

    G_rgb(simple->red, simple->blue, simple->green);
    G_fill_polygon(clipped.xs, clipped.ys, clipped.point_count);
    //G_fill_polygon(simple->xs, simple->ys, simple->point_count);
  }
}

void on_input(int poly_idx, Clip *clip) {
  static int current_poly_idx = 0;
  static int prev_poly_idx;

  prev_poly_idx = current_poly_idx;  
  current_poly_idx = poly_idx;

  G_rgb(0, 0, 0);
  G_clear();

  Poly *poly = &polys[current_poly_idx];
  if (current_poly_idx == prev_poly_idx) {
    rotate_poly(poly);
  }
  draw_poly(poly, clip);

  // Draw clip
  G_rgb(1, 0, 0);
  for (int point_idx = 0; point_idx < clip->point_count; point_idx++) {
    double x0 = clip->xs[point_idx];
    double y0 = clip->ys[point_idx];
    double xf = clip->xs[(point_idx + 1) % clip->point_count];
    double yf = clip->ys[(point_idx + 1) % clip->point_count];

    G_fill_rectangle(x0 - 2, y0 - 2, 4, 4);
    G_line(x0, y0, xf, yf);
  }
}

int main(int argc, char **argv) {
  if (argc == 1) {
    printf("Give me .xy files!!\n");
    exit(1);
  }

  printf("Press e to exit.\n");

  init_gfx(0);

  poly_count = argc - 1;

  for (int poly_idx = 0; poly_idx < poly_count; poly_idx++) {
    char *filename = argv[1 + poly_idx];
    load_poly(filename, poly_idx);
  }

  draw_click_bar();
  draw_bounding_box();
  Clip *clip = click_and_save_simple();

  const int numeral_one_key_code = 49;

  on_input(0, clip);
  while (1) {
    int got_key_code = G_wait_key();
    // press e for exit
    if (got_key_code == 101) {
      exit(0);
    }

    int poly_idx = got_key_code - numeral_one_key_code;
    if (0 <= poly_idx && poly_idx < poly_count) {
      on_input(poly_idx, clip);
    } else {
      printf("Requested polygon out of bounds...\n");
    }
  }
}
