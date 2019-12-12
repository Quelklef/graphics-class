#ifndef display_h_INCLUDED
#define display_h_INCLUDED

// Functions relating to displaying stuff

#include <math.h>

#include "misc.h"
#include "poly.h"
#include "model.h"
#include "pointvec.h"
#include "plane.h"

void pixel_coords_M(double *result_x, double *result_y, const Point *point) {
  /* Find the pixel coordinates on the screen of a given
   * (x, y, z) point. */

  const double x_bar = point->x / point->z;
  const double y_bar = point->y / point->z;

  // Scale with respect to only width OR height, because
  // scaling with respect to both will deform the object
  // by stretching it.
  const double minor = fmin(SCREEN_WIDTH, SCREEN_HEIGHT);

  const double H = tan(HALF_ANGLE);
  const double x_bar_bar = x_bar / H * (minor / 2);
  const double y_bar_bar = y_bar / H * (minor / 2);

  *result_x = x_bar_bar + minor / 2;
  *result_y = y_bar_bar + minor / 2;
}

void Line_display(const Line *line) {
  double pixel_x0, pixel_y0, pixel_xf, pixel_yf;
  pixel_coords_M(&pixel_x0, &pixel_y0, &line->p0);
  pixel_coords_M(&pixel_xf, &pixel_yf, &line->pf);

  G_line(pixel_x0, pixel_y0, pixel_xf, pixel_yf);
}

int shouldnt_display(const Poly *poly) {
  // Implements backface elimination

  if (!DO_BACKFACE_ELIMINATION) return 0;

  Point origin;
  PointVec_init(&origin, 0, 0, 0);

  Vec T;
  PointVec_between_M(&T, &origin, poly->points[0]);

  Vec N;
  Poly_normal_M(&N, poly);

  return BACKFACE_ELIMINATION_SIGN * PointVec_dot(&T, &N) < 0;
}

double Poly_calc_intensity(const Poly *poly, const Point *light_source_loc) {
  Point poly_center;
  Poly_calc_center_M(&poly_center, poly);

  Vec poly_normal;
  Poly_normal_M(&poly_normal, poly);

  Vec to_light;
  PointVec_between_M(&to_light, &poly_center, light_source_loc);
  PointVec_normalize(&to_light);

  // Make sure normal is going correct direction
  if (PointVec_dot(&poly_normal, &to_light) < 0) {
    PointVec_negate(&poly_normal);
  }

  const double cos_alpha = PointVec_dot(&poly_normal, &to_light);
  const double cos_alpha_0 = fmax(0, cos_alpha);

  Point observer;
  PointVec_init(&observer, 0, 0, 0);

  Vec to_observer;
  PointVec_between_M(&to_observer, &poly_center, &observer);
  PointVec_normalize(&to_observer);

  // If observer is on other side of polygon, no reflected light is seen.
  const double A = PointVec_dot(&poly_normal, &to_light);
  const double B = PointVec_dot(&poly_normal, &to_observer);
  if (sgn(A) != sgn(B)) {
    return AMBIENT;
  }

  Vec scaled_normal;
  PointVec_scale_M(&scaled_normal, &poly_normal, 2 * cos_alpha);

  Vec reflection;
  PointVec_negate_M(&reflection, &to_light);
  PointVec_add(&reflection, &scaled_normal);

  const double cos_beta = PointVec_dot(&reflection, &to_observer);
  const double cos_beta_0 = fmax(0, cos_beta);

  return AMBIENT
         + DIFFUSE_MAX * cos_alpha_0
         + (1 - AMBIENT - DIFFUSE_MAX) * pow(cos_beta_0, SPECULAR_POWER);
}

