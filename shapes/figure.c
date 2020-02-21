#ifndef figure_c_INCLUDED
#define figure_c_INCLUDED

#include "lattice.c"
#include "polyhedron.c"
#include "intersector.c"
#include "observer_figure.c"

typedef enum {
  fk_Polyhedron,
  fk_Lattice,
  fk_Intersector,
  fk_Observer
} FigureKind;

typedef struct {
  FigureKind kind;
  struct {
    Polyhedron  *polyhedron;
    Lattice       *lattice;
    Intersector *intersector;
    Observer    *observer;
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

Figure *Figure_from_Lattice(Lattice *lattice) {
#ifdef DEBUG
  if (lattice == NULL) {
    printf("will not wrap null lattice\n");
    exit(1);
  }
#endif

  Figure *figure = malloc(sizeof(Figure));
  figure->kind = fk_Lattice;
  figure->impl.lattice = lattice;
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

Figure *Figure_from_Observer(Observer *observer) {
#ifdef DEBUG
  if (observer == NULL) {
    printf("will not wrap null observer\n");
    exit(1);
  }
#endif

  Figure *figure = malloc(sizeof(Figure));
  figure->kind = fk_Observer;
  figure->impl.observer = observer;
  return figure;
}


// == Lifted functions == //

void Figure_transform(Figure *figure, const _Mat transformation) {
  switch (figure->kind) {
    case fk_Polyhedron: return Polyhedron_transform(figure->impl.polyhedron, transformation);
    case fk_Lattice: return Lattice_transform(figure->impl.lattice, transformation);
    case fk_Intersector: return Intersector_transform(figure->impl.intersector, transformation);
    case fk_Observer: return Observer_transform(figure->impl.observer, transformation);
  }
}

void Figure_bounds_M(v3 *lows, v3 *highs, const Figure *figure) {
  switch (figure->kind) {
    case fk_Polyhedron :return Polyhedron_bounds_M(lows, highs, figure->impl.polyhedron);
    case fk_Lattice: return Lattice_bounds_M(lows, highs, figure->impl.lattice);
    case fk_Intersector: return Intersector_bounds_M(lows, highs, figure->impl.intersector);
    case fk_Observer: return Observer_bounds_M(lows, highs, figure->impl.observer);
  }
}

void Figure_destroy(Figure *figure) {
  switch (figure->kind) {
    case fk_Polyhedron: return Polyhedron_destroy(figure->impl.polyhedron);
    case fk_Lattice: return Lattice_destroy(figure->impl.lattice);
    case fk_Intersector: return Intersector_destroy(figure->impl.intersector);
    case fk_Observer: return Observer_destroy(figure->impl.observer);
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

  const float scale = 0.1;	

  const v3 p0 = scale * (v3) { 0, 0, 0 };	
  const v3 p1 = scale * (v3) { 1, 0, 1 };	
  const v3 p2 = scale * (v3) { 1, 1, 0 };	
  const v3 p3 = scale * (v3) { 0, 1, 1 };	

  Polygon *polygon0 = Polygon_from_points(3,     p1, p2, p3);	
  Polygon *polygon1 = Polygon_from_points(3, p0,     p2, p3);	
  Polygon *polygon2 = Polygon_from_points(3, p0, p1,     p3);	
  Polygon *polygon3 = Polygon_from_points(3, p0, p1, p2    );	

  Polyhedron *polyhedron = Polyhedron_from_polygons(4, polygon0, polygon1, polygon2, polygon3);	
  Figure *figure = Figure_from_Polyhedron(polyhedron);
  return figure;
}

v3 Figure_size(Figure *figure) {
  v3 lows, highs;
  Figure_bounds_M(&lows, &highs, figure);
  return highs - lows;
}


#endif // figure_c_INCLUDED

