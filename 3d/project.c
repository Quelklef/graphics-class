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
    printf("Give my .xyz files!!!");
    exit(1);
  }

  printf("Press e to exit.\n");

  G_init_graphics(400, 400);

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
      Model_display(models[model_idx]);
    } else {
      printf("Requested model out of bounds...\n");
    }
  }
}