void Poly_calc_color_M(
      double *result_r, double *result_g, double *result_b, 
      const Poly *poly, const Point *light_source_loc,
      const double inherent_r, const double inherent_g, const double inherent_b
    ) {

  const double intensity = Poly_calc_intensity(poly, light_source_loc);
  const double full = AMBIENT + DIFFUSE_MAX;

  if (intensity == full) {
    *result_r = inherent_r;
    *result_g = inherent_g;
    *result_b = inherent_b;
    return;
  }

  if (intensity > full) {
    const double ratio = (intensity - full) / (1 - full);
    *result_r = inherent_r + (1 - inherent_r) * ratio;
    *result_g = inherent_g + (1 - inherent_g) * ratio;
    *result_b = inherent_b + (1 - inherent_b) * ratio;
    return;
  }

  if (intensity < full) {
    const double ratio = intensity / full;
    *result_r = inherent_r * ratio;
    *result_g = inherent_g * ratio;
    *result_b = inherent_b * ratio;
    return;
  }
}

void Poly_display_minimal(const Poly *poly) {
  double pxs[poly->point_count];
  double pys[poly->point_count];
  for (int point_idx = 0; point_idx < poly->point_count; point_idx++) {
    const Point *p = poly->points[point_idx];
    pixel_coords_M(&pxs[point_idx], &pys[point_idx], p);
  }

  G_fill_polygon(pxs, pys, poly->point_count);
}

void Poly_display_as_halo(const Poly *poly) {
  const double scale_amt = 3;

  double pxs[poly->point_count];
  double pys[poly->point_count];
  for (int point_idx = 0; point_idx < poly->point_count; point_idx++) {
    const Point *p = poly->points[point_idx];
    pixel_coords_M(&pxs[point_idx], &pys[point_idx], p);
  }

  double center_px, center_py;
  {
    double min_px = +DBL_MAX;
    double max_px = -DBL_MAX;
    double min_py = +DBL_MAX;
    double max_py = -DBL_MAX;

    for (int point_idx = 0; point_idx < poly->point_count; point_idx++) {
      double px = pxs[point_idx];
      double py = pys[point_idx];

      if (px < min_px) min_px = px;
      if (px > max_px) max_px = px;
      if (py < min_py) min_py = py;
      if (py > max_py) max_py = py;
    }

    center_px = min_px / 2 + max_px / 2;
    center_py = min_py / 2 + max_py / 2;
  }

  for (int point_idx = 0; point_idx < poly->point_count; point_idx++) {
    pxs[point_idx] = (pxs[point_idx] - center_px) * scale_amt + center_px;
    pys[point_idx] = (pys[point_idx] - center_py) * scale_amt + center_py;
  }

  G_rgb(1, 0, 0);
  G_fill_polygon(pxs, pys, poly->point_count);
}

void Poly_clip_with_plane(Poly *poly, const Plane *plane) {
  // Clip a polygon with a plane
  // The portion of the shape on the on the same side as the point
  //   (0, 0, (YON + HITHER)/2) will be kept
  //   (given that YON > HITHER)
  // Frees removed points

  // If HITHER >= YON, then all models are entirely clipped
  if (HITHER >= YON) {
    Poly_destroy(poly);
    const Poly *new = Poly_new();
    *poly = *new;
    return;
  }

  Poly result_poly;
  Poly_init(&result_poly);

  // Create a point that's definitely inside the clipping region
  Point inside_point;
  PointVec_init(&inside_point, 0, 0, YON * 0.5 + HITHER * 0.5);
  int inside_side = Plane_side_of(plane, &inside_point);

  for (int point_idx = 0; point_idx < poly->point_count; point_idx++) {
    Point *this_point = poly->points[point_idx];
    Point *next_point = poly->points[(point_idx + 1) % poly->point_count];

    const int this_is_inside = inside_side == Plane_side_of(plane, this_point);
    const int next_is_inside = inside_side == Plane_side_of(plane, next_point);

    Line this_to_next;
    Line_between(&this_to_next, this_point, next_point);

    if (this_is_inside) {
      Poly_add_point(&result_poly, this_point);
    }

    // If crosses over, add itersection
    if (this_is_inside != next_is_inside) {
      Point *intersection = malloc(sizeof(Point));
      const int found_intersection = Plane_intersect_line_M(intersection, plane, &this_to_next);
      // We know there should be an intersection since the points are on opposite sides of the plane
      if (!found_intersection) {
        printf("Error in Poly_clip_with_plane: intersection not found.\n");
        exit(1);
      }

      Poly_add_point(&result_poly, intersection);
    }

  }

  // Clean up:

  // Destroy unused points
  for (int point_idx = 0; point_idx < poly->point_count; point_idx++) {
    Point *point = poly->points[point_idx];
    const int is_inside = inside_side == Plane_side_of(plane, point);
    if (!is_inside) {
      PointVec_destroy(point);
    }
  }

  *poly = result_poly;
}

