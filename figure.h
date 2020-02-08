#ifndef figure_h_INCLUDED
#define figure_h_INCLUDED

#include "locus.h"
#include "polyhedron.h"

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

v3 Figure_center(Figure *figure) {
  switch (figure->kind) {
    case fk_Polyhedron:
      return Polyhedron_center(figure->impl.polyhedron);
    case fk_Locus:
      return Locus_center(figure->impl.locus);
  }
}


#endif // figure_h_INCLUDED

