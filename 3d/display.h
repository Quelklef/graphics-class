#ifndef display_h_INCLUDED
#define display_h_INCLUDED

// Functions relating to displaying stuff

#include <math.h>

#include "poly.h"
#include "model.h"
#include "pointvec.h"

double sgn(double x) {
  if (x > 0) return +1;
  if (x < 0) return -1;
  return 0;
}

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

void display_line(const Point *p0, const Point *pf) {
  double pixel_x0, pixel_y0, pixel_xf, pixel_yf;
  pixel_coords_M(&pixel_x0, &pixel_y0, p0);
  pixel_coords_M(&pixel_xf, &pixel_yf, pf);

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
  PointVec_normalize(&poly_normal);

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

void Poly_display(const Poly *poly, int focused, Point *light_source_loc) {
  if (DO_POLY_FILL) {

    if (focused) {
      Poly_display_as_halo(poly);
      return;
    }

    double r = 0.8;
    double g = 0.5;
    double b = 0.8;

    if (DO_LIGHT_MODEL) {
      Poly_calc_color_M(
        &r, &g, &b,
        poly, light_source_loc,
        r, g, b);
    }

    G_rgb(r, g, b);
    Poly_display_minimal(poly);
  }

  if (DO_WIREFRAME) {
    if (!DO_POLY_FILL && focused) {
      G_rgb(1, 0, 0);
    } else {
      G_rgb(.3, .3, .3);
    }

    for (int point_idx = 0; point_idx < poly->point_count; point_idx++) {
      const Point *p0 = poly->points[point_idx];
      const Point *pf = poly->points[(point_idx + 1) % poly->point_count];
      display_line(p0, pf);
    }
  }
}




typedef struct {
  Poly *poly;
  Model *belongs_to;
  int is_focused;
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
    if (dPoly0->is_focused && !dPoly1->is_focused) return -1;
    if (dPoly1->is_focused && !dPoly0->is_focused) return +1;
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
      if (DO_POLY_FILL && model == focused_model) total_poly_count++;
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
        .is_focused = is_focused };

      aggregate_dPolys[aggregate_dPolys_i] = dPoly;
      aggregate_dPolys_i++;

      // Create non-focused clone
      if (is_focused && DO_POLY_FILL) {
        const DisplayPoly focusedDPoly = {
          .poly = (Poly *) poly,
          .belongs_to = (Model *) model,
          .is_focused = 0 };

        aggregate_dPolys[aggregate_dPolys_i] = focusedDPoly;
        aggregate_dPolys_i++;
      }
    }
  }

  qsort((void *) aggregate_dPolys, total_poly_count, sizeof(DisplayPoly), comparator);

  for (int dPoly_idx = 0; dPoly_idx < total_poly_count; dPoly_idx++) {
    DisplayPoly dPoly = aggregate_dPolys[dPoly_idx];
    Poly_display(dPoly.poly, dPoly.is_focused, &light_source_loc);
  }
}


#endif // display_h_INCLUDED

