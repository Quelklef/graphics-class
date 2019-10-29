#ifndef point_h_INCLUDED
#define point_h_INCLUDED

/* Point and vector helper functions */

void vec_from_points(double p0[2], double p1[2], double v[2]) {
  v[0] = p1[0] - p0[0];
  v[1] = p1[1] - p0[1];
}

void vec_from_r_theta(double r, double theta, double v[2]) {
  v[0] = r * cos(theta);
  v[1] = r * sin(theta);
}

double mag(double vec[2]) {
  return sqrt(pow(vec[0], 2) + pow(vec[1], 2));
}

double dist(double v0[2], double v1[2]) {
  return sqrt( pow(v1[0] - v0[0], 2) + pow(v1[1] - v0[1], 2) );
}

double angle(double v[2]) {
  return atan2(v[1], v[0]);
}

// https://stackoverflow.com/a/19288271/4608364
int modulus(int a, int b) {
  int r = a % b;
  return r < 0 ? r + b : r;
}

void vec_midway(double v0[2], double v1[2], double v[2]) {
  // Set `v` to be the vector midway between `v0` and `v1`,
  // i.e. average magnitude and angle
  double theta = (angle(v0) + angle(v1)) / 2;
  double r = (mag(v0) + mag(v1)) / 2;
  vec_from_r_theta(r, theta, v);
}

int find_lines_intersection(
    double s0[2], double d0[2],
    double s1[2], double d1[2],
    double ts[2], double intersection[2]) {
  // Solve for the intersection of the lines
  // s0 + t0 * d0
  // and
  // s1 + t1 * d1
  // Set ts[0] = solution t0,
  //     ts[1] = solution t1
  // And intersection = the interesection point
  // Return whether or not an intersection was found

  // Did a bunch of math, and got this:

  double denom = d0[1] * d1[0] - d0[0] * d1[1];
  if (denom == 0) {
    return 0;
  }

  ts[0] = (d1[0] * (s1[1] - s0[1]) + d1[1] * (s0[0] - s1[0])) / denom;
  ts[1] = (d0[0] * (s1[1] - s0[1]) + d0[1] * (s0[0] - s1[0])) / denom;
  intersection[0] = s0[0] + ts[0] * d0[0];
  intersection[1] = s0[1] + ts[0] * d0[1];

  return 1;
}

#define EPSILON .5

int point_eq(double p0[2], double p1[2]) {
  // return p0[0] == p1[0] && p0[1] == p1[1];
  double p0_[2] = { round(p0[0]), round(p0[1]) };
  double p1_[2] = { round(p1[0]), round(p1[1]) };
  return dist(p0_, p1_) < EPSILON;
}

#endif // point_h_INCLUDED

