#ifndef controls_c_INCLUDED
#define controls_c_INCLUDED

// This file contains the program state relating
// to user input and controls and also handles
// user input

// Should the overlay be rendered?
int render_overlay = 0;

// What is the currently selected parameter?
typedef enum {
  param_HALF_ANGLE,
  param_AMBIENT,
  param_DIFFUSE_MAX,
  param_SPECULAR_POWER,
  param_HITHER,
  param_YON,
} Parameter;

Parameter selected_parameter = param_HALF_ANGLE;

// Two ways to move a figure: with an absolute distance
const float abs_speed = 0.5;
// or moving as a percentage of the object's size
const float rel_speed = 0.1;
// How much the scaling operations scale by
const float scale_amt = 0.01;
// How much the rotation operations rotate
const float rotate_amt = M_PI / 16;

// Movement matrices

// Independent of the figure
static const _Mat translate_backwards_abs = Mat_translate(0, 0, -abs_speed);
static const _Mat translate_forwards_abs  = Mat_translate(0, 0, +abs_speed);
static const _Mat translate_left_abs      = Mat_translate(-abs_speed, 0, 0);
static const _Mat translate_right_abs     = Mat_translate(+abs_speed, 0, 0);
static const _Mat translate_up_abs        = Mat_translate(0, +abs_speed, 0);
static const _Mat translate_down_abs      = Mat_translate(0, -abs_speed, 0);

// Figure-dependent movement
_Mat translate_backwards_rel;
_Mat translate_forwards_rel;
_Mat translate_left_rel;
_Mat translate_right_rel;
_Mat translate_up_rel;
_Mat translate_down_rel;

_Mat translate_to_origin;
_Mat translate_from_origin;

_Mat scale_up;
_Mat scale_down;

_Mat rotate_x_positive;
_Mat rotate_x_negative;
_Mat rotate_y_positive;
_Mat rotate_y_negative;
_Mat rotate_z_positive;
_Mat rotate_z_negative;

void make_dependent_movements(Figure *figure) {
  const v3 sizes = Figure_size(figure);

  { const _Mat m = Mat_translate(0, 0, -rel_speed * sizes[0]); Mat_clone_M(translate_backwards_rel, m); }
  { const _Mat m = Mat_translate(0, 0, +rel_speed * sizes[0]); Mat_clone_M(translate_forwards_rel , m); }
  { const _Mat m = Mat_translate(-rel_speed * sizes[1], 0, 0); Mat_clone_M(translate_left_rel     , m); }
  { const _Mat m = Mat_translate(+rel_speed * sizes[1], 0, 0); Mat_clone_M(translate_right_rel    , m); }
  { const _Mat m = Mat_translate(0, +rel_speed * sizes[2], 0); Mat_clone_M(translate_up_rel       , m); }
  { const _Mat m = Mat_translate(0, -rel_speed * sizes[2], 0); Mat_clone_M(translate_down_rel     , m); }

  const v3 figure_center = Figure_center(figure);
  { const _Mat m = Mat_translate_v(-figure_center); Mat_clone_M(translate_to_origin  , m); }
  { const _Mat m = Mat_translate_v(+figure_center); Mat_clone_M(translate_from_origin, m); }

#define make_rel_to_origin(name) \
  Mat_mult_M(name, name, translate_to_origin); \
  Mat_mult_M(name, translate_from_origin, name);

  { _Mat m = Mat_dilate(1 + scale_amt, 1 + scale_amt, 1 + scale_amt); make_rel_to_origin(m); Mat_clone_M(scale_up, m); }
  { _Mat m = Mat_dilate(1 - scale_amt, 1 - scale_amt, 1 - scale_amt); make_rel_to_origin(m); Mat_clone_M(scale_down, m); }

  { _Mat m = Mat_x_rot(+rotate_amt); make_rel_to_origin(m); Mat_clone_M(rotate_x_positive, m); }
  { _Mat m = Mat_x_rot(-rotate_amt); make_rel_to_origin(m); Mat_clone_M(rotate_x_negative, m); }
  { _Mat m = Mat_y_rot(+rotate_amt); make_rel_to_origin(m); Mat_clone_M(rotate_y_positive, m); }
  { _Mat m = Mat_y_rot(-rotate_amt); make_rel_to_origin(m); Mat_clone_M(rotate_y_negative, m); }
  { _Mat m = Mat_z_rot(+rotate_amt); make_rel_to_origin(m); Mat_clone_M(rotate_z_positive, m); }
  { _Mat m = Mat_z_rot(-rotate_amt); make_rel_to_origin(m); Mat_clone_M(rotate_z_negative, m); }

}


