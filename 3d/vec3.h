#ifndef vec3_h_INCLUDED
#define vec3_h_INCLUDED

typedef struct {
  double x;
  double y;
  double z;
} Vec3;

Vec3 *Vec3_new(double x, double y, double z) {
  Vec3 *v = malloc(sizeof(Vec3));
  v->x = x;
  v->y = y;
  v->z = z;
  return v;
}

Vec3 *Vec3_between(const double x0, const double y0, const double z0,
                  const double xf, const double yf, const double zf) {
  return Vec3_new(xf - x0, yf - y0, zf - z0);
}

double Vec3_dot(const Vec3 *A, const Vec3 *B) {
  return A->x * B->x + A->y * B->y + A->z * B->z;
}

Vec3 *Vec3_cross(const Vec3 *A, const Vec3 *B) {
  return Vec3_new(+(A->y * B->z - B->y * A->z),
                  -(A->x * B->z - B->x * A->z),
                  +(A->x * B->y - B->x * A->y));
}

double Vec3_mag(const Vec3 *A) {
  return sqrt(pow(A->x, 2) + pow(A->y, 2) + pow(A->z, 2));
}

Vec3 *Vec3_scale(const Vec3 *A, double c) {
  return Vec3_new(A->x * c, A->y * c, A->z * c);
}

Vec3 *Vec3_normalize(const Vec3 *A) {
  return Vec3_scale(A, 1 / Vec3_mag(A));
}

Vec3 *Vec3_neg(const Vec3 *A) {
  return Vec3_scale(A, -1);
}

Vec3 *plane_normal(const double *xs, const double *ys, const double *zs, const int point_count) {
  // Takes a polygon which MUST have at least 3 points
  // Returns the (length-1) normal veector to the polygon
  if (point_count < 3) {
    printf("ERROR: In plane_normal(): requires point_count >= 3");
    exit(1);
  }
  const Vec3 *A = Vec3_normalize(Vec3_between(xs[0], ys[0], zs[0], xs[1], ys[1], zs[1]));
  const Vec3 *B = Vec3_normalize(Vec3_between(xs[0], ys[0], zs[0], xs[2], ys[2], zs[2]));
  return Vec3_cross(A, B);
}

#endif // vec3_h_INCLUDED

