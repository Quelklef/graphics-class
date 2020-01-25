#include <FPT.h>
#include <math.h>

#include "matrix.h"
#include "poly.h"
#include "model.h"
#include "display.h"
#include "observer.h"

Model *models[ENOUGH];
int model_count = 0;

void add_model(Model *model) {
  models[model_count] = model;
  model_count++;
}

Model *light_source = NULL;


// Currently selected parameter
static const int param_HALF_ANGLE     = 1;
static const int param_AMBIENT        = 2;
static const int param_DIFFUSE_MAX    = 3;
static const int param_SPECULAR_POWER = 4;
static const int param_HITHER         = 5;
static const int param_YON            = 6;
static int current_parameter = 0;

int display_overlay = 0;

void on_key(Model *model, const char key) {
  // Two ways to move: with an absolute distance
  static const float abs_speed = 0.5;
  // or moving as a percentage of the object's size
  static const float rel_speed = 0.1;

  // Absolute transformations

  static const _Mat translate_backwards_abs = Mat_translate(0, 0, -abs_speed);
  static const _Mat translate_forwards_abs  = Mat_translate(0, 0, +abs_speed);
  static const _Mat translate_left_abs      = Mat_translate(-abs_speed, 0, 0);
  static const _Mat translate_right_abs     = Mat_translate(+abs_speed, 0, 0);
  static const _Mat translate_up_abs        = Mat_translate(0, +abs_speed, 0);
  static const _Mat translate_down_abs      = Mat_translate(0, -abs_speed, 0);

  // Relative transformations

  float model_x_size, model_y_size, model_z_size;
  Model_size_M(&model_x_size, &model_y_size, &model_z_size, model);

  const _Mat translate_backwards_rel = Mat_translate(0, 0, -rel_speed * model_z_size);
  const _Mat translate_forwards_rel  = Mat_translate(0, 0, +rel_speed * model_z_size);
  const _Mat translate_left_rel      = Mat_translate(-rel_speed * model_x_size, 0, 0);
  const _Mat translate_right_rel     = Mat_translate(+rel_speed * model_x_size, 0, 0);
  const _Mat translate_up_rel        = Mat_translate(0, +rel_speed * model_y_size, 0);
  const _Mat translate_down_rel      = Mat_translate(0, -rel_speed * model_y_size, 0);

  const v3 model_center = Model_center(model);

  const _Mat translate_to_origin   = Mat_translate_v(-model_center);
  const _Mat translate_from_origin = Mat_translate_v(+model_center);

#define make_rel_to_origin(name) \
  Mat_mult_M(name, name, translate_to_origin); \
  Mat_mult_M(name, translate_from_origin, name);

  const float scale_amt = 0.01;

  _Mat scale_up = Mat_dilate(1 + scale_amt, 1 + scale_amt, 1 + scale_amt);
  make_rel_to_origin(scale_up);

  _Mat scale_down = Mat_dilate(1 - scale_amt, 1 - scale_amt, 1 - scale_amt);
  make_rel_to_origin(scale_down);

  const float angle = M_PI / 16;

  const _Mat rotate_x_positive = Mat_x_rot(+angle);
  const _Mat rotate_x_negative = Mat_x_rot(-angle);
  const _Mat rotate_y_positive = Mat_y_rot(+angle);
  const _Mat rotate_y_negative = Mat_y_rot(-angle);
  const _Mat rotate_z_positive = Mat_z_rot(+angle);
  const _Mat rotate_z_negative = Mat_z_rot(-angle);

  switch(key) {
    case 'w': Model_transform(model, translate_forwards_rel ); break;
    case 's': Model_transform(model, translate_backwards_rel); break;
    case 'd': Model_transform(model, translate_right_rel    ); break;
    case 'a': Model_transform(model, translate_left_rel     ); break;
    case 'f': Model_transform(model, translate_down_rel     ); break;
    case 'r': Model_transform(model, translate_up_rel       ); break;

    case 'W': Model_transform(model, translate_forwards_abs ); break;
    case 'S': Model_transform(model, translate_backwards_abs); break;
    case 'D': Model_transform(model, translate_right_abs    ); break;
    case 'A': Model_transform(model, translate_left_abs     ); break;
    case 'F': Model_transform(model, translate_down_abs     ); break;
    case 'R': Model_transform(model, translate_up_abs       ); break;

    case 'o': Model_transform(model, rotate_x_positive  ); break;
    case 'p': Model_transform(model, rotate_x_negative  ); break;

    case 'k': Model_transform(model, rotate_y_positive  ); break;
    case 'l': Model_transform(model, rotate_y_negative  ); break;

    case 'm': Model_transform(model, rotate_z_positive  ); break;
    case ',': Model_transform(model, rotate_z_negative  ); break;

    case '[': Model_transform(model, scale_down         ); break;
    case ']': Model_transform(model, scale_up           ); break;

    case 'O': Model_transform(model, translate_to_origin); break;

    case '!': DO_WIREFRAME            = 1 - DO_WIREFRAME;            break;
    case '@': DO_BACKFACE_ELIMINATION = 1 - DO_BACKFACE_ELIMINATION; break;
    case '#': DO_POLY_FILL            = 1 - DO_POLY_FILL;            break;
    case '$': DO_LIGHT_MODEL          = 1 - DO_LIGHT_MODEL;          break;
    case '%': DO_HALO                 = 1 - DO_HALO;                 break;
    case '^': DO_CLIPPING             = 1 - DO_CLIPPING;             break;

    case '/': BACKFACE_ELIMINATION_SIGN *= -1; break;

    case '`': display_overlay = 1 - display_overlay; break;
  }

  // Parameter adjustment

// https://stackoverflow.com/a/1508589/4608364
#define reset() printf("\r"); fflush(stdout);
#define printr(...) printf(__VA_ARGS__); printf("                "); reset();

  switch(key) {
    case 'H':
      current_parameter = param_HALF_ANGLE;
      printr("Selected param: HALF_ANGLE (%lf)", HALF_ANGLE);
      break;
    case 'B':
      current_parameter = param_AMBIENT;
      printr("Selected param: AMBIENT (%lf)", AMBIENT);
      break;
    case 'M':
      current_parameter = param_DIFFUSE_MAX;
      printr("Selected param: DIFFUSE_MAX (%lf)", DIFFUSE_MAX);
      break;
    case 'P':
      current_parameter = param_SPECULAR_POWER;
      printr("Selected param: SPECULAR_POWER (%d)", SPECULAR_POWER);
      break;
    case 'T':
      current_parameter = param_HITHER;
      printr("Selected param: HITHER (%lf)", HITHER);
      break;
    case 'Y':
      current_parameter = param_YON;
      printr("Selected param: YON (%lf)", YON);
      break;
  }

  if (key == '=' || key == '-' || key == '+' || key == '_') {
    const int sign = (key == '=' || key == '+') ? 1 : -1;
    const int is_fast = key == '+' || key == '_';

    if (current_parameter == param_HALF_ANGLE) {
      HALF_ANGLE += sign * (is_fast ? 0.50 : 0.01);
      printr("HALF_ANGLE = %lf", HALF_ANGLE);
    } else if (current_parameter == param_AMBIENT) {
      AMBIENT += sign * (is_fast ? 0.5 : 0.05);
      printr("AMBIENT = %lf", AMBIENT);
    } else if (current_parameter == param_DIFFUSE_MAX) {
      DIFFUSE_MAX += sign * (is_fast ? 0.5 : 0.05);
      printr("DIFFUSE_MAX = %lf", DIFFUSE_MAX);
    } else if (current_parameter == param_SPECULAR_POWER) {
      SPECULAR_POWER += sign * (is_fast ? 25 : 1);
      printr("SPECULAR_POWER = %d", SPECULAR_POWER);
    } else if (current_parameter == param_HITHER) {
      HITHER += sign * (is_fast ? 1.5 : 0.1);
      if (HITHER > YON) HITHER = YON;  // Don't let HITHER and YON 'cross'
      printr("HITHER = %lf", HITHER);
    } else if (current_parameter == param_YON) {
      YON += sign * (is_fast ? 1.5 : 0.1);
      if (YON < HITHER) YON = HITHER;  // Don't let HITHER and YON 'cross'
      printr("YON = %lf", YON);
    }
  }

}

