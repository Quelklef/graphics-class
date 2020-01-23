#ifndef display_h_INCLUDED
#define display_h_INCLUDED

// Functions relating to displaying stuff

#include <math.h>

#include "misc.h"
#include "poly.h"
#include "model.h"
#include "plane.h"
#include "observer.h"

void pixel_coords_M(float *result_x, float *result_y, const v3 point) {
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

  *result_x = x_bar_bar + minor / 2;
  *result_y = y_bar_bar + minor / 2;
}

void Line_display(const Line *line) {
  float pixel_x0, pixel_y0, pixel_xf, pixel_yf;
  pixel_coords_M(&pixel_x0, &pixel_y0, line->p0);
  pixel_coords_M(&pixel_xf, &pixel_yf, line->pf);

  G_line(pixel_x0, pixel_y0, pixel_xf, pixel_yf);
}

int shouldnt_display(const Poly *poly) {
  // Implements backface elimination

  if (!DO_BACKFACE_ELIMINATION) return 0;

  const v3 origin = { 0, 0, 0 };
  const v3 T = poly->points[0] - origin;
  const v3 N = Poly_normal(poly);
  return BACKFACE_ELIMINATION_SIGN * v3_dot(T, N) < 0;
}

float Poly_calc_intensity(const Poly *poly, const v3 light_source_loc) {
  const v3 poly_center = Poly_center(poly);
  const v3 to_light = v3_normalize(light_source_loc - poly_center);

  v3 poly_normal = Poly_normal(poly);
  // Make sure normal is going correct direction
  if (v3_dot(poly_normal, to_light) < 0) {
    poly_normal = -poly_normal;
  }

  const float cos_alpha = v3_dot(poly_normal, to_light);
  const float cos_alpha_0 = fmax(0, cos_alpha);

  const v3 observer = { 0, 0, 0 };
  const v3 to_observer = v3_normalize(observer - poly_center);

  // If observer is on other side of polygon, no reflected light is seen.
  const float A = v3_dot(poly_normal, to_light);
  const float B = v3_dot(poly_normal, to_observer);
  if (sgn(A) != sgn(B)) {
    return AMBIENT;
  }

  const v3 scaled_normal = poly_normal * (float) (2 * cos_alpha);
  const v3 reflection = -to_light + scaled_normal;

  const float cos_beta = v3_dot(reflection, to_observer);
  const float cos_beta_0 = fmax(0, cos_beta);

  return AMBIENT
         + DIFFUSE_MAX * cos_alpha_0
         + (1 - AMBIENT - DIFFUSE_MAX) * pow(cos_beta_0, SPECULAR_POWER);
}

void Poly_calc_color_M(
      float *result_r, float *result_g, float *result_b, 
      const Poly *poly, const v3 light_source_loc,
      const float inherent_r, const float inherent_g, const float inherent_b
    ) {

  const float intensity = Poly_calc_intensity(poly, light_source_loc);
  const float full = AMBIENT + DIFFUSE_MAX;

  if (intensity == full) {
    *result_r = inherent_r;
    *result_g = inherent_g;
    *result_b = inherent_b;
    return;
  }

  if (intensity > full) {
    const float ratio = (intensity - full) / (1 - full);
    *result_r = inherent_r + (1 - inherent_r) * ratio;
    *result_g = inherent_g + (1 - inherent_g) * ratio;
    *result_b = inherent_b + (1 - inherent_b) * ratio;
    return;
  }

  if (intensity < full) {
    const float ratio = intensity / full;
    *result_r = inherent_r * ratio;
    *result_g = inherent_g * ratio;
    *result_b = inherent_b * ratio;
    return;
  }
}

void floats_to_doubles_M(double *result, const float *xs, const int count) {
  for (int i = 0; i < count; i++) {
    result[i] = (double) xs[i];
  }
}

void _fill_polygon(const float *xs, const float *ys, const int count) {
  double dxs[count];
  double dys[count];
  floats_to_doubles_M(dxs, xs, count);
  floats_to_doubles_M(dys, ys, count);
  G_fill_polygon(dxs, dys, count);
}

void Poly_display_minimal(const Poly *poly) {
  float pxs[poly->point_count];
  float pys[poly->point_count];

  for (int point_idx = 0; point_idx < poly->point_count; point_idx++) {
    const v3 p = poly->points[point_idx];
    pixel_coords_M(&pxs[point_idx], &pys[point_idx], p);
  }

  _fill_polygon(pxs, pys, poly->point_count);
}

