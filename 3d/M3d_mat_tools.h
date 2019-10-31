#ifndef M3d_mat_tools_INCLUDED
#define M3d_mat_tools_INCLUDED

#include <stdio.h>
#include <math.h>
#include <stdarg.h>


/*

 ( x')          (x)
 ( y')  =   M * (y)  
 ( 1 )          (1)

instead of (x',y',1) = (x,y,1) * M  

*/

void M3d_print_mat(const double a[4][4]) {
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

void M3d_copy_mat(double dest[4][4], const double src[4][4]) {
  for (int i = 0; i < 4; i++) { 
    for (int j = 0; j < 4; j++) {
      dest[i][j] = src[i][j];
    }
  }
} 

void M3d_make_identity(double a[4][4]) {
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      if (r == c) a[r][c] = 1.0;
      else        a[r][c] = 0.0;
    }
  }
} 

void M3d_make_translation(double a[4][4], const double dx, const double dy, const double dz) {
  M3d_make_identity(a);
  a[0][3] = dx;
  a[1][3] = dy;
  a[2][3] = dz;
}

void M3d_make_scaling(double a[4][4], const double sx, const double sy, const double sz) {
  for (int r = 0; r < 3; r++) {
      for (int c = 0; c < 3; c++) {
           if (r == c) a[r][c] = 1.0;
           else        a[r][c] = 0.0;
      }
  }

  a[0][0] = sx;
  a[1][1] = sy;
  a[2][2] = sz;
}

void M3d_make_x_rotation_cs(double a[4][4], const double cs, const double sn) {
  // this one assumes cosine and sine are already known
  a[0][0] = cs;   a[0][1] = -sn;   a[0][2] =  0;   a[0][3] =  0;
  a[1][0] = sn;   a[1][1] =  cs;   a[1][2] =  0;   a[1][3] =  0;
  a[2][0] =  0;   a[2][1] =   0;   a[2][2] =  1;   a[2][3] =  0;
  a[3][0] =  0;   a[3][1] =   0;   a[3][2] =  0;   a[3][3] =  1;
}

void M3d_make_y_rotation_cs(double a[4][4], const double cs, const double sn) {
  // this one assumes cosine and sine are already known
  a[0][0] =  1;   a[0][1] =   0;   a[0][2] =   0;   a[0][3] =  0;
  a[1][0] =  0;   a[1][1] =  cs;   a[1][2] = -sn;   a[1][3] =  0;
  a[2][0] =  0;   a[2][1] =  sn;   a[2][2] =  cs;   a[2][3] =  0;
  a[3][0] =  0;   a[3][1] =   0;   a[3][2] =   0;   a[3][3] =  1;
}

void M3d_make_z_rotation_cs(double a[4][4], const double cs, const double sn) {
  // this one assumes cosine and sine are already known
  a[0][0] =  1;   a[0][1] =   0;   a[0][2] =   0;   a[0][3] =   0;
  a[1][0] =  0;   a[1][1] =   1;   a[1][2] =   0;   a[1][3] =   0;
  a[2][0] =  0;   a[2][1] =   0;   a[2][2] =  cs;   a[2][3] = -sn;
  a[3][0] =  0;   a[3][1] =   0;   a[3][2] =  sn;   a[3][3] =  cs;
}


void M3d_mat_mult(double res[4][4], const double a[4][4], const double b[4][4]) {
  // res = a * b
  // this is SAFE, i.e. the user can make a call such as 
  // M2d_mat_mult(p,  p,q) or M2d_mat_mult(p,  q,p) or  M2d_mat_mult(p, p,p)
  double u[4][4];
  double v[4][4];
  M3d_copy_mat(u, a);
  M3d_copy_mat(v, b);

  for(int i = 0; i < 4; i++) {
    for(int j = 0; j < 4; j++) {
      res[i][j] = u[i][0] * v[0][j]
                + u[i][1] * v[1][j]
                + u[i][2] * v[2][j]
		+ u[i][3] * v[3][j];
    }
  }
}

void M3d_mat_mult_pt(double P[3], const double m[4][4], const double Q[3]) {
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

void M3d_mat_mult_points(double X[], double Y[], double Z[],
                         const double m[4][4],
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
    X[j] = m[0][0] * copyX[j] + m[0][1] * copyY[j] + m[0][2] * copyZ[j];
    Y[j] = m[1][0] * copyX[j] + m[1][1] * copyY[j] + m[1][2] * copyZ[j];
    Z[j] = m[2][0] * copyX[j] + m[2][1] * copyY[j] + m[2][2] * copyZ[j];
  }

}

#endif // M3d_mat_tools_INCLUDED
