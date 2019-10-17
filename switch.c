#include <stdio.h>
#include <FPT.h>
#include <float.h>

#include "poly.h"

void display_poly(Poly *poly) {
  G_rgb(0, 0, 0);
  G_clear();

  G_rgb(1, 0, 0);
  G_fill_rectangle(0, 0, screen_width, 1);
  G_fill_rectangle(screen_width - 1, 0, 1, screen_height);
  G_fill_rectangle(0, screen_height - 1, screen_width, 1);
  G_fill_rectangle(0, 0, 1, screen_height);

  for (int simple_idx = 0; simple_idx < poly->simple_count; simple_idx++) {
    SimplePoly *simple = poly->simples[simple_idx];

    G_rgb(simple->red, simple->blue, simple->green);
    G_fill_polygon(simple->xs, simple->ys, simple->point_count);
  }
}

void on_input(int poly_idx) {
  static int current_poly_idx = 0;
  static int prev_poly_idx;

  prev_poly_idx = current_poly_idx;  
  current_poly_idx = poly_idx;

  Poly *poly = &polys[current_poly_idx];
  if (current_poly_idx == prev_poly_idx) {
    rotate_poly(poly);
  }
  display_poly(poly);
}

int main(int argc, char **argv) {
  if (argc == 1) {
    printf("Give me .xy files!!\n");
    exit(1);
  }

  G_init_graphics(screen_width, screen_height);

  poly_count = argc - 1;

  for (int poly_idx = 0; poly_idx < poly_count; poly_idx++) {
    char *filename = argv[1 + poly_idx];
    load_poly(filename, poly_idx);
  }

  const int numeral_one_key_code = 49;


  on_input(0);
  while (1) {
    int got_key_code = G_wait_key();
    // press e for exit
    if (got_key_code == 101) {
      exit(0);
    }

    int poly_idx = got_key_code - numeral_one_key_code;
    if (0 <= poly_idx && poly_idx < poly_count) {
      on_input(poly_idx);
    } else {
      printf("Requested polygon out of bounds...\n");
    }
  }
}
