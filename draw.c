#ifndef draw_c_INCLUDED
#define draw_c_INCLUDED

// Low-level 2d drawing functions

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

  if (x < 0 || x > SCREEN_WIDTH || y < 0 || y > SCREEN_HEIGHT) {
    return;
  }

  // Overwrite on z == zbuf[x][y] so that things
  // can be given explicit priority by being drawn later 
  if (z <= zbuf[x][y]) {
    G_point(x, y);
    zbuf[x][y] = z;
  }
}

v2 pixel_coords(const v3 point) {
  /* Find the pixel coordinates on the screen of a given
   * (x, y, z) point. */

  const float x_bar = point[0] / point[2];
  const float y_bar = point[1] / point[2];

  // Scale with respect to only width OR height, because
  // scaling with respect to both will deform the object
  // by stretching it.
  const float minor = fmin(SCREEN_WIDTH, SCREEN_HEIGHT);

  const float H = tan(HALF_ANGLE);
  const float x_bar_bar = x_bar / H * (minor / 2);
  const float y_bar_bar = y_bar / H * (minor / 2);

  const float result_x = x_bar_bar + minor / 2;
  const float result_y = y_bar_bar + minor / 2;

  return (v2) { result_x, result_y };
}

v3 pixel_coords_inv(const v2 pixel, const float z) {
  /* Find the point corresponding to a given pixel with
   * a given z-value */

  // Derived directly by inverting the definition of pixel_coords

  const float m = fmin(SCREEN_WIDTH, SCREEN_HEIGHT) / 2;
  const float H = tan(HALF_ANGLE);

  const float x = (pixel[0] - m) * (z * H / m);
  const float y = (pixel[1] - m) * (z * H / m);

  return (v3) { x, y, z };
}

#endif // draw_c_INCLUDED
