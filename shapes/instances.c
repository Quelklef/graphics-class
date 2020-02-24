#ifndef instances_c_IMPORTED
#define instances_c_IMPORTED

#include "../xwd/xwd_tools.c"

// Example instances of shapes

// == Polyhedral sphere == //

v3 sphere_parameterization_1(float t, float s) {
  const float x = cos(t) * sin(s);
  const float y = cos(s);
  const float z = sin(t) * sin(s);

  return (v3) { x, y, z };
}

Figure *polyhedral_sphere_1() {
  Figure *sphere = Figure_from_Polyhedron(Polyhedron_from_parametric(
    sphere_parameterization_1,
    0, 2 * M_PI, 50,
    1,
    0, 2 * M_PI, 50,
    1
  ));

  nicely_place_figure(sphere);
  return sphere;
}


// == Alternatively parameterized polyhedral sphere == //

v3 sphere_parameterization_2(float t, float s) {
  const float r = sqrt(1 - s * s);

  const float x = r * cos(t);
  const float y = s;
  const float z = r * sin(t);

  return (v3) { x, y, z };
}

v3 const_purple(float t, float s) {
  return (v3) { .8, .5, .8 };
}

Figure *polyhedral_sphere_2() {
  Figure *sphere = Figure_from_Polyhedron(Polyhedron_from_parametric(
    sphere_parameterization_2,
    0, 2 * M_PI, 50,
    1,
    -1, 1, 50,
    0
  ));

  nicely_place_figure(sphere);
  return sphere;
}


// == Interector sphere == //

int sphere_intersect(v3 *result, Line *line) {
  const v3 d = Line_vector(line);
  const v3 s = line->p0;

  const float A = pow(d[0], 2) + pow(d[1], 2) + pow(d[2], 2);
  const float B = 2 * (s[0] * d[0] + s[1] * d[1] + s[2] * d[2]);
  const float C = pow(s[0], 2) + pow(s[1], 2) + pow(s[2], 2) - 1;

  const float t1 = (-B + sqrt(pow(B, 2) - 4 * A * C)) / (2 * A);
  const float t2 = (-B - sqrt(pow(B, 2) - 4 * A * C)) / (2 * A);

  if (isnan(t1) && isnan(t2)) {
    return 0;
  }

  const v3 p1 = s + t1 * d;
  const v3 p2 = s + t2 * d;

  if (isnan(t1) && !isnan(t2)) {
    *result = p1;
    return 1;
  }

  if (isnan(t2) && !isnan(t1)) {
    *result = p2;
    return 1;
  }

  if (p1[2] < p2[2]) {
    *result = p1;
    return 1;
  } else {
    *result = p2;
    return 1;
  }
}

Figure *intersector_sphere() {
  Figure *sphere = Figure_from_Intersector(Intersector_new(
    &sphere_intersect,
    (v3) { -1, -1, -1 },
    (v3) { 1, 1, 1 }
  ));

  nicely_place_figure(sphere);
  return sphere;
}


// == Vase thing == //

v3 vase_parameterization(float t, float s) {
  const float r = sqrt(1 + s * s);

  const float x = r * cos(t);
  const float y = s;
  const float z = r * sin(t);

  return (v3) { x, y, z };
}

Figure *vase() {
  Figure *vase = Figure_from_Polyhedron(Polyhedron_from_parametric(
    vase_parameterization,
    0, 2 * M_PI, 25,
    1,
    -2, 2, 25,
    0
  ));

  nicely_place_figure(vase);
  return vase;
}


// == Mandelbrot on a sphere == //

v3 mandel_sphere_parameterization(float t, float s) {
  const float x = cos(t) * sin(s);
  const float y = cos(s);
  const float z = sin(t) * sin(s);
  return (v3) { x, y, z };
}


v3 mandel_img_parameterization(float u, float v) {
  static int img_initialized = 0;

  static int id;
  static int width;
  static int height;

  if (!img_initialized) {
    id = init_xwd_map_from_file("xwd/mandelbrot.xwd");
    int dims[2];
    get_xwd_map_dimensions(id, dims);
    width = dims[0];
    height = dims[1];
    img_initialized = 1;
  }

  u = u / (2 * M_PI);
  v = v / (2 * M_PI);

  const int x = (int) floor(u * width);
  const int y = (int) floor(v * height);
  double rgb[3];
  get_xwd_map_color(id, x, y, rgb);

  return (v3) { rgb[0], rgb[1], rgb[2] };
}

Figure *mandelbrot() {
  Figure *mandelbrot = Figure_from_Lattice(Lattice_from_parametric(
    mandel_sphere_parameterization,
    mandel_img_parameterization,
    0, 2 * M_PI, 300, 1,
    0, 2 * M_PI, 300, 1
  ));

  nicely_place_figure(mandelbrot);
  return mandelbrot;
}


// == Lookup table == //

Figure *figure_instance_lookup(const char *key) {
       if (strcmp(key, "polysphere_1") == 0) return polyhedral_sphere_1();
  else if (strcmp(key, "polysphere_2") == 0) return polyhedral_sphere_2();
  else if (strcmp(key, "isphere"     ) == 0) return intersector_sphere();
  else if (strcmp(key, "vase"        ) == 0) return vase();
  else if (strcmp(key, "mandelbrot"  ) == 0) return mandelbrot();
  return NULL;
}


#endif // instances_c_IMPORTED
