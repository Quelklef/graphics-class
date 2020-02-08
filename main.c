#include <FPT.h>
#include <math.h>

#include "matrix.h"
#include "polygon.h"
#include "polyhedron.h"
#include "display.h"
#include "observer.h"

#include "dyn.h"
DYN_INIT(PolyhedraList, Polyhedron*)

PolyhedraList *polyhedra;
Polyhedron *light_source = NULL;

void PolyhedraList_destroy(PolyhedraList *polyhedra) {
  for (int polyhedron_idx = 0; polyhedron_idx < polyhedra->length; polyhedron_idx++) {
    Polyhedron_destroy(PolyhedraList_get(polyhedra, polyhedron_idx));
  }
  Dyn_destroy(polyhedra);
}


float clamp(float x, float lo, float hi) {
  if (x < lo) return lo;
  if (x > hi) return hi;
  return x;
}


// Currently selected parameter
static const int param_HALF_ANGLE     = 1;
static const int param_AMBIENT        = 2;
static const int param_DIFFUSE_MAX    = 3;
static const int param_SPECULAR_POWER = 4;
static const int param_HITHER         = 5;
static const int param_YON            = 6;
static int current_parameter = 0;

int display_overlay = 0;

void on_key(Polyhedron *polyhedron, const char key) {
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

  float polyhedron_x_size, polyhedron_y_size, polyhedron_z_size;
  Polyhedron_size_M(&polyhedron_x_size, &polyhedron_y_size, &polyhedron_z_size, polyhedron);

  const _Mat translate_backwards_rel = Mat_translate(0, 0, -rel_speed * polyhedron_z_size);
  const _Mat translate_forwards_rel  = Mat_translate(0, 0, +rel_speed * polyhedron_z_size);
  const _Mat translate_left_rel      = Mat_translate(-rel_speed * polyhedron_x_size, 0, 0);
  const _Mat translate_right_rel     = Mat_translate(+rel_speed * polyhedron_x_size, 0, 0);
  const _Mat translate_up_rel        = Mat_translate(0, +rel_speed * polyhedron_y_size, 0);
  const _Mat translate_down_rel      = Mat_translate(0, -rel_speed * polyhedron_y_size, 0);

  const v3 polyhedron_center = Polyhedron_center(polyhedron);

  const _Mat translate_to_origin   = Mat_translate_v(-polyhedron_center);
  const _Mat translate_from_origin = Mat_translate_v(+polyhedron_center);

#define make_rel_to_origin(name) \
  Mat_mult_M(name, name, translate_to_origin); \
  Mat_mult_M(name, translate_from_origin, name);

  const float scale_amt = 0.01;

  _Mat scale_up = Mat_dilate(1 + scale_amt, 1 + scale_amt, 1 + scale_amt);
  make_rel_to_origin(scale_up);

  _Mat scale_down = Mat_dilate(1 - scale_amt, 1 - scale_amt, 1 - scale_amt);
  make_rel_to_origin(scale_down);

  const float angle = M_PI / 16;

  _Mat rotate_x_positive = Mat_x_rot(+angle);
  make_rel_to_origin(rotate_x_positive);
  _Mat rotate_x_negative = Mat_x_rot(-angle);
  make_rel_to_origin(rotate_x_negative);
  _Mat rotate_y_positive = Mat_y_rot(+angle);
  make_rel_to_origin(rotate_y_positive);
  _Mat rotate_y_negative = Mat_y_rot(-angle);
  make_rel_to_origin(rotate_y_negative);
  _Mat rotate_z_positive = Mat_z_rot(+angle);
  make_rel_to_origin(rotate_z_positive);
  _Mat rotate_z_negative = Mat_z_rot(-angle);
  make_rel_to_origin(rotate_z_negative);

  switch(key) {
    case 'w': Polyhedron_transform(polyhedron, translate_forwards_rel ); break;
    case 's': Polyhedron_transform(polyhedron, translate_backwards_rel); break;
    case 'd': Polyhedron_transform(polyhedron, translate_right_rel    ); break;
    case 'a': Polyhedron_transform(polyhedron, translate_left_rel     ); break;
    case 'f': Polyhedron_transform(polyhedron, translate_down_rel     ); break;
    case 'r': Polyhedron_transform(polyhedron, translate_up_rel       ); break;

    case 'W': Polyhedron_transform(polyhedron, translate_forwards_abs ); break;
    case 'S': Polyhedron_transform(polyhedron, translate_backwards_abs); break;
    case 'D': Polyhedron_transform(polyhedron, translate_right_abs    ); break;
    case 'A': Polyhedron_transform(polyhedron, translate_left_abs     ); break;
    case 'F': Polyhedron_transform(polyhedron, translate_down_abs     ); break;
    case 'R': Polyhedron_transform(polyhedron, translate_up_abs       ); break;

    case 'o': Polyhedron_transform(polyhedron, rotate_x_positive  ); break;
    case 'p': Polyhedron_transform(polyhedron, rotate_x_negative  ); break;

    case 'k': Polyhedron_transform(polyhedron, rotate_y_positive  ); break;
    case 'l': Polyhedron_transform(polyhedron, rotate_y_negative  ); break;

    case 'm': Polyhedron_transform(polyhedron, rotate_z_positive  ); break;
    case ',': Polyhedron_transform(polyhedron, rotate_z_negative  ); break;

    case '[': Polyhedron_transform(polyhedron, scale_down         ); break;
    case ']': Polyhedron_transform(polyhedron, scale_up           ); break;

    case 'O': Polyhedron_transform(polyhedron, translate_to_origin); break;
    case 'L': printf("Object at (%f, %f, %f)\n", polyhedron_center[0], polyhedron_center[1], polyhedron_center[2]); break;

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

  // Draw indices for each polyhedron
  for (int polyhedron_i = 0; polyhedron_i < polyhedra->length; polyhedron_i++) {
    const Polyhedron *polyhedron = PolyhedraList_get(polyhedra, polyhedron_i);

    float min_x, max_x, min_y, max_y, min_z, max_z;
    Polyhedron_bounds_M(&min_x, &max_x, &min_y, &max_y, &min_z, &max_z, polyhedron);

    const v3 center = Polyhedron_center(polyhedron);
    const v2 pixel = pixel_coords(center);

    // TODO: better way to show off-bounds objects
    pixel[0] = clamp(pixel[0], 0, SCREEN_WIDTH);
    pixel[1] = clamp(pixel[1], 0, SCREEN_HEIGHT);
    draw_stringf(pixel[0], pixel[1], "%d        ", polyhedron_i);
  }
}

void show_help() {
  printf("Some controls:\n");
  printf("  e    - Exit program\n");
  printf("  O    - Move object to user\n");
  printf("  L    - Print object location\n");
  printf("  []   - Scale selected object down and up\n");
  printf("  0-9  - Switch polyhedra\n");
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
  printf("  $    - Enable/disable light polyhedron\n");
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
    Polyhedron *polyhedron = load_polyhedron(filename);
    PolyhedraList_append(polyhedra, polyhedron);
  }
}

