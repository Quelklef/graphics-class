#ifndef v3_c_INCLUDED
#define v3_c_INCLUDED

// Points and vectors are combined into one type

typedef float v3 __attribute__ (( vector_size(3 * sizeof(float)) ));

void v3_print(v3 v) {
  printf("v3 [ %f, %f, %f ] v3", v[0], v[1], v[2]);
}

float v3_dot(v3 a, v3 b) {
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

v3 v3_cross(const v3 a, const v3 b) {
  return (v3) {
    a[1] * b[2] - b[1] * a[2],
    b[0] * a[2] - a[0] * b[2],
    a[0] * b[1] - b[0] * a[1]
  };
}

float v3_mag(v3 v) {
  return sqrt( pow(v[0], 2) + pow(v[1], 2) + pow(v[2], 2) );
}

v3 v3_normalize(v3 v) {
  return v / v3_mag(v);
}

v3 v3_transform(v3 v, const _Mat transformation) {
  // Matrix multiplication

  const float x = v[0];
  const float y = v[1];
  const float z = v[2];

  float result_x, result_y, result_z;

  result_x = transformation[0][0] * x
           + transformation[0][1] * y
           + transformation[0][2] * z
           + transformation[0][3] * 1;

  result_y = transformation[1][0] * x
           + transformation[1][1] * y
           + transformation[1][2] * z
           + transformation[1][3] * 1;

  result_z = transformation[2][0] * x
           + transformation[2][1] * y
           + transformation[2][2] * z
           + transformation[2][3] * 1;

  return (v3) { result_x, result_y, result_z };
}

int v3_eq(const v3 a, const v3 b) {
  return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
}

static const v3 v3_zero = { 0, 0, 0 };

#endif // v3_c_INCLUDED
