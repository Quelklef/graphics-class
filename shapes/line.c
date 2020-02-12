#ifndef line_c_INCLUDED
#define line_c_INCLUDED

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

void Line_transform(Line *line, _Mat transformation) {
  line->p0 = v3_transform(line->p0, transformation);
  line->pf = v3_transform(line->pf, transformation);
}

#endif // line_c_INCLUDED

