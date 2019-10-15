#include <FPT.h>
#include <float.h>

#include "point_vec.h"
#include "point_set.h"

void my_fill_polygon(double *xs, double *ys, int n);
PointSet *get_me_points_inside_the_polygon(double *xs, double *ys, int n);
void flood_fill_that_motherfucker(double *xs, double *ys, int n, PointSet *points);

void G_draw_point(double x, double y) {
  G_fill_rectangle(x - .5, y - .5, 1, 1);
}

void make_poly(double *xs, double *ys, int *n, int blrw, int blrh) {
   while (1) {
      double click[2];
      G_wait_click(click);

      if (click[0] < blrw && click[1] < blrh) {
         // Finish polygon
         if (*n > 0) G_line(xs[*n-1], ys[*n-1], xs[0], ys[0]);
         break;
      } else {
         // Another vertex
         xs[*n] = click[0];
         ys[*n] = click[1];
         G_fill_rectangle(click[0] - 2, click[1] - 2, 4, 4);
         if (*n > 0) G_line(xs[*n-1], ys[*n-1], click[0], click[1]);
         *n += 1;
      }
   }
}

void main() {
  G_init_graphics(800, 800);

  int blrw = 800;
  int blrh = 20;

  G_rgb(0, 0, 0);
  G_clear();

  G_rgb(1, 0, 0);
  G_fill_rectangle(0, 0, blrw, blrh);

  // Vertex array array
  double **xs = malloc(100 * sizeof(double*));
  double **ys = malloc(100 * sizeof(double*));
  int *n = malloc(100 * sizeof(int));
  int k = 0;

  while (1) {
    xs[k] = malloc(100 * sizeof(double));
    ys[k] = malloc(100 * sizeof(double));

    n[k] = 0;
    G_rgb(1, 1, 1);
    make_poly(xs[k], ys[k], &n[k], blrw, blrh);
    my_fill_polygon(xs[k], ys[k], n[k]);
    k += 1;
  }

}

void my_fill_polygon(double *xs, double *ys, int n) {
  PointSet *points = get_me_points_inside_the_polygon(xs, ys, n);
  flood_fill_that_motherfucker(xs, ys, n, points);
  PointSet_destroy(points);
}

double *get_elbow(double *xs, double *ys, int n, int i) {
  // Get the previous point
  double prev[2] = { xs[modulus(i - 1, n)],
                     ys[modulus(i - 1, n)] };

  // Get the current point
  double current[2] = {xs[i], ys[i]};

  // Get the next point
  double next[2] = { xs[modulus(i + 1, n)],
                     ys[modulus(i + 1, n)] };

  // Get the vector from the current point to the previous point
  double current_to_prev[2];
  vec_from_points(current, prev, current_to_prev);

  // Get the vector from the current point to the next point
  double current_to_next[2];
  vec_from_points(current, next, current_to_next);

  // Create the vector whose angle is halfway between them, rotated around the current point
  double *midvec = malloc(sizeof(double));
  vec_midway(current_to_prev, current_to_next, midvec);

  return midvec;
}

int get_orientation(double *xs, double *ys, int n) {
  /* The orientation is defined to be +1 if the elbow of the 0th and 1st points
   * points towards the inside of the polygon; otherwise, it's -1.
   */

  for (int i = 0; i < n; i++) {
    double current[2] = { xs[i], ys[i] };
    double *elbow = get_elbow(xs, ys, n, i);

    // Keep track of whether the intersections followed
    // the direction of midvec, or went the other way
    int forward_intersection_count = 0;
    int backward_intersection_count = 0;

    for (int j = 0; j < n; j++) {
      // Skip lines containing this point
      if (j == i || j + 1 == i) continue;

      double start[2] = { xs[j], ys[j] };
      double end[2] = { xs[modulus(j + 1, n)], ys[modulus(j + 1, n)] };
      double d[2] = { end[0] - start[0], end[1] - start[1] };

      // A vect for line is "START + t * MOVEMENT"
      // This array will contain the solution values for these ts
      double soln_ts[2];
      // And this will be the actual intersection point
      double intersection[2];
      // Return value is whether or not an intersection was actually found
      int found_intersection = find_lines_intersection(
        start, d,
        current, elbow,
        soln_ts, intersection);

      // If we got solutions
      if (found_intersection) {
        // Ensure within line SEGMENT
        if (0 <= soln_ts[1] && soln_ts[1] <= 1) {
          if (soln_ts[0] >= 0) {
            forward_intersection_count++;
          } else {
            backward_intersection_count++;
          }
        }
      }

    }

    // if there were ONLY intersections in ONE direction, we know that
    // direcetion is the direction of 'inside'.
    // This is because there will ALWAYS be intersections in the direction of 'inside',
    // but there MAY ALSO be intersections in the direction of outside.
    if ((backward_intersection_count == 0 && forward_intersection_count > 0)) {
      return 1;
    }
    if ((forward_intersection_count == 0 && backward_intersection_count > 0)) {
      return -1;
    }

    // Loop to next point
  }
}

