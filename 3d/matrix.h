#ifndef Mat_mat_tools_INCLUDED
#define Mat_mat_tools_INCLUDED

#include <stdio.h>
#include <math.h>
#include <stdarg.h>

// Define the matrix type.
// The underscore before it is meant
// to remind the user that it is 
// an array type and NOT a struct.
typedef double _Mat[4][4];


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
    printf("|");
    printf("\n");
  }
} 

void Mat_copy_M(_Mat result, const _Mat src) {
  for (int i = 0; i < 4; i++) { 
    for (int j = 0; j < 4; j++) {
      result[i][j] = src[i][j];
    }
  }
} 

void Mat_identity_M(_Mat result) {
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      if (r == c) result[r][c] = 1.0;
      else        result[r][c] = 0.0;
    }
  }
} 

void Mat_translation_M(_Mat result, const double dx, const double dy, const double dz) {
  Mat_identity_M(result);
  result[0][3] = dx;
  result[1][3] = dy;
  result[2][3] = dz;
}

void Mat_scaling_M(_Mat result, const double sx, const double sy, const double sz) {
  Mat_identity_M(result);
  result[0][0] = sx;
  result[1][1] = sy;
  result[2][2] = sz;
}

void Mat_z_rotation_cs_M(_Mat result, const double cs, const double sn) {
  result[0][0] =  cs;   result[0][1] = -sn;   result[0][2] =   0;   result[0][3] =   0;
  result[1][0] =  sn;   result[1][1] =  cs;   result[1][2] =   0;   result[1][3] =   0;
  result[2][0] =   0;   result[2][1] =   0;   result[2][2] =   1;   result[2][3] =   0;
  result[3][0] =   0;   result[3][1] =   0;   result[3][2] =   0;   result[3][3] =   1;
}

void Mat_x_rotation_cs_M(_Mat result, const double cs, const double sn) {
  result[0][0] =   1;   result[0][1] =   0;   result[0][2] =   0;   result[0][3] =   0;
  result[1][0] =   0;   result[1][1] =  cs;   result[1][2] = -sn;   result[1][3] =   0;
  result[2][0] =   0;   result[2][1] =  sn;   result[2][2] =  cs;   result[2][3] =   0;
  result[3][0] =   0;   result[3][1] =   0;   result[3][2] =   0;   result[3][3] =   1;
}

void Mat_y_rotation_cs_M(_Mat result, const double cs, const double sn) {
  result[0][0] =  cs;   result[0][1] =   0;   result[0][2] =  sn;   result[0][3] =   0;
  result[1][0] =   0;   result[1][1] =   1;   result[1][2] =   0;   result[1][3] =   0;
  result[2][0] = -sn;   result[2][1] =   0;   result[2][2] =  cs;   result[2][3] =   0;
  result[3][0] =   0;   result[3][1] =   0;   result[3][2] =   0;   result[3][3] =   1;
}


void Mat_mult_M(_Mat result, const _Mat a, const _Mat b) {
  // result = a * b
  // this is SAFE, i.e. the user can make a call such as 
  // M2d_mat_mult(p,  p,q) or M2d_mat_mult(p,  q,p) or  M2d_mat_mult(p, p,p)
  _Mat u;
  Mat_copy_M(u, a);
  _Mat v;
  Mat_copy_M(v, b);

  for(int i = 0; i < 4; i++) {
    for(int j = 0; j < 4; j++) {
      result[i][j] = u[i][0] * v[0][j]
                   + u[i][1] * v[1][j]
                   + u[i][2] * v[2][j]
                   + u[i][3] * v[3][j];
    }
  }
}

void Mat_mat_mult_pt_M(double result[3], const _Mat m, const double Q[3]) {
  // result = m*Q
  // SAFE, user may make a call like M2d_mat_mult_pt (W, m,W);
  double u[2];
  u[0] = Q[0];
  u[1] = Q[1];
  for (int i = 0; i < 2; i++) {
    result[i] = m[i][0] * u[0]
         + m[i][1] * u[1]
         + m[i][2] * 1   ;
  }
}

void Mat_mat_mult_points_M(double result_X[], double result_Y[], double result_Z[],
                           const _Mat m,
                           const double x[], const double y[], const double z[],
                           const int numpoints) {
  // |X0 X1 X2 ...|       |x0 x1 x2 ...|
  // |Y0 Y1 Y2 ...| = m * |y0 y1 y2 ...|
  // |Z0 Z1 Z2 ...|       |z0 z1 z2 ...|
  // | 1  1  1 ...|       | 1  1  1 ...|

  // SAFE, user may make a call like M2d_mat_mult_points (x,y, m, x,y, n);
  double copyX[numpoints];
  double copyY[numpoints];
  double copyZ[numpoints];

  for (int i = 0; i < numpoints; i++) {
    copyX[i] = x[i];
    copyY[i] = y[i];
    copyZ[i] = z[i];
  }

  for (int j = 0; j < numpoints; j++) {
    result_X[j] = m[0][0] * copyX[j] + m[0][1] * copyY[j] + m[0][2] * copyZ[j] + m[0][3] * 1;
    result_Y[j] = m[1][0] * copyX[j] + m[1][1] * copyY[j] + m[1][2] * copyZ[j] + m[1][3] * 1;
    result_Z[j] = m[2][0] * copyX[j] + m[2][1] * copyY[j] + m[2][2] * copyZ[j] + m[2][3] * 1;
  }

}

#endif // Mat_mat_tools_INCLUDED
