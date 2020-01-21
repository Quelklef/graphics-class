#ifndef pointvec_h_INCLUDED
#define pointvec_h_INCLUDED

// Points and vectors are combined into one type

typedef struct PointVec {
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

void PointVec_clone_M(PointVec *result, const PointVec *source) {
  result->x = source->x;
  result->y = source->y;
  result->z = source->z;
}

PointVec *PointVec_new(const double x, const double y, const double z) {
  PointVec *point = malloc(sizeof(PointVec));
  PointVec_init(point, x, y, z);
  return point;
}

PointVec *PointVec_clone(PointVec *source) {
  return PointVec_new(source->x, source->y, source->z);
}

void PointVec_destroy(PointVec *pv) {
  free(pv);
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

double PointVec_magnitude(const Vec *A) {
  return sqrt(pow(A->x, 2) + pow(A->y, 2) + pow(A->z, 2));
}

void PointVec_scale_M(PointVec *result, const PointVec *pv, const double s) {
  result->x = pv->x * s;
  result->y = pv->y * s;
  result->z = pv->z * s;
}

void PointVec_scale(Vec *A, double c) {
  PointVec_scale_M(A, A, c);
}

void PointVec_normalize_M(PointVec *result, const PointVec *pv) {
  PointVec_scale_M(result, pv, 1 / PointVec_magnitude(pv));
}

void PointVec_normalize(Vec *v) {
  PointVec_normalize_M(v, v);
}

void PointVec_negate_M(PointVec *result, const PointVec *v) {
  PointVec_scale_M(result, v, -1);
}

void PointVec_negate(Vec *v) {
  PointVec_negate_M(v, v);
}

void PointVec_add_M(PointVec *result, const PointVec *A, const PointVec *B) {
  PointVec_init(result, A->x + B->x, A->y + B->y, A->z + B->z);
}

void PointVec_add(PointVec *pv, const PointVec *a) {
  PointVec_add_M(pv, pv, a);
}

void PointVec_subtract_M(PointVec *result, const PointVec *A, const PointVec *B) {
  PointVec_init(result, A->x - B->x, A->y - B->y, A->z - B->z);
}

void PointVec_subtract(PointVec *pv, const PointVec *v) {
  PointVec_subtract_M(pv, pv, v);
}

void PointVec_print(const PointVec *pv) {
  printf("POINTVEC [ %lf, %lf, %lf ] POINTVEC\n", pv->x, pv->y, pv->z);
}

#endif // pointvec_h_INCLUDED

