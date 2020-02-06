#ifndef globals_h_INCLUDED
#define globals_h_INCLUDED

#include <math.h>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 800;

#define ENOUGH 15000

#define DEGREES(deg) ((float) (deg) / 180 * M_PI)

float HALF_ANGLE                = DEGREES(30);

int   DO_WIREFRAME              = 1;
int   DO_BACKFACE_ELIMINATION   = 0;
int   DO_HALO                   = 0;
int   DO_CLIPPING               = 1;

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

#endif // globals_h_INCLUDED

