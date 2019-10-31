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

  const int screen_width = 800;
  const int screen_height = 400;
  G_init_graphics(screen_width, screen_height);

  for (int i = 1; i < argc; i++) {
    const char *filename = argv[i];
    add_model(load_model(filename));
  }

  double mat_translate_backwards[4][4];
  M3d_make_translation(mat_translate_backwards, -1, 0, 0);

  double mat_translate_forwards[4][4];
  M3d_make_translation(mat_translate_forwards, 1, 0, 0);

  double mat_translate_left[4][4];
  M3d_make_translation(mat_translate_left, 0, 0, -1);

  double mat_translate_right[4][4];
  M3d_make_translation(mat_translate_right, 0, 0, 1);

  double mat_translate_up[4][4];
  M3d_make_translation(mat_translate_up, 0, 1, 0);

  double mat_translate_down[4][4];
  M3d_make_translation(mat_translate_down, 0, -1, 0);

  int model_idx;
  char key = '1';
  do {
    G_rgb(0, 0, 0);
    G_clear();
    G_rgb(1, 0, 0);
    G_fill_rectangle(0               , 0                , screen_width, 1            );
    G_fill_rectangle(screen_width - 1, 0                , 1           , screen_height);
    G_fill_rectangle(0               , screen_height - 1, screen_width, 1            );
    G_fill_rectangle(0               , 0                , 1           , screen_height);

    if (0 <= key - '1' && key - '1' < model_count) {
      model_idx = key - '1';
    }

    Model *model = models[model_idx];

         if (key == 'e') exit(0);
    else if (key == 's') Model_transform(model, mat_translate_forwards);
    else if (key == 'w') Model_transform(model, mat_translate_backwards);
    else if (key == 'a') Model_transform(model, mat_translate_right);
    else if (key == 'd') Model_transform(model, mat_translate_left);
    else if (key == 'r') Model_transform(model, mat_translate_down);
    else if (key == 'f') Model_transform(model, mat_translate_up);

    Model_display(model, screen_width, screen_height);

  } while (key = G_wait_key());
}
