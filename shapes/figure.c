#ifndef figure_c_INCLUDED
#define figure_c_INCLUDED

#include "locus.c"
#include "polyhedron.c"
#include "intersector.c"

typedef enum {
  fk_Polyhedron,
  fk_Locus,
  fk_Intersector
} FigureKind;

typedef struct {
  FigureKind kind;
  struct {
    Polyhedron *polyhedron;
    Locus *locus;
    Intersector *intersector;
  } impl;
} Figure;

Figure *Figure_from_Polyhedron(Polyhedron *polyhedron) {
#ifdef DEBUG
  if (polyhedron == NULL) {
    printf("will not wrap null polyhedron\n");
    exit(1);
  }
#endif

  Figure *figure = malloc(sizeof(Figure));
  figure->kind = fk_Polyhedron;
  figure->impl.polyhedron = polyhedron;
  return figure;
}

Figure *Figure_from_Locus(Locus *locus) {
#ifdef DEBUG
  if (locus == NULL) {
    printf("will not wrap null locus\n");
    exit(1);
  }
#endif

  Figure *figure = malloc(sizeof(Figure));
  figure->kind = fk_Locus;
  figure->impl.locus = locus;
  return figure;
}

Figure *Figure_from_Intersector(Intersector *intersector) {
#ifdef DEBUG
  if (intersector == NULL) {
    printf("will not wrap null intersector\n");
    exit(1);
  }
#endif

  Figure *figure = malloc(sizeof(Figure));
  figure->kind = fk_Intersector;
  figure->impl.intersector = intersector;
  return figure;
}


// == Lifted functions == //

void Figure_transform(Figure *figure, const _Mat transformation) {
  switch (figure->kind) {
    case fk_Polyhedron: return Polyhedron_transform(figure->impl.polyhedron, transformation);
    case fk_Locus: return Locus_transform(figure->impl.locus, transformation);
    case fk_Intersector: return Intersector_transform(figure->impl.intersector, transformation);
  }
}

void Figure_bounds_M(v3 *lows, v3 *highs, const Figure *figure) {
  switch (figure->kind) {
  case fk_Polyhedron :return Polyhedron_bounds_M(lows, highs, figure->impl.polyhedron);
  case fk_Locus: return Locus_bounds_M(lows, highs, figure->impl.locus);
  case fk_Intersector: return Intersector_bounds_M(lows, highs, figure->impl.intersector);
  }
}

void Figure_destroy(Figure *figure) {
  switch (figure->kind) {
    case fk_Polyhedron: return Polyhedron_destroy(figure->impl.polyhedron);
    case fk_Locus: return Locus_destroy(figure->impl.locus);
    case fk_Intersector: return Intersector_destroy(figure->impl.intersector);
  }
}


// == Derived functions == //

v3 Figure_center(const Figure *figure) {
  v3 lows, highs;
  Figure_bounds_M(&lows, &highs, figure);
  return (v3) {
    lows[0] / 2 + highs[0] / 2,
    lows[1] / 2 + highs[1] / 2,
    lows[2] / 2 + highs[2] / 2
  };
}

void Figure_move_to(Figure *figure, const v3 target) {
  const v3 center = Figure_center(figure);
  const _Mat translation = Mat_translate_v(-center + target);
  Figure_transform(figure, translation);
}

void nicely_place_figure(Figure *figure) {
  // Move the figure to somewhere nice

  v3 lows, highs;
  Figure_bounds_M(&lows, &highs, figure);
  const float width = highs[2] - lows[2];

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

v3 Figure_size(Figure *figure) {
  v3 lows, highs;
  Figure_bounds_M(&lows, &highs, figure);
  return highs - lows;
}


#endif // figure_c_INCLUDED

