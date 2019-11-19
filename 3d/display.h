#ifndef display_h_INCLUDED
#define display_h_INCLUDED

// Functions relating to displaying stuff

#include <math.h>

#include "poly.h"
#include "model.h"
#include "pointvec.h"

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

double Poly_calc_intensity(const Poly *poly) {
  // TODO: REMOVE
  Point light_source;
  PointVec_init(&light_source, 0, 0, 0);

  if (!DO_LIGHT_MODEL) return 1;

  Vec poly_normal;
  Poly_normal_M(&poly_normal, poly);

  Point poly_center;
  Poly_calc_center_M(&poly_center, poly);

  Vec to_light;
  PointVec_between_M(&to_light, &poly_center, &light_source);
  PointVec_normalize(&to_light);

  // Make sure normal is going correct direction
  if (PointVec_dot(&poly_normal, &to_light) < 0) {
    PointVec_negate(&poly_normal);
  }

  const double cos_alpha = PointVec_dot(&poly_normal, &to_light);

  Point observer;
  PointVec_init(&observer, 0, 0, 0);

  Vec to_observer;
  PointVec_between_M(&to_observer, &poly_center, &observer);
  PointVec_normalize(&to_observer);

  // If bserver is on other side of polygon, no reflected light
  // is seen.
  if (PointVec_dot(&to_observer, &to_light) < 0) {
    return AMBIENT;
  }

  Vec in_plane;
  PointVec_cross_M(&in_plane, &poly_normal, &to_light);
  Vec plane_normal;
  PointVec_cross_M(&plane_normal, &poly_normal, &in_plane);

  Vec reflection;
  PointVec_clone_M(&reflection, &to_light);
  PointVec_reflect_over_plane_M(&reflection, &reflection, &poly_center, &plane_normal);

  const double cos_beta = PointVec_dot(&reflection, &to_observer);

  return AMBIENT
         + DIFFUSE_MAX * fmax(0, cos_alpha)
         + (1 - AMBIENT - DIFFUSE_MAX)
           * pow(fmax(0, cos_beta), SPECULAR_POWER);
}

void Poly_display(const Poly *poly, int focused) {
  double xs[poly->point_count];
  double ys[poly->point_count];
  for (int point_idx = 0; point_idx < poly->point_count; point_idx++) {
    const Point *p = poly->points[point_idx];
    pixel_coords_M(&xs[point_idx], &ys[point_idx], p);
  }

  const double intensity = Poly_calc_intensity(poly);

  if (DO_POLY_FILL) {
    G_rgb(0.8 * intensity, 0.5 * intensity, 0.8 * intensity);
    G_fill_polygon(xs, ys, poly->point_count);
  }

  focused ? G_rgb(0.7, 0.7, 0.7) : G_rgb(0, 0, 0);
  for (int point_idx = 0; point_idx < poly->point_count; point_idx++) {
    const Point *p0 = poly->points[point_idx];
    const Point *pf = poly->points[(point_idx + 1) % poly->point_count];
    display_line(p0, pf);
  }
}




typedef struct {
  Poly *poly;
  int is_focused;
}  MaybeFocusedPoly;

int comparator(const void *_mfPoly0, const void *_mfPoly1) {
  const MaybeFocusedPoly *mfPoly0 = (const MaybeFocusedPoly *) _mfPoly0;
  const MaybeFocusedPoly *mfPoly1 = (const MaybeFocusedPoly *) _mfPoly1;

  Point center0;
  Point center1;

  Poly_calc_center_M(&center0, mfPoly0->poly);
  Poly_calc_center_M(&center1, mfPoly1->poly);

  const double dist0 = PointVec_magnitude(&center0);
  const double dist1 = PointVec_magnitude(&center1);

  return dist1 > dist0;
}

void display_models(Model *models[], int model_count, Model *focused_model) {
  // We need to draw aw polygons at once, not model-by-model, in order to
  // correctly handle overlapping models.
  int total_poly_count = 0;
  for (int model_idx = 0; model_idx < model_count; model_idx++) {
    const Model *model = models[model_idx];
    for (int poly_idx = 0; poly_idx < model->poly_count; poly_idx++) {
      const Poly *poly = model->polys[poly_idx];
      if (shouldnt_display(poly)) continue;
      total_poly_count++;
    }
  }

  MaybeFocusedPoly aggregate_mfPolys[total_poly_count];
  int aggregate_mfPolys_i = 0;

  for (int model_idx = 0; model_idx < model_count; model_idx++) {
    const Model *model = models[model_idx];
    const int is_focused = model == focused_model;
    for (int poly_idx = 0; poly_idx < model->poly_count; poly_idx++) {
      const Poly *poly = model->polys[poly_idx];

      if (shouldnt_display(poly)) continue;

      MaybeFocusedPoly mfPoly;
      mfPoly.poly = (Poly *) poly;
      mfPoly.is_focused = is_focused;

      aggregate_mfPolys[aggregate_mfPolys_i] = mfPoly;
      aggregate_mfPolys_i++;
    }
  }

  qsort((void *) aggregate_mfPolys, total_poly_count, sizeof(MaybeFocusedPoly), comparator);

  for (int mfPoly_idx = 0; mfPoly_idx < total_poly_count; mfPoly_idx++) {
    MaybeFocusedPoly mfPoly = aggregate_mfPolys[mfPoly_idx];
    Poly_display(mfPoly.poly, mfPoly.is_focused);
  }
}


#endif // display_h_INCLUDED

