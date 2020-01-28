#ifndef Mat_mat_tools_INCLUDED
#define Mat_mat_tools_INCLUDED

#include <stdio.h>
#include <math.h>
#include <stdarg.h>

// Define the matrix type.
// The underscore before it is meant
// to remind the user that it is 
// an array type and NOT a struct.
typedef float _Mat[4][4];


/*

 ( x')          (x)
 ( y')  =   M * (y)  
 ( 1 )          (1)

instead of (x',y',1) = (x,y,1) * M  

*/

void Mat_print(const _Mat a) {
  int r, c;
  for (r = 0; r < 4; r++) {
    printf("|");
    for (c = 0; c < 4; c++) {
      printf(" %12.4lf ", a[r][c]);
    }
    printf("|\n");
  }
  printf("\n");
} 

void Mat_clone_M(_Mat result, const _Mat src) {
  for (int i = 0; i < 4; i++) { 
    for (int j = 0; j < 4; j++) {
      result[i][j] = src[i][j];
    }
  }
}

void Mat_scale(_Mat m, float c) {
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      m[i][j] *= c;
    }
  }
}

#define Mat_identity() \
  { { 1, 0, 0, 0 }, \
    { 0, 1, 0, 0 }, \
    { 0, 0, 1, 0 }, \
    { 0, 0, 0, 1 } }

#define Mat_translate(dx, dy, dz) \
  { { 1, 0, 0, (dx) }, \
    { 0, 1, 0, (dy) }, \
    { 0, 0, 1, (dz) }, \
    { 0, 0, 0, 1    } }

#define Mat_translate_v(v) \
  Mat_translate((v)[0], (v)[1], (v)[2])

#define Mat_dilate(dx, dy, dz) \
  { { (dx), 0 , 0 , 0 }, \
    { 0 , (dy), 0 , 0 }, \
    { 0 , 0 , (dz), 0 }, \
    { 0 , 0 , 0 , 1 } }

#define Mat_dilate_v(v) \
  Mat_dilate((v)[0], (v)[1], (v)[2])

#define Mat_x_rot(t) \
  { { 1, 0     , 0      , 0 }, \
    { 0, cos(t), -sin(t), 0 }, \
    { 0, sin(t),  cos(t), 0 }, \
    { 0, 0     , 0      , 1 } }

#define Mat_y_rot(t) \
  { {  cos(t), 0, sin(t), 0 }, \
    { 0      , 1, 0     , 0 }, \
    { -sin(t), 0, cos(t), 0 }, \
    { 0      , 0, 0     , 1 } }

#define Mat_z_rot(t) \
  { { cos(t), -sin(t), 0, 0 }, \
    { sin(t),  cos(t), 0, 0 }, \
    { 0     , 0      , 1, 0 }, \
    { 0     , 0      , 0, 1 } }

float Mat_det(_Mat m) {
  return m[0][0] * ( m[1][1] * m[2][2] * m[3][3] + m[1][2] * m[2][3] * m[3][1] + m[1][3] * m[2][1] * m[3][2]
                     - ( m[1][3] * m[2][2] * m[3][1] + m[1][2] * m[2][1] * m[3][3] + m[1][1] * m[2][3] * m[3][2] ) )
       - m[1][0] * ( m[0][1] * m[2][2] * m[3][3] + m[0][2] * m[2][3] * m[3][1] + m[0][3] * m[2][1] * m[3][2]
                     - ( m[0][3] * m[2][2] * m[3][1] + m[0][2] * m[2][1] * m[3][3] + m[0][1] * m[2][3] * m[3][2] ) )
       + m[2][0] * ( m[0][1] * m[1][2] * m[3][3] + m[0][2] * m[1][3] * m[3][1] + m[0][3] * m[1][1] * m[3][2]
                     - ( m[0][3] * m[1][2] * m[3][1] + m[0][2] * m[1][1] * m[3][3] + m[0][1] * m[1][3] * m[3][2] ) )
       - m[3][0] * ( m[0][1] * m[1][2] * m[2][3] + m[0][2] * m[1][3] * m[2][1] + m[0][3] * m[1][1] * m[2][2]
                     - ( m[0][3] * m[1][2] * m[2][1] + m[0][2] * m[1][1] * m[2][3] + m[0][1] * m[1][3] * m[2][2] ) );
}

