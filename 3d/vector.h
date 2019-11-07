#ifndef vector_h_INCLUDED
#define vector_h_INCLUDED

#include "point.h"

typedef struct {
  double x;
  double y;
  double z;
} Vec;

void Vec_init(Vec *vec, const double x, const double y, const double z) {
  vec->x = x;
  vec->y = y;
  vec->z = z;
}

void Vec_between_M(Vec *result, const Point *p0, const Point *pf) {
  Vec_init(result, pf->x - p0->x, pf->y - p0->y, pf->z - p0->z);
}

double Vec_dot(const Vec *A, const Vec *B) {
  return A->x * B->x + A->y * B->y + A->z * B->z;
}

void Vec_cross_M(Vec *result, const Vec *A, const Vec *B) {
  Vec_init(result, A->y * B->z - B->y * A->z,
                   B->x * A->z - A->x * B->z,
                   A->x * B->y - B->x * A->y);
}

double Vec_mag(const Vec *A) {
  return sqrt(pow(A->x, 2) + pow(A->y, 2) + pow(A->z, 2));
}

void Vec_scale_M(Vec *result, const Vec *A, double c) {
  Vec_init(result, A->x * c, A->y * c, A->z * c);
}

void Vec_normalize_M(Vec *result, const Vec *A) {
  Vec_scale_M(result, A, 1 / Vec_mag(A));
}

void Vec_neg_M(Vec *result, const Vec *A) {
  Vec_scale_M(result, result, -1);
}

void Poly_normal_M(Vec *result, const Poly *poly) {
  // Takes a polygon which MUST have at least 3 points
  // Returns the (length-1) normal veector to the polygon
  if (poly->point_count < 3) {
    printf("ERROR: In plane_normal(): requires point_count >= 3");
    exit(1);
  }

  Vec A;
  Vec_between_M(&A, poly->points[0], poly->points[1]);
  Vec_normalize_M(&A, &A);

  Vec B;
  Vec_between_M(&B, poly->points[0], poly->points[2]);
  Vec_normalize_M(&B, &B);

  Vec_cross_M(result, &A, &B);
}

#endif // vector_h_INCLUDED

