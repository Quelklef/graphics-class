#include <math.h>

#include <libgfx.h>

#include "state.c"
#include "matrix.c"
#include "shapes/figure.c"
#include "shapes/v2.c"
#include "shapes/instances.c"
#include "rendering/draw.c"
#include "rendering/render.c"

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

void draw_box() {
  G_fill_rectangle(0               , 0                , SCREEN_WIDTH, 1            );
  G_fill_rectangle(SCREEN_WIDTH - 1, 0                , 1           , SCREEN_HEIGHT);
  G_fill_rectangle(0               , SCREEN_HEIGHT - 1, SCREEN_WIDTH, 1            );
  G_fill_rectangle(0               , 0                , 1           , SCREEN_HEIGHT);
}

#include "controls.c"

void event_loop() {

  char key = '1';
  do {

    // Clear screen
    G_rgb(0, 0, 0);
    G_clear();
    G_rgb(1, 0, 0);
    draw_box();

    on_key(key);
    render_figures(figures->items, figures->length, focused_figure, observer, light_source);
    display_state();

  } while ((key = G_wait_key()) != 'e');
}

int main(const int argc, const char **argv) {

  // == Setup == //

  figures = FigureList_new(argc);
  G_init_graphics(SCREEN_WIDTH, SCREEN_HEIGHT);
  draw_init();

  //show_help();

  observer = Observer_new();
  observer_figure = Figure_from_Observer(observer);

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
