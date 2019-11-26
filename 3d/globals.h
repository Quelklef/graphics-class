#ifndef globals_h_INCLUDED
#define globals_h_INCLUDED

#include <math.h>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 800;

#define ENOUGH 5000

double HALF_ANGLE                = M_PI / 8;

int    DO_WIREFRAME              = 1;
int    DO_BACKFACE_ELIMINATION   = 0;
int    DO_HALO                   = 1;

int    BACKFACE_ELIMINATION_SIGN = 1;

int    DO_POLY_FILL              = 1;
int    DO_LIGHT_MODEL            = 1;

double AMBIENT                   = 0.4;
double DIFFUSE_MAX               = 0.3;
int    SPECULAR_POWER            = 50;

#endif // globals_h_INCLUDED

