#include <stdio.h>
#include <FPT.h>
#include <float.h>

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

void displayPolygon(char *filename, double sWidth, double sHeight) {
  G_rgb(1, 1, 1);
  G_clear();

  FILE *file = fopen(filename, "r");

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
  for (int i = 0; i < pointCount; i++) {
    xs[i] *= scaleFactor;
    ys[i] *= scaleFactor;
  }

  // Distance from center of poly to center of screen
  double diffX = (midX * scaleFactor - sWidth / 2);
  double diffY = (midY * scaleFactor - sHeight / 2);

  // Move polygon to center of screen
  for (int i = 0; i < pointCount; i++) {
    xs[i] -= diffX;
    ys[i] -= diffY;
  }

  int polygonCount;
  fscanf(file, "%d", &polygonCount);

  // Array of the number of points in each polygon
  int polygonPointCounts[100];
  // Array of the actual points in each polygon
  double polygonXs[100][100];
  double polygonYs[100][100];

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

  // Color & display the polygon
  for (int i = 0; i < polygonCount; i++) {
    double r, g, b;
    fscanf(file, "%lf %lf %lf", &r, &g, &b);

    G_rgb(r, g, b);
    G_fill_polygon(polygonXs[i], polygonYs[i], polygonPointCounts[i]);
  }
}

int main(int argc, char **argv) {
  // Define screen width and height
  double sWidth = 800;
  double sHeight = 800;
  G_init_graphics(sWidth, sHeight);

  int numeralOneKeyCode = 49;
  int keyCode = numeralOneKeyCode;
  while (1) {
    displayPolygon(argv[1 + keyCode - numeralOneKeyCode], sWidth, sHeight);
    keyCode = G_wait_key();
  }
}
