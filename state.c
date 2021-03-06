#ifndef globals_c_INCLUDED
#define globals_c_INCLUDED

#include <math.h>
#include "shapes/figure.c"

#define DEGREES(deg) ((float) (deg) / 180 * M_PI)
#define min(x, y) ((x) < (y) ? (x) : (y))


// == Rendering Parameters == //

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 800;

float HALF_ANGLE                = DEGREES(30);

int   DO_WIREFRAME              = 1;
int   DO_BACKFACE_ELIMINATION   = 0;
int   DO_HALO                   = 1;
int   DO_CLIPPING               = 1;
int   DO_BOUNDING_BOXES         = 0;

int   BACKFACE_ELIMINATION_SIGN = 1;

int   DO_POLY_FILL              = 1;
int   DO_LIGHT_MODEL            = 1;

float AMBIENT                   = 0.4;
float DIFFUSE_MAX               = 0.3;
int   SPECULAR_POWER            = 50;

// Threshold for which objects too close to the observer aren't shown
float HITHER                    = 1;
//Threshold for which objects too far from the observer aren't shown
float YON                       = 30;


// == Current World State == //

#include "util/dyn.c"
DYN_INIT(FigureList, Figure*);

// All the figures in the world
FigureList *figures;
// The light source
Figure *light_source = NULL;
// The currently focused figure
Figure *focused_figure = NULL;

// The observer
Observer *observer;
Figure *observer_figure;


#endif // globals_c_INCLUDED

