#include <stdio.h>
#include <FPT.h>
#include <float.h>

#include "M2d_matrix_tools.c"




double RotX[3][3];
M3d_make_x_rotation_cs(RotX, cos(M_PI/16), sin(M_PI/16));

double RotY[3][3];
M3d_make_y_rotation_cs(RotX, cos(M_PI/16), sin(M_PI/16));

double RotZ[3][3];
M3d_make_z_rotation_cs(RotX, cos(M_PI/16), sin(M_PI/16));

int polygonCount[10];
double storageMat[3][3];

// Array of the actual points in each polygon
double polygonXs[10][10000][8];
double polygonYs[10][10000][8];
double polygonZs[10][10000][8];

// amount of points in each polygon
int pointCount[10];

// Array of the number of points in each polygon
int polygonPointCount[10][10000];

//colors
//double colors[10000][3];

// Color & display the polygon
FILE *file;
/*
void colorPoly(int index){  
G_rgb(1,1,1);
G_clear();
  for (int i = 0; i < polygonCount[index]; i++) {
    G_rgb(colors[index][i][0], colors[i][1], colors[i][2]);
    G_fill_polygon(polygonXs[index][i], polygonYs[i], polygonPointCounts[i]);
  }
}
*/

void arrayMin(double *ns, int n,
              double *min) {
  *min = DBL_MAX;
  for (int i = 0; i < n; i++) {
    if (ns[i] < *min) *min = ns[i];
  }
}

void arrayMax(double *ns, int n,
              double *max) {
  *max = DBL_MIN;
  for (int i = 0; i < n; i++) {
    if (ns[i] > *max) *max = ns[i];
  }
}

void collectPoints(char *filename, int index) {
  G_rgb(1, 1, 1);
  G_clear();

   file = fopen(filename, "r");

  if (file == NULL) {
    printf("Can not open file: %s\n", filename);
    exit(0);
  }

  fscanf(file, "%d", &pointCount[index]);

  // Initialize (x, y) arrays to store points
  double xs[pointCount[index]+1];
  double ys[pointCount[index]+1];
  double zs[pointCount[index]+1];


  for (int i = 0; i < pointCount[index]+1; i++) {
     if(i = pointCount[index]) {
        xs = 0;
	ys = 0;
	zs = 0;
     } 
     else{
        fscanf(file, "%lf %lf %lf", &xs[i], &ys[i], &zs[i]);
     }
     }


  fscanf(file, "%d", &polygonCount);
  int a, b, c;

  for(int i = 0; i < polygonCount; i++){
    fscanf(file, "%d", polygonPointCount[i]);

    for(int j = 0; j < polygonPointCount[i]; j++) {
      fscanf(file, "%d", &a);

      polygonXs[index][i][j] = xs[a];
      polygonYs[index][i][j] = ys[a];
      polygonZs[index][i][j] = zs[a];

    
    }
  }


  polygonXs[index][polygonCount][0] = 0;
  polygonYs[index][polygonCount][0] = 0;
  polygonZs[index][polygonCount][0] = 0;

}
  //--------------------------------------------------------------------------
void drawPoly(int index){
    
    //calculating x bar and y bar for display
    double finalX[polygonCount[index][8]];
    double finalY[polygonCount[index][8]];

    for(int i = 0; i < polygonCount[index]; i++) {
	for(int j = 0; j < polygonPointCount[i]; j++){
	    finalX[i][j] = 
	    finalY[i][j] = 
}
}

    for(int i = 0; i < polygonCount[index]; i++) {

	G_draw_polygon(polygonXs[index][i], polygonsYs[index][i], )
	    
    }
}	

int main(int argc, char **argv) {
  // Define screen width and height
  double sWidth = 800;
  double sHeight = 800;
  G_init_graphics(sWidth, sHeight);

  int numeralOneKeyCode = 49;
  int keyCode = numeralOneKeyCode;

  for(int i = 1; i < argc; i++)  {
    collectPoints(argv[i], i);
    
    q = G_wait_key();

  }

  while(1){

    if(q = 




  }

} 
