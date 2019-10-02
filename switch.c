#include <stdio.h>
#include <FPT.h>

int main(int argc, char **argv) {
  // Define screen width and height
  double sWidth = 800;
  double sHeight = 800;
  G_init_graphics(sWidth, sHeight);

  int q = 49;
  while(q != 48) {
    G_rgb(1, 1, 1);
    G_clear();

    FILE *fp = fopen(argv[q - 48], "r");

    if(fp == NULL) {
      printf("Can not open file: %s\n", argv[2]);
      exit(0);
    }

    int np;  // Number of points in polygon
    fscanf(fp, "%d", &np);
    // Initialize (x, y) arrays to store points
    double x[np];
    double y[np];

    double maxX = -4000;
    double maxY = -4000;
    double minX = 4000;
    double minY = 4000;

    for (int i = 0; i < np; i++){
      fscanf(fp,"%lf %lf" , &x[i], &y[i]);

      if (x[i] > maxX) {
        maxX = x[i];
      } else if (x[i] < minX) {
        minX = x[i];
      }

      if (y[i] > maxY) {
        maxY = y[i];
       } else if (y[i] < minY) {
         minY = y[i];
       }
    }

    printf("%lf %lf %lf %lf\n", maxX, minX, maxY, minY);
    double midX = (maxX + minX) / 2;
    double midY = (maxY + minY) / 2;
    double var;

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
    for (int i = 0; i < np; i++) {
      x[i] *= scaleFactor;
      y[i] *= scaleFactor;
    }

    // Distance from center of poly to center of screen
    double diffX = (midX * scaleFactor - sWidth / 2);
    double diffY = (midY * scaleFactor - sHeight / 2);

    printf("%lf %lf \n", diffX, diffY);
    // Move polygon to center of screen
    for (int i = 0; i < np; i++) {
      x[i] -= diffX;
      y[i] -= diffY;
    }

    int numpoly;
    fscanf(fp, "%d" , &numpoly);

    printf("hi\n");

    // Array of the number of points in each polygon
    int polygonpoint[100];
    // Array of the actual points in each polygon
    double polygons_x[100][100];
    double polygons_y[100][100];

    // Populate these arrays
    for (int i = 0; i < numpoly; i++) {
      fscanf(fp, "%d", &polygonpoint[i]);
      for (int j = 0; j < polygonpoint[i]; j++) {
        int poly_idx;
        fscanf(fp, "%d", &poly_idx);
        polygons_x[i][j] = x[poly_idx];
        polygons_y[i][j] = y[poly_idx];
      }
    }

    // Color the polygon
    printf("bill\n");
    double colors[numpoly][3];
    for (int i = 0; i < numpoly; i++) {
      double r, g, b;
      fscanf(fp, "%lf %lf %lf", &r, &g, &b);

      G_rgb(r, g, b);
      colors[i][0] = r;
      colors[i][1] = g;
      colors[i][2] = b;
      G_fill_polygon(polygons_x[i], polygons_y[i], polygonpoint[i]);
    }

    q =  G_wait_key();
  }

}