void Mat_adj_M(_Mat result, _Mat m) {

  _Mat c;
  Mat_clone_M(c, m);

  result[0][0] =
     + ( c[1][1] * c[2][2] * c[3][3] + c[1][2] * c[2][3] * c[3][1] + c[1][3] * c[2][1] * c[3][2] )
     - ( c[1][3] * c[2][2] * c[3][1] + c[1][2] * c[2][1] * c[3][3] + c[1][1] * c[2][3] * c[3][2] );

  result[0][1] =
     - ( c[0][1] * c[2][2] * c[3][3] + c[0][2] * c[2][3] * c[3][1] + c[0][3] * c[2][1] * c[3][2] )
     + ( c[0][3] * c[2][2] * c[3][1] + c[0][2] * c[2][1] * c[3][3] + c[0][1] * c[2][3] * c[3][2] );

  result[0][2] =
     + ( c[0][1] * c[1][2] * c[3][3] + c[0][2] * c[1][3] * c[3][1] + c[0][3] * c[1][1] * c[3][2] )
     - ( c[0][3] * c[1][2] * c[3][1] + c[0][2] * c[1][1] * c[3][3] + c[0][1] * c[1][3] * c[3][2] );

  result[0][3] =
     - ( c[0][1] * c[1][2] * c[2][3] + c[0][2] * c[1][3] * c[2][1] + c[0][3] * c[1][1] * c[2][2] )
     + ( c[0][3] * c[1][2] * c[2][1] + c[0][2] * c[1][1] * c[2][3] + c[0][1] * c[1][3] * c[2][2] );

  result[1][0] =
     - ( c[1][0] * c[2][2] * c[3][3] + c[1][2] * c[2][3] * c[3][0] + c[1][3] * c[2][0] * c[3][2] )
     + ( c[1][3] * c[2][2] * c[3][0] + c[1][2] * c[2][0] * c[3][3] + c[1][0] * c[2][3] * c[3][2] );

  result[1][1] =
     + ( c[0][0] * c[2][2] * c[3][3] + c[0][2] * c[2][3] * c[3][0] + c[0][3] * c[2][0] * c[3][2] )
     - ( c[0][3] * c[2][2] * c[3][0] + c[0][2] * c[2][0] * c[3][3] + c[0][0] * c[2][3] * c[3][2] );

  result[1][2] =
     - ( c[0][0] * c[1][2] * c[3][3] + c[0][2] * c[1][3] * c[3][0] + c[0][3] * c[1][0] * c[3][2] )
     + ( c[0][3] * c[1][2] * c[3][0] + c[0][2] * c[1][0] * c[3][3] + c[0][0] * c[1][3] * c[3][2] );

  result[1][3] =
     + ( c[0][0] * c[1][2] * c[2][3] + c[0][2] * c[1][3] * c[2][0] + c[0][3] * c[1][0] * c[2][2] )
     - ( c[0][3] * c[1][2] * c[2][0] + c[0][2] * c[1][0] * c[2][3] + c[0][0] * c[1][3] * c[2][2] );

  result[2][0] =
     + ( c[1][0] * c[2][1] * c[3][3] + c[1][1] * c[2][3] * c[3][0] + c[1][3] * c[2][0] * c[3][1] )
     - ( c[1][3] * c[2][1] * c[3][0] + c[1][1] * c[2][0] * c[3][3] + c[1][0] * c[2][3] * c[3][1] );

  result[2][1] =
     - ( c[0][0] * c[2][1] * c[3][3] + c[0][1] * c[2][3] * c[3][0] + c[0][3] * c[2][0] * c[3][1] )
     + ( c[0][3] * c[2][1] * c[3][0] + c[0][1] * c[2][0] * c[3][3] + c[0][0] * c[2][3] * c[3][1] );

  result[2][2] =
     + ( c[0][0] * c[1][1] * c[3][3] + c[0][1] * c[1][3] * c[3][0] + c[0][3] * c[1][0] * c[3][1] )
     - ( c[0][3] * c[1][1] * c[3][0] + c[0][1] * c[1][0] * c[3][3] + c[0][0] * c[1][3] * c[3][1] );

  result[2][3] =
     - ( c[0][0] * c[1][1] * c[2][3] + c[0][1] * c[1][3] * c[2][0] + c[0][3] * c[1][0] * c[2][1] )
     + ( c[0][3] * c[1][1] * c[2][0] + c[0][1] * c[1][0] * c[2][3] + c[0][0] * c[1][3] * c[2][1] );

  result[3][0] =
     - ( c[1][0] * c[2][1] * c[3][2] + c[1][1] * c[2][2] * c[3][0] + c[1][2] * c[2][0] * c[3][1] )
     + ( c[1][2] * c[2][1] * c[3][0] + c[1][1] * c[2][0] * c[3][2] + c[1][0] * c[2][2] * c[3][1] );

  result[3][1] =
     + ( c[0][0] * c[2][1] * c[3][2] + c[0][1] * c[2][2] * c[3][0] + c[0][2] * c[2][0] * c[3][1] )
     - ( c[0][2] * c[2][1] * c[3][0] + c[0][1] * c[2][0] * c[3][2] + c[0][0] * c[2][2] * c[3][1] );

  result[3][2] =
     - ( c[0][0] * c[1][1] * c[3][2] + c[0][1] * c[2][2] * c[3][0] + c[0][2] * c[1][0] * c[3][1] )
     + ( c[0][2] * c[1][1] * c[3][0] + c[0][1] * c[1][0] * c[3][2] + c[0][0] * c[1][2] * c[3][1] );

  result[3][3] =
     + ( c[0][0] * c[1][1] * c[2][2] + c[0][1] * c[1][2] * c[2][0] + c[0][2] * c[1][0] * c[2][1] )
     - ( c[0][2] * c[1][1] * c[2][0] + c[0][1] * c[1][0] * c[2][2] + c[0][0] * c[1][2] * c[2][1] );

}

