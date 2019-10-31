#include <FPT.h>

#include "poly.h"

Model *models[ENOUGH];
int model_count;

void add_model(Model *model) {
  models[model_count] = model;
  model_count++;
}

int main(const int argc, const char **argv) {
  if (argc == 1) {
    printf("Give my .xyz files!!!\n");
    exit(1);
  }

  printf("Press e to exit.\n");

  const int screen_width = 400;
  const int screen_height = 400;
  G_init_graphics(screen_width, screen_height);

  G_rgb(1, 0, 0);
  G_fill_rectangle(0               , 0                , screen_width, 1            );
  G_fill_rectangle(screen_width - 1, 0                , 1           , screen_height);
  G_fill_rectangle(0               , screen_height - 1, screen_width, 1            );
  G_fill_rectangle(0               , 0                , 1           , screen_height);

  for (int i = 1; i < argc; i++) {
    const char *filename = argv[i];
    add_model(load_model(filename));
  }

  while (1) {
    int key_code = G_wait_key();

    // Press e to exit
    if (key_code == 101) {
      exit(0);
    }

    const int numeral_one_key_code = 49;
    int model_idx = key_code - numeral_one_key_code;
    if (0 <= model_idx && model_idx < model_count) {
      Model_display(models[model_idx], screen_width, screen_height);
    } else {
      printf("Requested model out of bounds...\n");
    }
  }
}
