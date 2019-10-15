#include <FPT.h>
#include "M2d_matrix_tools.c"

int main()
{
  G_init_graphics(700,700) ;
  // rocket
  double rx[8] = {0, 16,  7,  7,  0, -7, -7, -16 } ;
  double ry[8] = {0,  0, 15, 35, 50, 35, 15,   0 } ;

  double A[2],B[2] ;
  int q ;
  
  G_rgb(0,0,0) ;
  G_clear() ;
  G_rgb(1,0,0) ;
  G_fill_polygon(rx,ry,8) ;
  
  G_rgb(1,1,0) ;
  G_wait_click(A) ;
  G_fill_circle(A[0],A[1],2) ;
  G_wait_click(B) ;
  G_fill_circle(B[0],B[1],2) ;
  G_line(A[0],A[1], B[0],B[1]) ;


  q = G_wait_key() ;
}
