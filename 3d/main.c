#include <FPT.h>

#include "matrix.h"
#include "poly.h"
#include "model.h"
#include "display.h"

Model *models[ENOUGH];
int model_count = 0;

void add_model(Model *model) {
  models[model_count] = model;
  model_count++;
}

void on_key(Model *model, const char key) {
  Point model_center;
  Model_center_M(&model_center, model);

  const double speed = 1.5;
  double mat_translate_backwards[4][4];
  Mat_translation_M(mat_translate_backwards, 0, 0, -speed);

  double mat_translate_forwards[4][4];
  Mat_translation_M(mat_translate_forwards, 0, 0, speed);

  double mat_translate_left[4][4];
  Mat_translation_M(mat_translate_left, -speed, 0, 0);

  double mat_translate_right[4][4];
  Mat_translation_M(mat_translate_right, speed, 0, 0);

  double mat_translate_up[4][4];
  Mat_translation_M(mat_translate_up, 0, speed, 0);

  double mat_translate_down[4][4];
  Mat_translation_M(mat_translate_down, 0, -speed, 0);

  double mat_translate_to_origin[4][4];
  Mat_translation_M(mat_translate_to_origin, -model_center.x, -model_center.y, -model_center.z);

  double mat_translate_from_origin[4][4];
  Mat_translation_M(mat_translate_from_origin, model_center.x, model_center.y, model_center.z);

#define make_rel_to_origin(name) \
  Mat_mult_M(name, name, mat_translate_to_origin); \
  Mat_mult_M(name, mat_translate_from_origin, name);

  const double scale_amt = 0.001;

  double mat_scale_up[4][4];
  Mat_scaling_M(mat_scale_up, 1 + scale_amt, 1 + scale_amt, 1 + scale_amt);
  make_rel_to_origin(mat_scale_up);

  double mat_scale_down[4][4];
  Mat_scaling_M(mat_scale_up, 1 - scale_amt, 1 - scale_amt, 1 - scale_amt);
  make_rel_to_origin(mat_scale_down);

  const double angle = M_PI / 16;

#define make_rot_with_sign(name, axis, sign) \
  double name[4][4]; \
  Mat_ ## axis ## _rotation_cs_M(name, cos(sign * angle), sin(sign * angle)); \
  make_rel_to_origin(name)

#define make_rot_mats(axis) \
  make_rot_with_sign(mat_rotate_ ## axis ## _positive, axis, +1); \
  make_rot_with_sign(mat_rotate_ ## axis ## _negative, axis, -1);

  make_rot_mats(x)
  make_rot_mats(y)
  make_rot_mats(z)

  switch(key) {
    case 'e': exit(0); break;

    case 's': Model_transform(model, mat_translate_forwards ); break;
    case 'w': Model_transform(model, mat_translate_backwards); break;
    case 'a': Model_transform(model, mat_translate_right    ); break;
    case 'd': Model_transform(model, mat_translate_left     ); break;
    case 'r': Model_transform(model, mat_translate_down     ); break;
    case 'f': Model_transform(model, mat_translate_up       ); break;

    case 'o': Model_transform(model, mat_rotate_x_positive  ); break;
    case 'p': Model_transform(model, mat_rotate_x_negative  ); break;

    case 'k': Model_transform(model, mat_rotate_y_positive  ); break;
    case 'l': Model_transform(model, mat_rotate_y_negative  ); break;

    case 'm': Model_transform(model, mat_rotate_z_positive  ); break;
    case ',': Model_transform(model, mat_rotate_z_negative  ); break;

    case '[': Model_transform(model, mat_scale_down         ); break;
    case ']': Model_transform(model, mat_scale_up           ); break;

    case '0': Model_transform(model, mat_translate_to_origin); break;

    case '=': half_angle += 0.01; break;
    case '-': half_angle -= 0.01; break;

    case '/': backface_elimination_sign *= -1;
  }
}

int main(const int argc, const char **argv) {
  if (argc == 1) {
    printf("Give my .xyz files!!!\n");
    exit(1);
  }

  printf("Controls:\n");
  printf("  e    - Exit program\n");
  printf("  0    - Move object to user\n");
  printf("  wasd - Strafe user along xz plane\n");
  printf("  rf   - Strafe user up and down\n");
  printf("  op   - Rotate object around x axis\n");
  printf("  kl   - Rotate object around y axis\n");
  printf("  m,   - Rotate object around z axis\n");
  printf("  []   - Scale object down and up\n");
  printf("  -+   - Adjust perspective\n");
  printf("  /    - Change backface elimination sign\n");

  G_init_graphics(screen_width, screen_height);

  for (int i = 1; i < argc; i++) {
    const char *filename = argv[i];
    Model *model = load_model(filename);
    add_model(model);
  }

  int model_idx;
  char key = '1';
  do {

    G_rgb(1, 1, 1);
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
    on_key(model, key);
    Model_display(model);

  } while ((key = G_wait_key()));
}
