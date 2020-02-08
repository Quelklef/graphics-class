#ifndef line_h_INCLUDED
#define line_h_INCLUDED

#include "v3.c"

// We define a line by two points
typedef struct Line {
  v3 p0;
  v3 pf;
} Line;

void Line_between(Line *result, const v3 p0, const v3 pf) {
  result->p0 = p0;
  result->pf = pf;
}

v3 Line_vector(const Line *line) {
  return line->pf - line->p0;
}

#endif // line_h_INCLUDED

