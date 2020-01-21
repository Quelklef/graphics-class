#ifndef line_h_INCLUDED
#define line_h_INCLUDED

#include "pointvec.h"

// We define a line by two points
typedef struct Line {
  Point p0;
  Point pf;
} Line;

void Line_between(Line *result, const Point *p0, const Point *pf) {
  result->p0 = *p0;
  result->pf = *pf;
}

void Line_vector_M(Vec *result, const Line *line) {
  PointVec_between_M(result, &line->p0, &line->pf);
}

#endif // line_h_INCLUDED