void Poly_clip(Poly *poly) {
  // Clip a polygon according to HALF_ANGLE, YON, and HITHER
  // Frees removed points

  Point observer;
  PointVec_init(&observer, 0, 0, 0);

  const double tha = tan(HALF_ANGLE);

  // --

  Point screen_top_left;
  PointVec_init(&screen_top_left, -tha, tha, 1);

  Point screen_top_right;
  PointVec_init(&screen_top_right, tha, tha, 1);

  Point screen_bottom_left;
  PointVec_init(&screen_bottom_left, -tha, -tha, 1);

  Point screen_bottom_right;
  PointVec_init(&screen_bottom_right, tha, -tha, 1);

  // --

  Plane above_plane;
  Plane_from_points(&above_plane, &observer, &screen_top_left, &screen_top_right);

  Plane left_plane;
  Plane_from_points(&left_plane, &observer, &screen_top_left, &screen_bottom_left);

  Plane bottom_plane;
  Plane_from_points(&bottom_plane, &observer, &screen_bottom_left, &screen_bottom_right);

  Plane right_plane;
  Plane_from_points(&right_plane, &observer, &screen_bottom_right, &screen_top_right);

  Plane hither_plane;
  {
    Point p0;
    PointVec_init(&p0, 0, 0, HITHER);

    Vec normal;
    PointVec_init(&normal, 0, 0, 1);

    Plane_from_point_normal(&hither_plane, &p0, &normal);
  }

  Plane yon_plane;
  {
    Point p0;
    PointVec_init(&p0, 0, 0, YON);

    Vec normal;
    PointVec_init(&normal, 0, 0, -1);

    Plane_from_point_normal(&yon_plane, &p0, &normal);
  }

  // --

  Poly_clip_with_plane(poly, &above_plane);
  Poly_clip_with_plane(poly, &left_plane);
  Poly_clip_with_plane(poly, &bottom_plane);
  Poly_clip_with_plane(poly, &right_plane);
  Poly_clip_with_plane(poly, &hither_plane);
  Poly_clip_with_plane(poly, &yon_plane);
}

void Poly_display(const Poly *poly, const int is_focused, const int is_halo, const Point *light_source_loc) {
  // focused: is the polygon part of the focused model? (NOT part of the halo)
  // halo: is the polygon part of a halo?

  Poly *clipped = Poly_clone(poly);
  if (DO_CLIPPING) {
    Poly_clip(clipped);
  }

  if (clipped->point_count != 0) {
    // Only display if there are points
    // (Some display subroutines require a minimum point count)

    if (DO_POLY_FILL) {
      if (is_halo) {
        Poly_display_as_halo(clipped);
      } else {
        double r = 0.8;
        double g = 0.5;
        double b = 0.8;

        if (DO_LIGHT_MODEL) {
          Poly_calc_color_M(
            &r, &g, &b,
            clipped, light_source_loc,
            r, g, b);
        }

        G_rgb(r, g, b);
        Poly_display_minimal(clipped);
      }
    }

    if (DO_WIREFRAME && !is_halo) {
      if ((!DO_POLY_FILL || !DO_HALO) && is_focused) {
        G_rgb(1, 0, 0);
      } else {
        G_rgb(.3, .3, .3);
      }

      for (int point_idx = 0; point_idx < clipped->point_count; point_idx++) {
        const Point *p0 = clipped->points[point_idx];
        const Point *pf = clipped->points[(point_idx + 1) % clipped->point_count];

        Line line;
        Line_between(&line, p0, pf);
        Line_display(&line);
      }
    }

  }

  Poly_destroy(clipped);
}