void event_loop() {
  Polyhedron *focused_polyhedron = NULL;

  char key = '1';
  do {

    // Clear screen
    G_rgb(0, 0, 0);
    G_clear();
    G_rgb(1, 0, 0);
    draw_box();

    const int polyhedron_idx = key - '0';
    if (0 <= polyhedron_idx && polyhedron_idx < polyhedra->length) {
      focused_polyhedron = PolyhedraList_get(polyhedra, polyhedron_idx);
    }

    on_key(focused_polyhedron, key);

    display_polyhedra(polyhedra->items, polyhedra->length, focused_polyhedron, light_source);

    display_state();

  } while ((key = G_wait_key()) != 'e');
}








v3 sphere_f(float t, float s) {
  const float x = cos(t) * sin(s);
  const float y = cos(s);
  const float z = sin(t) * sin(s);

  return (v3) { x, y, z };
}

v3 sphere_g(float t, float s) {
  const float r = sqrt(1 - s * s);

  const float x = r * cos(t);
  const float y = s;
  const float z = r * sin(t);

  return (v3) { x, y, z };
}

v3 vase_f(float t, float s) {
  const float r = sqrt(1 + s * s);

  const float x = r * cos(t);
  const float y = s;
  const float z = r * sin(t);

  return (v3) { x, y, z };
}

void prepare_3d_lab() {

  Polyhedron *sphere1 = Polyhedron_from_parametric(
    sphere_f,
    0, 2 * M_PI, 50,
    1,
    0, 2 * M_PI, 50,
    1
  );

  nicely_place_polyhedron(sphere1);
  PolyhedraList_append(polyhedra, sphere1);

  Polyhedron *sphere2 = Polyhedron_from_parametric(
    sphere_g,
    0, 2 * M_PI, 25,
    1,
    -1, 1, 25,
    0
  );

  nicely_place_polyhedron(sphere2);
  PolyhedraList_append(polyhedra, sphere2);

  Polyhedron *vase = Polyhedron_from_parametric(
    vase_f,
    0, 2 * M_PI, 25,
    1,
    -2, 2, 25,
    0
  );

  nicely_place_polyhedron(vase);
  PolyhedraList_append(polyhedra, vase);

}


DYN_INIT(LongList, long);

int main(const int argc, const char **argv) {

  // == Setup == //

  polyhedra = PolyhedraList_new(1);

  G_init_graphics(SCREEN_WIDTH, SCREEN_HEIGHT);

  //show_help();

  light_source = make_small_polyhedron();
  PolyhedraList_append(polyhedra, light_source);

  //load_files(&argv[1], argc - 1);

  // == Main == //

  prepare_3d_lab();
  event_loop();

  // == Teardown == //

  PolyhedraList_destroy(polyhedra);

  G_close();

}