void on_key(const char key) {

  // == Commands that don't require `focused_figure != NULL` == //

  switch(key) {

    // Switch to moving observer rather than object
    case 'q':
      focused_figure = observer_figure;
      break;

    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9': ;
      const int fig_idx = key - '0';
      if (fig_idx < figures->length) {
        focused_figure = FigureList_get(figures, fig_idx);
      }
      break;

    // Parameters
    case '!': DO_WIREFRAME            = !DO_WIREFRAME;            break;
    case '@': DO_BACKFACE_ELIMINATION = !DO_BACKFACE_ELIMINATION; break;
    case '#': DO_POLY_FILL            = !DO_POLY_FILL;            break;
    case '$': DO_LIGHT_MODEL          = !DO_LIGHT_MODEL;          break;
    case '%': DO_HALO                 = !DO_HALO;                 break;
    case '^': DO_CLIPPING             = !DO_CLIPPING;             break;
    case '&': DO_BOUNDING_BOXES       = !DO_BOUNDING_BOXES;       break;

    case '/': BACKFACE_ELIMINATION_SIGN *= -1; break;
    case '`': render_overlay = !render_overlay; break;

    case 'H': selected_parameter = param_HALF_ANGLE;     break;
    case 'B': selected_parameter = param_AMBIENT;        break;
    case 'M': selected_parameter = param_DIFFUSE_MAX;    break;
    case 'P': selected_parameter = param_SPECULAR_POWER; break;
    case 'T': selected_parameter = param_HITHER;         break;
    case 'Y': selected_parameter = param_YON;            break;
  }

  if (key == '=' || key == '-' || key == '+' || key == '_') {
    const int sign = (key == '=' || key == '+') ? 1 : -1;
    const int is_fast = key == '+' || key == '_';

    switch(selected_parameter) {
    case param_HALF_ANGLE    : HALF_ANGLE     += sign * (is_fast ? 0.50 : 0.01);
    case param_AMBIENT       : AMBIENT        += sign * (is_fast ? 0.5  : 0.05);
    case param_DIFFUSE_MAX   : DIFFUSE_MAX    += sign * (is_fast ? 0.5  : 0.05);
    case param_SPECULAR_POWER: SPECULAR_POWER += sign * (is_fast ? 25   : 1   );

    case param_HITHER:
      HITHER += sign * (is_fast ? 1.5 : 0.1);
      if (HITHER > YON) HITHER = YON;  // Don't let HITHER and YON 'cross'
    case param_YON:
      YON += sign * (is_fast ? 1.5 : 0.1);
      if (YON < HITHER) YON = HITHER;  // Don't let HITHER and YON 'cross'
    }
  }

  // == Commands that require that `focused_figure != NULL` == //

  // Noop otherwise
  if (focused_figure == NULL) return;

  switch(key) {

    // Figure-independent movements
    case 'W': Figure_transform(focused_figure, translate_forwards_abs ); break;
    case 'S': Figure_transform(focused_figure, translate_backwards_abs); break;
    case 'D': Figure_transform(focused_figure, translate_right_abs    ); break;
    case 'A': Figure_transform(focused_figure, translate_left_abs     ); break;
    case 'F': Figure_transform(focused_figure, translate_down_abs     ); break;
    case 'R': Figure_transform(focused_figure, translate_up_abs       ); break;

    // Figure-dependent movements
    case 'w':
    case 'a':
    case 's':
    case 'd':
    case 'f':
    case 'r':
    case 'o':
    case 'p':
    case 'k':
    case 'l':
    case 'm':
    case ',':
    case '[':
    case ']':
    case 'O':
      make_dependent_movements(focused_figure);
      switch(key) {
        case 'w': Figure_transform(focused_figure, translate_forwards_rel ); break;
        case 's': Figure_transform(focused_figure, translate_backwards_rel); break;
        case 'd': Figure_transform(focused_figure, translate_right_rel    ); break;
        case 'a': Figure_transform(focused_figure, translate_left_rel     ); break;
        case 'f': Figure_transform(focused_figure, translate_down_rel     ); break;
        case 'r': Figure_transform(focused_figure, translate_up_rel       ); break;

        case 'o': Figure_transform(focused_figure, rotate_x_positive  ); break;
        case 'p': Figure_transform(focused_figure, rotate_x_negative  ); break;
        case 'k': Figure_transform(focused_figure, rotate_y_positive  ); break;
        case 'l': Figure_transform(focused_figure, rotate_y_negative  ); break;
        case 'm': Figure_transform(focused_figure, rotate_z_positive  ); break;
        case ',': Figure_transform(focused_figure, rotate_z_negative  ); break;

        case '[': Figure_transform(focused_figure, scale_down         ); break;
        case ']': Figure_transform(focused_figure, scale_up           ); break;

        case 'O': Figure_transform(focused_figure, translate_to_origin); break;
      }
      break;

    case 'L': ;
      const v3 figure_center = Figure_center(focused_figure);
      printf("Object at (%f, %f, %f)\n", figure_center[0], figure_center[1], figure_center[2]);
      break;

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
    if (selected_parameter == code) { \
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
  draw_stringf(20, SCREEN_HEIGHT - 220, "(&) Boxes : %d", DO_BOUNDING_BOXES);

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
  printf("  &    - Enable/disable bounding boxes\n");
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

#endif // controls_c_INCLUDED
