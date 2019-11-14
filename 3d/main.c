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
  static const double speed = 0.5;
  static const double fast_speed = 7.5;

  // Absolute transformations

  static int already_init = 0;
  static _Mat translate_backwards;
  static _Mat translate_forwards;
  static _Mat translate_left;
  static _Mat translate_right;
  static _Mat translate_up;
  static _Mat translate_down;

  static _Mat translate_backwards_fast;
  static _Mat translate_forwards_fast;
  static _Mat translate_left_fast;
  static _Mat translate_right_fast;
  static _Mat translate_up_fast;
  static _Mat translate_down_fast;

  if (!already_init) {
    Mat_translation_M(translate_backwards, 0     , 0     , -speed);
    Mat_translation_M(translate_forwards , 0     , 0     , speed );
    Mat_translation_M(translate_left     , -speed, 0     , 0     );
    Mat_translation_M(translate_right    , +speed, 0     , 0     );
    Mat_translation_M(translate_up       , 0     , +speed, 0     );
    Mat_translation_M(translate_down     , 0     , -speed, 0     );

    Mat_translation_M(translate_backwards_fast, 0          , 0          , -fast_speed);
    Mat_translation_M(translate_forwards_fast , 0          , 0          , fast_speed );
    Mat_translation_M(translate_left_fast     , -fast_speed, 0          , 0          );
    Mat_translation_M(translate_right_fast    , +fast_speed, 0          , 0          );
    Mat_translation_M(translate_up_fast       , 0          , +fast_speed, 0          );
    Mat_translation_M(translate_down_fast     , 0          , -fast_speed, 0          );
  }
  already_init = 1;

  // Relative transformations

  Point model_center;
  Model_center_M(&model_center, model);

  _Mat translate_to_origin;
  _Mat translate_from_origin;

  Mat_translation_M(translate_to_origin, -model_center.x, -model_center.y, -model_center.z);
  Mat_translation_M(translate_from_origin, model_center.x, model_center.y, model_center.z);

#define make_rel_to_origin(name) \
  Mat_mult_M(name, name, translate_to_origin); \
  Mat_mult_M(name, translate_from_origin, name);

  const double scale_amt = 0.01;

  _Mat scale_up;
  Mat_scaling_M(scale_up, 1 + scale_amt, 1 + scale_amt, 1 + scale_amt);
  make_rel_to_origin(scale_up);

  _Mat scale_down;
  Mat_scaling_M(scale_up, 1 - scale_amt, 1 - scale_amt, 1 - scale_amt);
  make_rel_to_origin(scale_down);

  const double angle = M_PI / 16;

#define make_rot_with_sign(name, axis, sign) \
  _Mat name; \
  Mat_ ## axis ## _rotation_cs_M(name, cos(sign * angle), sin(sign * angle)); \
  make_rel_to_origin(name)

#define make_rot_mats(axis) \
  make_rot_with_sign(rotate_ ## axis ## _positive, axis, +1); \
  make_rot_with_sign(rotate_ ## axis ## _negative, axis, -1);

  make_rot_mats(x)
  make_rot_mats(y)
  make_rot_mats(z)

  switch(key) {
    case 'e': exit(0); break;

    case 'w': Model_transform(model, translate_forwards ); break;
    case 's': Model_transform(model, translate_backwards); break;
    case 'd': Model_transform(model, translate_right    ); break;
    case 'a': Model_transform(model, translate_left     ); break;
    case 'f': Model_transform(model, translate_down     ); break;
    case 'r': Model_transform(model, translate_up       ); break;

    case 'W': Model_transform(model, translate_forwards_fast ); break;
    case 'S': Model_transform(model, translate_backwards_fast); break;
    case 'D': Model_transform(model, translate_right_fast    ); break;
    case 'A': Model_transform(model, translate_left_fast     ); break;
    case 'F': Model_transform(model, translate_down_fast     ); break;
    case 'R': Model_transform(model, translate_up_fast       ); break;

    case 'o': Model_transform(model, rotate_x_positive  ); break;
    case 'p': Model_transform(model, rotate_x_negative  ); break;

    case 'k': Model_transform(model, rotate_y_positive  ); break;
    case 'l': Model_transform(model, rotate_y_negative  ); break;

    case 'm': Model_transform(model, rotate_z_positive  ); break;
    case ',': Model_transform(model, rotate_z_negative  ); break;

    case '[': Model_transform(model, scale_down         ); break;
    case ']': Model_transform(model, scale_up           ); break;

    case '0': Model_transform(model, translate_to_origin); break;

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
  printf("  wasd - Strafe selected object along xz plane\n");
  printf("  rf   - Strafe selected object up and down\n");
  printf("  op   - Rotate selected object around x axis\n");
  printf("  kl   - Rotate selected object around y axis\n");
  printf("  m,   - Rotate selected object around z axis\n");
  printf("  []   - Scale selected object down and up\n");
  printf("  -+   - Adjust perspective\n");
  printf("  /    - Change backface elimination sign\n");

  G_init_graphics(screen_width, screen_height);

  for (int i = 1; i < argc; i++) {
    const char *filename = argv[i];
    Model *model = load_model(filename);
    add_model(model);
  }

  Model *focused_model = NULL;
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
      focused_model = models[key - '1'];
    }

    if (focused_model != NULL) {
      on_key(focused_model, key);
    }

    display_models(models, model_count, focused_model);

  } while ((key = G_wait_key()));
}