typedef struct {
  Poly *poly;
  Model *belongs_to;
  int is_halo;
} DisplayPoly;

int comparator(const void *_dPoly0, const void *_dPoly1) {
  const DisplayPoly *dPoly0 = (const DisplayPoly *) _dPoly0;
  const DisplayPoly *dPoly1 = (const DisplayPoly *) _dPoly1;

  Point center0;
  Point center1;

  Poly_calc_center_M(&center0, dPoly0->poly);
  Poly_calc_center_M(&center1, dPoly1->poly);

  const double dist0 = PointVec_magnitude(&center0);
  const double dist1 = PointVec_magnitude(&center1);

  // If they belong to the same model, place  focused polygons at the back.
  if (dPoly0->belongs_to == dPoly1->belongs_to) {
    if (dPoly0->is_halo && !dPoly1->is_halo) return -1;
    if (dPoly1->is_halo && !dPoly0->is_halo) return +1;
  }

  // Place distant polygons before
  if (dist0 > dist1) return -1;
  if (dist1 > dist0) return +1;

  return 0;
}

void display_models(Model *models[], int model_count, Model *focused_model, Model *light_source) {
  Point light_source_loc;
  Model_center_M(&light_source_loc, light_source);

  // We need to draw aw polygons at once, not model-by-model, in order to
  // correctly handle overlapping models.
  int total_poly_count = 0;
  for (int model_idx = 0; model_idx < model_count; model_idx++) {
    const Model *model = models[model_idx];
    for (int poly_idx = 0; poly_idx < model->poly_count; poly_idx++) {
      const Poly *poly = model->polys[poly_idx];
      if (shouldnt_display(poly)) continue;
      total_poly_count++;

      // Will have a focused duplicate
      if (DO_POLY_FILL && DO_HALO && model == focused_model) total_poly_count++;
    }
  }

  DisplayPoly aggregate_dPolys[total_poly_count];
  int aggregate_dPolys_i = 0;

  for (int model_idx = 0; model_idx < model_count; model_idx++) {
    const Model *model = models[model_idx];
    const int is_focused = model == focused_model;
    for (int poly_idx = 0; poly_idx < model->poly_count; poly_idx++) {
      const Poly *poly = model->polys[poly_idx];

      if (shouldnt_display(poly)) continue;

      const DisplayPoly dPoly = {
        .poly = (Poly *) poly,
        .belongs_to = (Model *) model,
        .is_halo = 0 };

      aggregate_dPolys[aggregate_dPolys_i] = dPoly;
      aggregate_dPolys_i++;

      // Create halo clone
      if (DO_POLY_FILL && DO_HALO && is_focused) {
        const DisplayPoly focusedDPoly = {
          .poly = (Poly *) poly,
          .belongs_to = (Model *) model,
          .is_halo = 1 };

        aggregate_dPolys[aggregate_dPolys_i] = focusedDPoly;
        aggregate_dPolys_i++;
      }
    }
  }

  qsort((void *) aggregate_dPolys, total_poly_count, sizeof(DisplayPoly), comparator);

  for (int dPoly_idx = 0; dPoly_idx < total_poly_count; dPoly_idx++) {
    DisplayPoly dPoly = aggregate_dPolys[dPoly_idx];
    Poly_display(dPoly.poly, dPoly.belongs_to == focused_model, dPoly.is_halo, &light_source_loc);
  }
}


#endif // display_h_INCLUDED