// Would prefer to do this with functions than macros,
// but can't pass varargs from one variadic function to another
#define draw_stringf(llx, lly, fstring, ...) \
  /* Draw a format string. The length of the string after formatting */ \
  /* is bounded by the length of the format string */ \
  { \
    const int len = strlen(fstring); \
    char buffer[len]; \
    buffer[0] = '\0'; \
    snprintf(buffer, len, fstring, ##__VA_ARGS__); \
    G_draw_string(buffer,  llx, lly); \
  }

#define draw_param(llx, lly, key, code, fstring, ...) \
  { \
    char *left; \
    char *right; \
    if (current_parameter == code) { \
      left = "["; \
      right = "] "; \
    } else { \
      left = "("; \
      right = ") "; \
    } \
    char catted[strlen(left) + strlen(key) + strlen(right) + strlen(fstring)]; \
    catted[0] = '\0'; \
    strcat(catted, left); \
    strcat(catted, key); \
    strcat(catted, right); \
    strcat(catted, fstring); \
    draw_stringf(llx, lly, catted, ##__VA_ARGS__); \
  }


void display_state() {
  G_rgb(1, 1, 1);
  draw_stringf(20, SCREEN_HEIGHT -  40, "(`) Overlay: %d", display_overlay);
  if (!display_overlay) return;

  draw_stringf(20, SCREEN_HEIGHT -  80, "(!) Wframe: %d", DO_WIREFRAME);
  draw_stringf(20, SCREEN_HEIGHT - 100, "(@) BFElim: %d", DO_BACKFACE_ELIMINATION);
  draw_stringf(20, SCREEN_HEIGHT - 120, "(/)   Sign: %d ", BACKFACE_ELIMINATION_SIGN);
  draw_stringf(20, SCREEN_HEIGHT - 140, "(#) Fill  : %d", DO_POLY_FILL);
  draw_stringf(20, SCREEN_HEIGHT - 160, "($) Light : %d", DO_LIGHT_MODEL);
  draw_stringf(20, SCREEN_HEIGHT - 180, "(%%) Halos : %d", DO_HALO);
  draw_stringf(20, SCREEN_HEIGHT - 200, "(^) Clip  : %d", DO_CLIPPING);

  draw_stringf(20, 140, "Use +/- to adjust ");
  draw_param(20, 120, "H", param_HALF_ANGLE    , "HAngle : %lf      ", HALF_ANGLE);
  draw_param(20, 100, "B", param_AMBIENT       , "Ambient: %lf      ", AMBIENT);
  draw_param(20,  80, "M", param_DIFFUSE_MAX   , "DifMax : %lf      ", DIFFUSE_MAX);
  draw_param(20,  60, "P", param_SPECULAR_POWER, "SpecPow: %d      ", SPECULAR_POWER);
  draw_param(20,  40, "T", param_HITHER        , "Hither : %lf      ", HITHER);
  draw_param(20,  20, "Y", param_YON           , "Yon    : %lf      ", YON);

  // Draw indices for each model
  for (int model_i = 0; model_i < model_count; model_i++) {
    const Model *model = models[model_i];

    float min_x, max_x, min_y, max_y, min_z, max_z;
    Model_bounds_M(&min_x, &max_x, &min_y, &max_y, &min_z, &max_z, model);
    // Skip models that are entirely clipped
    if ( max_z <= HITHER || min_z >= YON
         || HITHER >= YON ) {  // If HITHER >= YON then all models are entirely clipped
      continue;
    }

    const v3 center = Model_center(model);
    float px, py;
    pixel_coords_M(&px, &py, center);

    draw_stringf(px, py, "%d        ", model_i);
  }
}

void show_help() {
  printf("Some controls:\n");
  printf("  e    - Exit program\n");
  printf("  O    - Move object to user\n");
  printf("  []   - Scale selected object down and up\n");
  printf("  0-9  - Switch models\n");
  printf("\n");
  printf("Strafing (& shift-):\n");
  printf("  wasd - Strafe selected object along xz plane\n");
  printf("  rf   - Strafe selected object up and down\n");
  printf("  op   - Rotate selected object around x axis\n");
  printf("  kl   - Rotate selected object around y axis\n");
  printf("  m,   - Rotate selected object around z axis\n");
  printf("\n");
  printf("Binary parameters:\n");
  printf("  !    - Enable/disable wireframe\n");
  printf("  @    - Enable/disable backface elimination\n");
  printf("  #    - Enable/disable polygon filling\n");
  printf("  $    - Enable/disable light model\n");
  printf("  %%    - Enable/disable halos\n");
  printf("  /    - Change backface elimination sign\n");
  printf("  ^    - Enable/disable clipping\n");
  printf("\n");
  printf("Scalar parameters:\n");
  printf("  -+   - Adjust parameter (use shift for fast)\n");
  printf("  H    - Select parameter HALF_ANGLE\n");
  printf("  B    - Select parameter AMBIENT\n");
  printf("  M    - Select parameter DIFFUSE_MAX\n");
  printf("  P    - Select parameter SPECULAR_POWER\n");
  printf("  T    - Select parameter HITHER\n");
  printf("  Y    - Select parameter YON\n");
  printf("\n");
}

void draw_box() {
  G_fill_rectangle(0               , 0                , SCREEN_WIDTH, 1            );
  G_fill_rectangle(SCREEN_WIDTH - 1, 0                , 1           , SCREEN_HEIGHT);
  G_fill_rectangle(0               , SCREEN_HEIGHT - 1, SCREEN_WIDTH, 1            );
  G_fill_rectangle(0               , 0                , 1           , SCREEN_HEIGHT);
}

void load_files(const char **filenames, const int file_count) {
  for (int i = 0; i < file_count; i++) {
    const char *filename = filenames[i];
    Model *model = load_model(filename);
    add_model(model);
  }
}

void event_loop() {
  Model *focused_model = NULL;

  char key = '1';
  do {

    // Clear screen
    G_rgb(0, 0, 0);
    G_clear();
    G_rgb(1, 0, 0);
    draw_box();

    const int model_idx = key - '0';
    if (0 <= model_idx && model_idx < model_count) {
      focused_model = models[model_idx];
    }

    on_key(focused_model, key);

    display_models(models, model_count, focused_model, light_source);

    display_state();

  } while ((key = G_wait_key()) != 'e');
}



void place_at_origin(Model *model) {
  const v3 center = Model_center(model);
  Model_transform(model, (_Mat) Mat_translate_v(-center));
}

void setup_scene() {

  // Place sphere

  Model *sphere = load_model("xyz/sphere.xyz");
  place_at_origin(sphere);

  Model_transform(sphere, (_Mat) Mat_dilate(0.25, 0.25, 0.25));

  add_model(sphere);

  // Place cylinders

  Model *cyl = load_model("xyz/cylinder.xyz");
  place_at_origin(cyl);

  Model_transform(cyl, (_Mat) Mat_translate(1, 0, 0));
  Model_transform(cyl, (_Mat) Mat_dilate(2, 1, 1));

  // x cylinder

  Model *x_cyl = Model_clone(cyl);
  add_model(x_cyl);

  // y cylinder

  Model *y_cyl = Model_clone(cyl);
  // Rotate to be on y-axis
  Model_transform(y_cyl, (_Mat) Mat_z_rot(M_PI / 2));
  add_model(y_cyl);


  // z cylinder

  Model *z_cyl = Model_clone(cyl);
  // Rotate to be on z-axis
  Model_transform(z_cyl, (_Mat) Mat_y_rot(-M_PI / 2));
  add_model(z_cyl);

  // cleanup
  Model_destroy(cyl);

}

void do_animation() {
  setup_scene();

  const int frame_count = 100;

  while (1) {

    for (int frame_idx = 0; frame_idx <= frame_count; frame_idx++) {

      // Clear screen
      G_rgb(0, 0, 0);
      G_clear();
      G_rgb(1, 0, 0);
      draw_box();

      const float t = (float) frame_idx / frame_count;

      eye[0] = 15 * cos(2 * M_PI * t),
      eye[1] = 6 * t,
      eye[2] = 7 * sin(2 * M_PI * t),

      center_of_interest[0] = 0;
      center_of_interest[1] = 0;
      center_of_interest[2] = 0;

      up_point[0] = eye[0];
      up_point[1] = eye[1] + 1;
      up_point[2] = eye[2];

      display_models(models, model_count, NULL, light_source);


      const char key = G_wait_key();
      if (key == 'e') return;

    }

  }

}

int main(const int argc, const char **argv) {

  // Setup

  G_init_graphics(SCREEN_WIDTH, SCREEN_HEIGHT);

  //show_help();

  light_source = make_small_model();
  add_model(light_source);

  // Main

  //load_files(&argv[1], argc - 1);
  //event_loop();
  do_animation(); 

  // Teardown

  for (int model_idx = 0; model_idx < model_count; model_idx++) {
    Model_destroy(models[model_idx]);
  }

  G_close();

}
