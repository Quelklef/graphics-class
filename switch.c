#include <stdio.h>
#include <FPT.h>
#include <float.h>

#include "M2d_matrix_tools.c"

double oldRot[3][3];
int polygonCount;
double storageMat[3][3];

// Array of the actual points in each polygon
double polygonXs[10000][10000];
double polygonYs[10000][10000];

// Array of the number of points in each polygon
int polygonPointCounts[1000];

//colors


double colors[10000][3];

// Color & display the polygon
FILE *file;
void colorPoly(){  
G_rgb(1,1,1);
G_clear();
  for (int i = 0; i < polygonCount; i++) {
    G_rgb(colors[i][0], colors[i][1], colors[i][2]);
    G_fill_polygon(polygonXs[i], polygonYs[i], polygonPointCounts[i]);
  }
}


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

void displayPolygon(char *filename, double rotAmt, double sWidth, double sHeight) {
  G_rgb(1, 1, 1);
  G_clear();

   file = fopen(filename, "r");

  if (file == NULL) {
    printf("Can not open file: %s\n", filename);
    exit(0);
  }

  int pointCount;  // Number of points in polygon
  fscanf(file, "%d", &pointCount);

  // Initialize (x, y) arrays to store points
  double xs[pointCount];
  double ys[pointCount];

  for (int i = 0; i < pointCount; i++) {
     fscanf(file, "%lf %lf", &xs[i], &ys[i]);
  }

  // Find the bounding box
  double minX, maxX, minY, maxY;
  arrayMin(xs, pointCount, &minX);
  arrayMax(xs, pointCount, &maxX);
  arrayMin(ys, pointCount, &minY);
  arrayMax(ys, pointCount, &maxY);

  double midX = (maxX + minX) / 2;
  double midY = (maxY + minY) / 2;

  double width = maxX - minX;
  double height = maxY - minY;

  // How much of the screen should the polygon take up
  // (slightly more complicated than that)
  double relPolygonSize = 0.6;

  double scaleFactor;
  if (width > height) {
    scaleFactor = relPolygonSize * sWidth / width;
  } else {
    scaleFactor = relPolygonSize * sHeight / height;
  }

  // Scale the polygon
  /*
  for (int i = 0; i < pointCount; i++) {
    xs[i] *= scaleFactor;
    ys[i] *= scaleFactor;
  }
  */
  //Scaling with Matrices
  double scaleMatrix[3][3];
  M2d_make_scaling(scaleMatrix, scaleFactor, scaleFactor);

  // Rotate the polygon
  double rotMat[3][3];
  M2d_make_rotation_radians(rotMat, rotAmt);

  //M2d_mat_mult_points(xs, ys, rotMat, xs, ys, pointCount); 

  // Distance from center of poly to center of screen
  double diffX = (midX * scaleFactor - sWidth / 2);
  double diffY = (midY * scaleFactor - sHeight / 2);

  // Move polygon to center of screen
  /*
  for (int i = 0; i < pointCount; i++) {
    xs[i] -= diffX;
    ys[i] -= diffY;
  }
  */
  double centerMat[3][3];
  M2d_make_translation(centerMat, diffX, diffY);

  fscanf(file, "%d", &polygonCount);


  // Populate these arrays
  for (int i = 0; i < polygonCount; i++) {
    fscanf(file, "%d", &polygonPointCounts[i]);
    for (int j = 0; j < polygonPointCounts[i]; j++) {
      int pointIdx;
      fscanf(file, "%d", &pointIdx);
      polygonXs[i][j] = xs[pointIdx];
      polygonYs[i][j] = ys[pointIdx];
    }
  }
  
  
  double toCenter[3][3];
  M2d_make_translation(toCenter, sWidth/2, sHeight/2);
  double toOrigin[3][3];
  M2d_make_translation(toOrigin, -midX, -midY);

  //--------------------------------------------------------------------------
  //Matrix Operations here!

  M2d_print_mat(toCenter);
  //M2d_print_mat(rotMat);
  M2d_mat_mult(storageMat, toCenter, scaleMatrix);
  //M2d_print_mat(storageMat);
  M2d_mat_mult(storageMat, storageMat, toOrigin);
  for(int i = 0; i < polygonCount; i++){
    M2d_mat_mult_points(polygonXs[i], polygonYs[i], storageMat, polygonXs[i], polygonYs[i], polygonPointCounts[i]);

  }


  double centerToOrigin[3][3];
  M2d_make_translation(centerToOrigin, -400, -400);
  //Creating General Rotation Matrix
  M2d_mat_mult(oldRot, toCenter, rotMat);
  M2d_mat_mult(oldRot, oldRot, centerToOrigin);

  //M2d_print_mat(toCenter);
 // M2d_print_mat(toOrigin);
  M2d_print_mat(rotMat);
  
  //M2d_print_mat(scaleMatrix);
  //M2d_print_mat(toOrigin);

double r, g, b;
  for (int i = 0; i < polygonCount; i++) {
    fscanf(file, "%lf %lf %lf", &r, &g, &b);
    colors[i][0] = r;
    colors[i][1] = g;
    colors[i][2] = b;
  }

  colorPoly();
}
  //--------------------------------------------------------------------------


int main(int argc, char **argv) {
  // Define screen width and height
  double sWidth = 800;
  double sHeight = 800;
  G_init_graphics(sWidth, sHeight);

  int numeralOneKeyCode = 49;
  int keyCode = numeralOneKeyCode;
  int prevKeyCode = -1;
  double rotAmt = 0.1;

  while (1) {
    if(keyCode != prevKeyCode){ 
	displayPolygon(argv[1 + keyCode - numeralOneKeyCode], rotAmt, sWidth, sHeight);
    }

    if (keyCode == prevKeyCode) {
      printf("yesssssss\n");
      
      for(int i = 0; i < polygonCount; i++){
	printf("here\n");
	      M2d_mat_mult_points(polygonXs[i], polygonYs[i], oldRot, polygonXs[i], polygonYs[i], polygonPointCounts[i]);
} 
       printf("plz\n");
       colorPoly();
       M2d_print_mat(oldRot);

    } else {
      rotAmt = 0.1;
    }
	
    prevKeyCode = keyCode;	
    keyCode = G_wait_key();
  }
}
