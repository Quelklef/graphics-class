#ifndef point_h_INCLUDED
#define point_h_INCLUDED

typedef struct {
  double x;
  double y;
  double z;
} Point;

void Point_init(Point *point, const double x, const double y, const double z) {
  point->x = x;
  point->y = y;
  point->z = z;
}

Point *Point_new(const double x, const double y, const double z) {
  Point *point = malloc(sizeof(Point));
  Point_init(point, x, y, z);
  return point;
}

void Point_print(const Point* point) {
  printf("POINT [ %lf, %lf, %lf ] POINT\n", point->x, point->y, point->z);
}

#endif // point_h_INCLUDED