void Mat_inv_M(_Mat result, _Mat m) {
  Mat_adj_M(result, m);
  Mat_scale(result, 1 / Mat_det(m));
}

void Mat_mult_M(_Mat result, const _Mat a, const _Mat b) {
  // result = a * b
  // this is SAFE, i.e. the user can make a call such as 
  // M2d_mat_mult(p,  p,q) or M2d_mat_mult(p,  q,p) or  M2d_mat_mult(p, p,p)
  _Mat u;
  Mat_clone_M(u, a);
  _Mat v;
  Mat_clone_M(v, b);

  for(int i = 0; i < 4; i++) {
    for(int j = 0; j < 4; j++) {
      result[i][j] = u[i][0] * v[0][j]
                   + u[i][1] * v[1][j]
                   + u[i][2] * v[2][j]
                   + u[i][3] * v[3][j];
    }
  }
}

void Mat_chain_M(_Mat result, const int count, ...) {
  // Mat_chain(r, a, b, c) makes r = c*b*a*I

  va_list args;
  va_start(args, count);

  const _Mat id = Mat_identity();
  Mat_clone_M(result, id);

  for (int i = 0; i < count; i++) {
    float (*mat)[] = va_arg(args, float(*)[]);
    Mat_mult_M(result, mat, result);
  }

  va_end(args);
}

#endif // Mat_mat_tools_INCLUDED