void Poly_display_as_halo(const Poly *poly) {
  const float scale_amt = 3;

  float pxs[poly->point_count];
  float pys[poly->point_count];
  for (int point_idx = 0; point_idx < poly->point_count; point_idx++) {
    const v3 p = poly->points[point_idx];
    pixel_coords_M(&pxs[point_idx], &pys[point_idx], p);
  }

  float center_px, center_py;
  {
    float min_px = +DBL_MAX;
    float max_px = -DBL_MAX;
    float min_py = +DBL_MAX;
    float max_py = -DBL_MAX;

    for (int point_idx = 0; point_idx < poly->point_count; point_idx++) {
      float px = pxs[point_idx];
      float py = pys[point_idx];

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
  _fill_polygon(pxs, pys, poly->point_count);
}

void Poly_clip_with_plane(Poly *poly, const Plane *plane) {
  // Clip a polygon with a plane
  // The portion of the shape on the on the same side as the point
  //   (0, 0, (YON + HITHER)/2) will be kept
  //   (given that YON > HITHER)
  // Frees removed points

  // If HITHER >= YON, then all models are entirely clipped
  if (HITHER >= YON) {
    Poly_clear(poly);
    return;
  }

  Poly result_poly;
  Poly_init(&result_poly);

  // Create a point that's definitely inside the clipping region
  const v3 inside_point = { 0, 0, YON / 2 + HITHER / 2 };
  const int inside_side = Plane_side_of(plane, inside_point);

  for (int point_idx = 0; point_idx < poly->point_count; point_idx++) {
    v3 this_point = poly->points[point_idx];
    v3 next_point = poly->points[(point_idx + 1) % poly->point_count];

    const int this_is_inside = inside_side == Plane_side_of(plane, this_point);
    const int next_is_inside = inside_side == Plane_side_of(plane, next_point);

    Line this_to_next;
    Line_between(&this_to_next, this_point, next_point);

    if (this_is_inside) {
      Poly_add_point(&result_poly, this_point);
    }

    // If crosses over, add itersection
    if (this_is_inside != next_is_inside) {
      v3 intersection;
      const int found_intersection = Plane_intersect_line_M(&intersection, plane, &this_to_next);
      // We know there should be an intersection since the points are on opposite sides of the plane
      if (!found_intersection) {
        printf("Error in Poly_clip_with_plane: intersection not found.\n");
        exit(1);
      }

      Poly_add_point(&result_poly, intersection);
    }

  }

  *poly = result_poly;
}

void Poly_clip(Poly *poly) {
  // Clip a polygon according to HALF_ANGLE, YON, and HITHER
  // Frees removed points

  v3 observer = { 0, 0, 0 };

  const float tha = tan(HALF_ANGLE);

  // --

  v3 screen_top_left     = { -tha,  tha, 1 };
  v3 screen_top_right    = {  tha,  tha, 1 };
  v3 screen_bottom_left  = { -tha, -tha, 1 };
  v3 screen_bottom_right = {  tha, -tha, 1 };

  // --

  Plane above_plane;
  Plane_from_points(&above_plane, observer, screen_top_left, screen_top_right);

  Plane left_plane;
  Plane_from_points(&left_plane, observer, screen_top_left, screen_bottom_left);

  Plane bottom_plane;
  Plane_from_points(&bottom_plane, observer, screen_bottom_left, screen_bottom_right);

  Plane right_plane;
  Plane_from_points(&right_plane, observer, screen_bottom_right, screen_top_right);

  Plane hither_plane;
  {
    v3 p0 = { 0, 0, HITHER };
    v3 normal = { 0, 0, 1 };
    Plane_from_point_normal(&hither_plane, p0, normal);
  }

  Plane yon_plane;
  {
    v3 p0 = { 0, 0, YON };
    v3 normal = { 0, 0, -1 };
    Plane_from_point_normal(&yon_plane, p0, normal);
  }

  // --

  Poly_clip_with_plane(poly, &above_plane);
  Poly_clip_with_plane(poly, &left_plane);
  Poly_clip_with_plane(poly, &bottom_plane);
  Poly_clip_with_plane(poly, &right_plane);
  Poly_clip_with_plane(poly, &hither_plane);
  Poly_clip_with_plane(poly, &yon_plane);
}

void Poly_display(const Poly *poly, const int is_focused, const int is_halo, const v3 light_source_loc) {
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
        float r = 0.8;
        float g = 0.5;
        float b = 0.8;

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
        const v3 p0 = clipped->points[point_idx];
        const v3 pf = clipped->points[(point_idx + 1) % clipped->point_count];

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

  v3 center0 = Poly_center(dPoly0->poly);
  v3 center1 = Poly_center(dPoly1->poly);

  const float dist0 = v3_mag(center0);
  const float dist1 = v3_mag(center1);

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

void display_models_aux(Model *models[], int model_count, Model *focused_model, Model *light_source) {
  v3 light_source_loc = Model_center(light_source);

  // We need to draw all polygons at once, not model-by-model, in order to
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
    Poly_display(dPoly.poly, dPoly.belongs_to == focused_model, dPoly.is_halo, light_source_loc);
  }
}

void display_models(Model *models[], const int model_count, const Model *focused_model, const Model *light_source) {

  // Move from world space to eyespace

  // Will clone all models and store their transformed
  // versions in this array
  Model *in_eyespace[model_count];
  Model *focused_clone = NULL;
  Model *light_source_clone = NULL;

  _Mat to_eyespace;
  calc_eyespace_matrix_M(to_eyespace);

  for (int model_i = 0; model_i < model_count; model_i++) {
    const Model *model = models[model_i];
    Model *clone = Model_clone(model);
    Model_transform(clone, to_eyespace);
    in_eyespace[model_i] = clone;
    if (model == focused_model) focused_clone = clone;
    if (model == light_source) light_source_clone = clone;
  }

  // Pass to auxiliary after moving to eyespace
  display_models_aux(in_eyespace, model_count, focused_clone, light_source_clone);

  // Clean up
  for (int clone_i = 0; clone_i < model_count; clone_i++) {
    Model *clone = in_eyespace[clone_i];
    Model_destroy(clone);
  }

}


#endif // display_h_INCLUDED

