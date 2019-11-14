#ifndef pointvec_h_INCLUDED
#define pointvec_h_INCLUDED

// Points and vectors are combined into one type

typedef struct {
  double x;
  double y;
  double z;
} PointVec;

// It's recommended to use one of these types
// instead of PointVec  in order to communicate context
typedef PointVec Vec;
typedef PointVec Point;

void PointVec_init(PointVec *pv, const double x, const double y, const double z) {
  pv->x = x;
  pv->y = y;
  pv->z = z;
}

PointVec *PointVec_new(const double x, const double y, const double z) {
  PointVec *point = malloc(sizeof(PointVec));
  PointVec_init(point, x, y, z);
  return point;
}

void PointVec_between_M(Vec *result, const Point *p0, const Point *pf) {
  PointVec_init(result, pf->x - p0->x, pf->y - p0->y, pf->z - p0->z);
}

double PointVec_dot(const Vec *A, const Vec *B) {
  return A->x * B->x + A->y * B->y + A->z * B->z;
}

void PointVec_cross_M(Vec *result, const Vec *A, const Vec *B) {
  PointVec_init(result, A->y * B->z - B->y * A->z,
                        B->x * A->z - A->x * B->z,
                        A->x * B->y - B->x * A->y);
}

double PointVec_mag(const Vec *A) {
  return sqrt(pow(A->x, 2) + pow(A->y, 2) + pow(A->z, 2));
}

void PointVec_scale_M(Vec *result, const Vec *A, double c) {
  PointVec_init(result, A->x * c, A->y * c, A->z * c);
}

void PointVec_normalize_M(Vec *result, const Vec *A) {
  PointVec_scale_M(result, A, 1 / PointVec_mag(A));
}

void PointVec_neg_M(Vec *result, const Vec *A) {
  PointVec_scale_M(result, result, -1);
}

void PointVec_add_M(PointVec *result, const PointVec *A, const PointVec *B) {
  PointVec_init(result, A->x + B->x, A->y + B->y, A->z + B->z);
}

void PointVec_print(const PointVec *pv) {
  printf("POINTVEC [ %lf, %lf, %lf ] POINTVEC\n", pv->x, pv->y, pv->z);
}

#endif // pointvec_h_INCLUDED