PointSet *get_me_points_inside_the_polygon(double *xs, double *ys, int n) {
  /* Get a bunch of points in the polygon, one for each vertext,
   * such that there is one point in every simple subpolygon
   */

   PointSet *points = PointSet_create();

   int orientation = get_orientation(xs, ys, n);

   for (int i = 0; i < n; i++) {
     double point[2] = { xs[i], ys[i] };
     double *elbow = get_elbow(xs, ys, n, i);

     elbow[0] *= orientation;
     elbow[1] *= orientation;

     double closest_intersection[2];
     double closest_intersection_dist = DBL_MAX;

     for (int j = 0; j < n; j++) {
       if (j == i || j + 1 == i) continue;

       double start[2] = { xs[j], ys[j] };
       double end[2] = { xs[modulus(j + 1, n)], ys[modulus(j + 1, n)] };
       double d[2] = { end[0] - start[0], end[1] - start[1] };

       double soln_ts[2];
       double intersection[2];
       int found_intersection = find_lines_intersection(
         start, d,
         point, elbow,
         soln_ts, intersection);

       if (found_intersection) {
         if (0 <= soln_ts[1] && soln_ts[1] <= 1) {
           double distance = dist(point, intersection);
           if (distance < closest_intersection_dist) {
             closest_intersection_dist = distance;
             closest_intersection[0] = intersection[0];
             closest_intersection[1] = intersection[1];
           }
         }
       }
     }

     PointSet_put(points, closest_intersection);
  }

  return points;
}

double dist_to_segment(double start[2], double end[2], double point[2]) {
  // Distance from `point` to segment from `start` to `end`

  if (point_eq(start, end)) {
    return dist(start, point);
  }

  // The line is "start + t * (end - start)"

  // Did some math, and the following should work
  // This is the t value of the point closest to `point`
  double t =   (   (point[0] - start[0]) * (end[0] - start[0])
                 + (point[1] - start[1]) * (end[1] - start[1]) )
             / (pow(end[0] - start[0], 2) + pow(end[1] - start[1], 2));

  // Since we want distance to segment, need to know if the 
  // t value designates a point on the segment or not
  if (t <= 0) {  // Before the segment
    // Return distance to start of segment
    return dist(start, point);
  } else if (t >= 1) {  // After the segment
    // Return distance to end of segment
    return dist(end, point);
  } else {
    double closest[2] = { start[0] + t * (end[0] - start[0]),
                          start[1] + t * (end[1] - start[1]) };
    return dist(closest, point);
  }
}

int on_boundary(double *xs, double *ys, int n, double p[2]) {
  // Return whether or not (x, y) is on any boundary of the given polygon
  for (int i = 0; i < n; i++) {
    double current[2] = { xs[i], ys[i] };
    double next[2] = { xs[modulus(i + 1, n)],
                       ys[modulus(i + 1, n)] };

    double dist = dist_to_segment(current, next, p);
    if (dist <= EPSILON) {
      return 1;
    }
  }

  return 0;
}

void flood_fill_that_motherfucker(double *xs, double *ys, int n, PointSet *points) {
  // `current` is hash set of points to draw
  PointSet *current = PointSet_create();
  PointSet_iterate(points, p, ({ PointSet_put(current, p); }));

  // Hash set of points drawn last step
  PointSet *completed_last_step = PointSet_create();
  PointSet *completed_two_steps_ago = PointSet_create();

  // Hash set of points drawn next
  // (This is used only as a temp. structure)
  PointSet *todo = PointSet_create();

  while (!PointSet_empty(current)) {

    PointSet_iterate(current, point, ({
      G_draw_point(point[0], point[1]);

      if (!on_boundary(xs, ys, n, point)) {
        double neighbors[4][2] = {
          { point[0] - 1, point[1] },
          { point[0] + 1, point[1] },
          { point[0], point[1] - 1 },
          { point[0], point[1] + 1 },
        };

        for (int k = 0; k < 4; k++) {
          if (!PointSet_contains(completed_last_step, neighbors[k])
              && !PointSet_contains(completed_two_steps_ago, neighbors[k])) {
            PointSet_put(todo, neighbors[k]);
          }
        }
      }
    }));

    // Make it nicely animated
    G_display_image();
    usleep(5000);

    PointSet_destroy(completed_two_steps_ago);
    completed_two_steps_ago = completed_last_step;
    completed_last_step = current;
    current = todo;
    todo = PointSet_create();

  }

  PointSet_destroy(completed_last_step);
  PointSet_destroy(completed_two_steps_ago);
  PointSet_destroy(todo);
}

