#ifndef draw_c_INCLUDED
#define draw_c_INCLUDED

// Low-level 2d drawing functions

#include <libgfx.h>

#include "../shapes/line.c"

void G_rgbv(const v3 rgb) {
  G_rgb(rgb[0], rgb[1], rgb[2]);
}

void G_pointv(const v2 point) {
  G_point(point[0], point[1]);
}


typedef float Zbuf[SCREEN_WIDTH][SCREEN_HEIGHT];

void zbuf_init(Zbuf zbuf) {
  for (int x = 0; x < SCREEN_WIDTH; x++) {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
      zbuf[x][y] = INFINITY;
    }
  }
}

void zbuf_draw(Zbuf zbuf, const int x, const int y, const float z) {
  if (   x < 0
      || x >= SCREEN_WIDTH
      || y < 0
      || y >= SCREEN_HEIGHT
  ) {
    return;
  }

  // Overwrite on z == zbuf[x][y] so that things
  // can be given explicit priority by being drawn later 
  if (z <= zbuf[x][y]) {
    zbuf[x][y] = z; 
    G_point(x, y);
  }
}

void zbuf_drawv(Zbuf zbuf, const v2 pixel, const float z) {
  zbuf_draw(zbuf, pixel[0], pixel[1], z);
}


// Scale with respect to only width OR height, because
// scaling with respect to both will deform the object
// by stretching it.
const int minor = min(SCREEN_WIDTH, SCREEN_HEIGHT);

const float m = (float) minor / 2;

float H;
float H_over_m;
float m_over_H;
float H_times_m;

v2 pixel_coords(const v3 point) {
  /* Find the pixel coordinates on the screen of a given  (x, y, z) point. */
  const v3 prime = point / point[2] * m_over_H + m;
  return (v2) { prime[0], prime[1] };
}

v3 pixel_coords_inv_z(const v2 pixel, const float z) {
  /* Find the point corresponding to a given pixel with a given z-value */
  // Derived directly by inverting the definition of pixel_coords
  const v2 result = (pixel - m) * z * H_over_m;
  return (v3) { result[0], result[1], z };
}

void pixel_coords_inv_line(Line *result, const v2 pixel) {
  const v3 p0 = pixel_coords_inv_z(pixel, 0);
  const v3 p1 = pixel_coords_inv_z(pixel, 1);
  Line_between(result, p0, p1);
}

Line *pixel_coords_inv_cache[SCREEN_WIDTH][SCREEN_HEIGHT];

Line *pixel_coords_inv(const v2 pixel) {
  const int x = pixel[0];
  const int y = pixel[1];
  return pixel_coords_inv_cache[x][y];
}


void draw_init() {
  // initialize constants
  H = tan(HALF_ANGLE);
  H_over_m = H / m;
  m_over_H = m / H;
  H_times_m = H * m;

  // initialize inv cache
  for (int x = 0; x < SCREEN_WIDTH; x++) {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
      const v2 pixel = { x, y };
      Line *line = malloc(sizeof(Line));
      pixel_coords_inv_line(line, pixel);
      pixel_coords_inv_cache[x][y] = line;
    }
  }
}

void draw_close() {
  for (int x = 0; x < SCREEN_WIDTH; x++) {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
      Line *line = pixel_coords_inv_cache[x][y];
      free(line);
    }
  }
}

#endif // draw_c_INCLUDED
