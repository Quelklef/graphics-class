#include <math.h>

#include <libgfx.h>

#include "globals.c"
#include "matrix.c"
#include "shapes/figure.c"
#include "shapes/v2.c"
#include "shapes/instances.c"
#include "rendering/draw.c"
#include "rendering/render.c"

#include "util/dyn.c"
DYN_INIT(FigureList, Figure*)

FigureList *figures;
Figure *light_source = NULL;

void FigureList_destroy(FigureList *figures) {
  for (int polyhedron_idx = 0; polyhedron_idx < figures->length; polyhedron_idx++) {
    Figure_destroy(FigureList_get(figures, polyhedron_idx));
  }
  Dyn_destroy(figures);
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

int render_overlay = 0;

void on_key(Figure *figure, const char key) {

  // TODO: Following quickfail is only relelvant if the key
  //       that's pressed transforms a figure.
  if (figure == NULL) return;

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

  v3 sizes = Figure_size(figure);

  const _Mat translate_backwards_rel = Mat_translate(0, 0, -rel_speed * sizes[0]);
  const _Mat translate_forwards_rel  = Mat_translate(0, 0, +rel_speed * sizes[0]);
  const _Mat translate_left_rel      = Mat_translate(-rel_speed * sizes[1], 0, 0);
  const _Mat translate_right_rel     = Mat_translate(+rel_speed * sizes[1], 0, 0);
  const _Mat translate_up_rel        = Mat_translate(0, +rel_speed * sizes[2], 0);
  const _Mat translate_down_rel      = Mat_translate(0, -rel_speed * sizes[2], 0);

  const v3 figure_center = Figure_center(figure);

  const _Mat translate_to_origin   = Mat_translate_v(-figure_center);
  const _Mat translate_from_origin = Mat_translate_v(+figure_center);

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
    case 'w': Figure_transform(figure, translate_forwards_rel ); break;
    case 's': Figure_transform(figure, translate_backwards_rel); break;
    case 'd': Figure_transform(figure, translate_right_rel    ); break;
    case 'a': Figure_transform(figure, translate_left_rel     ); break;
    case 'f': Figure_transform(figure, translate_down_rel     ); break;
    case 'r': Figure_transform(figure, translate_up_rel       ); break;

    case 'W': Figure_transform(figure, translate_forwards_abs ); break;
    case 'S': Figure_transform(figure, translate_backwards_abs); break;
    case 'D': Figure_transform(figure, translate_right_abs    ); break;
    case 'A': Figure_transform(figure, translate_left_abs     ); break;
    case 'F': Figure_transform(figure, translate_down_abs     ); break;
    case 'R': Figure_transform(figure, translate_up_abs       ); break;

    case 'o': Figure_transform(figure, rotate_x_positive  ); break;
    case 'p': Figure_transform(figure, rotate_x_negative  ); break;

    case 'k': Figure_transform(figure, rotate_y_positive  ); break;
    case 'l': Figure_transform(figure, rotate_y_negative  ); break;

    case 'm': Figure_transform(figure, rotate_z_positive  ); break;
    case ',': Figure_transform(figure, rotate_z_negative  ); break;

    case '[': Figure_transform(figure, scale_down         ); break;
    case ']': Figure_transform(figure, scale_up           ); break;

    case 'O': Figure_transform(figure, translate_to_origin); break;
    case 'L': printf("Object at (%f, %f, %f)\n", figure_center[0], figure_center[1], figure_center[2]); break;

    case '!': DO_WIREFRAME            = 1 - DO_WIREFRAME;            break;
    case '@': DO_BACKFACE_ELIMINATION = 1 - DO_BACKFACE_ELIMINATION; break;
    case '#': DO_POLY_FILL            = 1 - DO_POLY_FILL;            break;
    case '$': DO_LIGHT_MODEL          = 1 - DO_LIGHT_MODEL;          break;
    case '%': DO_HALO                 = 1 - DO_HALO;                 break;
    case '^': DO_CLIPPING             = 1 - DO_CLIPPING;             break;

    case '/': BACKFACE_ELIMINATION_SIGN *= -1; break;

    case '`': render_overlay = 1 - render_overlay; break;
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
    G_draw_string(buffer, llx, lly); \
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
  draw_stringf(20, SCREEN_HEIGHT -  40, "(`) Overlay: %d", render_overlay);
  if (!render_overlay) return;

  draw_stringf(20, SCREEN_HEIGHT -  80, "(!) Wframe: %d", DO_WIREFRAME);
  draw_stringf(20, SCREEN_HEIGHT - 100, "(@) BFElim: %d", DO_BACKFACE_ELIMINATION);
  draw_stringf(20, SCREEN_HEIGHT - 120, "(/)   Sign: %d", BACKFACE_ELIMINATION_SIGN);
  draw_stringf(20, SCREEN_HEIGHT - 140, "(#) Fill  : %d", DO_POLY_FILL);
  draw_stringf(20, SCREEN_HEIGHT - 160, "($) Light : %d", DO_LIGHT_MODEL);
  draw_stringf(20, SCREEN_HEIGHT - 180, "(%%) Halos : %d", DO_HALO);
  draw_stringf(20, SCREEN_HEIGHT - 200, "(^) Clip  : %d", DO_CLIPPING);

  draw_stringf(20, 140, "Use +/- to adjust ");
  draw_param(20, 120, "H", param_HALF_ANGLE    , "HAngle : %lf      ", HALF_ANGLE);
  draw_param(20, 100, "B", param_AMBIENT       , "Ambient: %lf      ", AMBIENT);
  draw_param(20,  80, "M", param_DIFFUSE_MAX   , "DifMax : %lf      ", DIFFUSE_MAX);
  draw_param(20,  60, "P", param_SPECULAR_POWER, "SpecPow: %d       ", SPECULAR_POWER);
  draw_param(20,  40, "T", param_HITHER        , "Hither : %lf      ", HITHER);
  draw_param(20,  20, "Y", param_YON           , "Yon    : %lf      ", YON);

  // Draw indices for each figure
  for (int figure_i = 0; figure_i < figures->length; figure_i++) {
    const Figure *figure = FigureList_get(figures, figure_i);

    const v3 center = Figure_center(figure);
    const v2 pixel = pixel_coords(center);

    // TODO: better way to show off-bounds objects
    pixel[0] = clamp(pixel[0], 0, SCREEN_WIDTH);
    pixel[1] = clamp(pixel[1], 0, SCREEN_HEIGHT);
    draw_stringf(pixel[0], pixel[1], "%d        ", figure_i);
  }
}

void show_help() {
  printf("Some controls:\n");
  printf("  e    - Exit program\n");
  printf("  O    - Move object to user\n");
  printf("  L    - Print object location\n");
  printf("  []   - Scale selected object down and up\n");
  printf("  0-9  - Switch figures\n");
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

void event_loop() {
  Figure *focused_figure = NULL;

  char key = '1';
  do {

    // Clear screen
    G_rgb(0, 0, 0);
    G_clear();
    G_rgb(1, 0, 0);
    draw_box();

    const int figure_idx = key - '0';
    if (0 <= figure_idx && figure_idx < figures->length) {
      focused_figure = FigureList_get(figures, figure_idx);
    }

    on_key(focused_figure, key);
    render_figures(figures->items, figures->length, focused_figure, light_source);
    display_state();

  } while ((key = G_wait_key()) != 'e');
}

int main(const int argc, const char **argv) {

  // == Setup == //

  figures = FigureList_new(argc);
  G_init_graphics(SCREEN_WIDTH, SCREEN_HEIGHT);
  draw_init();

  show_help();

  light_source = make_small_figure();
  Figure_move_to(light_source, (v3) { 0, 0, 0 });
  FigureList_append(figures, light_source);

  // Parse command-line args
  for (int i = 1; i < argc; i++) {
    const char *arg = argv[i];
    const int is_path = strchr(arg, '/') != NULL;

    Figure *figure;
    if (is_path) {
      // load a polyhedron from a filname
      const char *filename = arg;
      Polyhedron *polyhedron = load_polyhedron(filename);
      figure = Figure_from_Polyhedron(polyhedron);
      nicely_place_figure(figure);
    } else {
      // get a premade figure
      const char *key = arg;
      figure = figure_instance_lookup(key);

      if (figure == NULL) {
        printf("Unrecognized path or figure name '%s'\n", arg);
        exit(1);
      }
    }
    FigureList_append(figures, figure);
  }

  // == Main == //

  event_loop();

  // == Teardown == //

  FigureList_destroy(figures);
  draw_close();
  G_close();

}
