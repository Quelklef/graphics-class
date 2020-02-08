#ifndef figure_c_INCLUDED
#define figure_c_INCLUDED

#include "locus.c"
#include "polyhedron.c"

typedef enum {
  fk_Polyhedron,
  fk_Locus
} FigureKind;

typedef struct {
  FigureKind kind;
  struct {
    Polyhedron *polyhedron;
    Locus *locus;
  } impl;
} Figure;

Figure *Figure_from_Polyhedron(Polyhedron *polyhedron) {
  if (polyhedron == NULL) {
    printf("will not wrap null polyhedron\n");
    exit(1);
  }

  Figure *figure = malloc(sizeof(Figure));
  figure->kind = fk_Polyhedron;
  figure->impl.polyhedron = polyhedron;
  return figure;
}

Figure *Figure_from_Locus(Locus *locus) {
  if (locus == NULL) {
    printf("will not wrap null locus\n");
    exit(1);
  }

  Figure *figure = malloc(sizeof(Figure));
  figure->kind = fk_Locus;
  figure->impl.locus = locus;
  return figure;
}


// == Lifted functions == //

void Figure_transform(Figure *figure, const _Mat transformation) {
  switch (figure->kind) {
    case fk_Polyhedron: return Polyhedron_transform(figure->impl.polyhedron, transformation);
    case fk_Locus: return Locus_transform(figure->impl.locus, transformation);
  }
}

void Figure_bounds_M(
  float *min_x, float *max_x,
  float *min_y, float *max_y,
  float *min_z, float *max_z,
  const Figure *figure
) {
  switch (figure->kind) {
    case fk_Polyhedron: return Polyhedron_bounds_M(min_x, max_x, min_y, max_y, min_z, max_z, figure->impl.polyhedron);
    case fk_Locus: return Locus_bounds_M(min_x, max_x, min_y, max_y, min_z, max_z, figure->impl.locus);
  }
}

void Figure_destroy(Figure *figure) {
  switch (figure->kind) {
    case fk_Polyhedron: return Polyhedron_destroy(figure->impl.polyhedron);
    case fk_Locus: return Locus_destroy(figure->impl.locus);
  }
}

Figure *Figure_clone(const Figure *figure) {
  switch (figure->kind) {
    case fk_Polyhedron: return Figure_from_Polyhedron(Polyhedron_clone(figure->impl.polyhedron));
    case fk_Locus: return Figure_from_Locus(Locus_clone(figure->impl.locus));
  }
}


// == Derived functions == //

v3 Figure_center(const Figure *figure) {
  float min_x, max_x, min_y, max_y, min_z, max_z;
  Figure_bounds_M(&min_x, &max_x, &min_y, &max_y, &min_z, &max_z, figure);
  return (v3) {
    min_x / 2 + max_x / 2,
    min_y / 2 + max_y / 2,
    min_z / 2 + max_z / 2
  };
}

void Figure_move_to(Figure *figure, const v3 target) {
  const v3 center = Figure_center(figure);
  const _Mat translation = Mat_translate_v(-center + target);
  Figure_transform(figure, translation);
}

void nicely_place_figure(Figure *figure) {
  // Move the figure to somewhere nice

  float min_x, max_x, min_y, max_y, min_z, max_z;
  Figure_bounds_M(&min_x, &max_x, &min_y, &max_y, &min_z, &max_z, figure);
  const float width = max_z - min_z;

  // We choose that "somewhere nice" means that x=y=0 and the closest z value is at some z
  const float desired_z = 15;
  v3 desired_location = (v3) { 0, 0, width + desired_z };
  Figure_move_to(figure, desired_location);
}

Figure *make_small_figure() {
  /* Make a small polyhedron. No specified size or shape. Just small. */
  const v3 point = { 0, 0, 0 };
  Locus *locus = Locus_from_points(1, point);
  Figure *figure = Figure_from_Locus(locus);
  return figure;
}

void Figure_size_M(float *result_x_size, float *result_y_size, float *result_z_size, Figure *figure) {
  float min_x, max_x, min_y, max_y, min_z, max_z;
  Figure_bounds_M(&min_x, &max_x, &min_y, &max_y, &min_z, &max_z, figure);
  *result_x_size = max_x - min_x;
  *result_y_size = max_y - min_y;
  *result_z_size = max_z - min_z;
}


#endif // figure_c_INCLUDED

