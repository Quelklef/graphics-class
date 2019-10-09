#include <stdio.h>
#include <math.h>
#include <stdarg.h>


/*

 ( x')          (x)
 ( y')  =   M * (y)  
 ( 1 )          (1)

instead of (x',y',1) = (x,y,1) * M  

*/

void M2d_print_mat (double a[3][3]) {
  int r, c;
  for (r = 0; r < 3; r++) {
    printf("|");
    for (c = 0; c < 3; c++) {
      printf(" %12.4lf ", a[r][c]);
    }
    printf("|");
    printf("\n");
  }
} 

void M2d_copy_mat(double dest[3][3], double src[3][3]) {
  for (int i = 0; i < 3; i++) { 
    for (int j = 0; j < 3; j++) {
      dest[i][j] = src[i][j];
    }
  }
} 

void M2d_make_identity(double a[3][3]) {
  for (int r = 0; r < 3; r++) {
    for (int c = 0; c < 3; c++) {
      if (r == c) a[r][c] = 1.0;
      else        a[r][c] = 0.0;
    }
  }
} 

void M2d_make_translation(double a[3][3], double dx, double dy) {
  M2d_make_identity(a);
  a[0][2] = dx;
  a[1][2] = dy;  
}

void M2d_make_scaling(double a[3][3], double sx, double sy) {
  for (int r = 0; r < 3; r++) {
      for (int c = 0; c < 3; c++) {
           if (r == c) a[r][c] = 1.0;
           else        a[r][c] = 0.0;
      }
  }

  a[0][0] = sx;
  a[1][1] = sy;
}

void M2d_make_rotation_cs(double a[3][3], double cs, double sn) {
  // this one assumes cosine and sine are already known
  a[0][0] = cs;   a[0][1] = -sn;   a[0][2] = 0;
  a[1][0] = sn;   a[1][1] =  cs;   a[1][2] = 0;
  a[2][0] =  0;   a[2][1] =   0;   a[2][2] = 1;
}

void M2d_make_rotation_radians(double a[3][3], double radians) {
  M2d_make_rotation_cs(a, cos(radians), sin(radians));
}

void M2d_make_rotation_degrees(double a[3][3], double degrees) {
  M2d_make_rotation_radians(a, degrees * M_PI / 180);
}

void M2d_mat_mult(double res[3][3], double a[3][3], double b[3][3]) {
  // res = a * b
  // this is SAFE, i.e. the user can make a call such as 
  // M2d_mat_mult(p,  p,q) or M2d_mat_mult(p,  q,p) or  M2d_mat_mult(p, p,p)
  double u[3][3];
  double v[3][3];
  M2d_copy_mat(u, a);
  M2d_copy_mat(v, b);

  for(int i = 0; i < 3; i++) {
    for(int j = 0; j < 3; j++) {
      res[i][j] = u[i][0] * v[0][j]
                + u[i][1] * v[1][j]
                + u[i][2] * v[2][j];
    }
  }
}

void M2d_mat_mult_pt(double P[2], double m[3][3], double Q[2]) {
  // P = m*Q
  // SAFE, user may make a call like M2d_mat_mult_pt (W, m,W);
  double u[2];
  u[0] = Q[0];
  u[1] = Q[1];
  for (int i = 0; i < 2; i++) {
    P[i] = m[i][0] * u[0]
         + m[i][1] * u[1]
         + m[i][2] * 1   ;
  }
}

void M2d_mat_mult_points(double X[], double Y[],
                         double m[3][3],
                         double x[], double y[], int numpoints) {
  // |X0 X1 X2 ...|       |x0 x1 x2 ...|
  // |Y0 Y1 Y2 ...| = m * |y0 y1 y2 ...|
  // | 1  1  1 ...|       | 1  1  1 ...|

  // SAFE, user may make a call like M2d_mat_mult_points (x,y, m, x,y, n);
  double copyX[numpoints];
  double copyY[numpoints];

  for (int i = 0; i < numpoints; i++) {
    copyX[i] = x[i];
    copyY[i] = y[i];
  }

  for (int j = 0; j < numpoints; j++) {
    X[j] = m[0][0] * copyX[j] + m[0][1] * copyY[j] + m[0][2];
    Y[j] = m[1][0] * copyX[j] + m[1][1] * copyY[j] + m[1][2];
  }
}
