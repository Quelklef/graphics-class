#ifndef draw_c_INCLUDED
#define draw_c_INCLUDED

// Low-level 2d drawing functions

#include "line.c"

void G_rgbv(const v3 rgb) {
  G_rgb(rgb[0], rgb[1], rgb[2]);
}

void G_pointv(const v2 point) {
  G_point(point[0], point[1]);
}



typedef float Zbuf[SCREEN_WIDTH][SCREEN_HEIGHT];

void zbuf_init(Zbuf zbuf) {
  for (int i = 0; i < SCREEN_WIDTH; i++) {
    for (int j = 0; j < SCREEN_HEIGHT; j++) {
      zbuf[i][j] = INFINITY;
    }
  }
}

void zbuf_draw(Zbuf zbuf, const v2 pixel, const float z) {
  const int x = pixel[0];
  const int y = pixel[1];

  if (   pixel[0] < 0
      || pixel[0] > SCREEN_WIDTH
      || pixel[1] < 0
      || pixel[1] > SCREEN_HEIGHT
  ) {
    return;
  }

  // Overwrite on z == zbuf[x][y] so that things
  // can be given explicit priority by being drawn later 
  if (z <= zbuf[x][y]) {
    G_pointv(pixel);
    zbuf[x][y] = z;
  }
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

void draw_init() {
  H = tan(HALF_ANGLE);
  H_over_m = H / m;
  m_over_H = m / H;
  H_times_m = H * m;
}

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

void pixel_coords_inv(Line *result, const v2 pixel) {
  const v3 p0 = pixel_coords_inv_z(pixel, 0);
  const v3 p1 = pixel_coords_inv_z(pixel, 1);
  Line_between(result, p0, p1);
}

#endif // draw_c_INCLUDED
