#ifndef display_h_INCLUDED
#define display_h_INCLUDED

// Functions relating to displaying stuff

#include <math.h>

#include "misc.h"
#include "polygon.h"
#include "polyhedron.h"
#include "plane.h"
#include "observer.h"
#include "v2.h"

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

void Line_display(const Line *line) {
  const v2 pixel0 = pixel_coords(line->p0);
  const v2 pixelf = pixel_coords(line->pf);
  G_line(pixel0[0], pixel0[1], pixelf[0], pixelf[1]);
}

int shouldnt_display(const Polygon *polygon) {
  // Implements backface elimination

  if (!DO_BACKFACE_ELIMINATION) return 0;

  const v3 origin = { 0, 0, 0 };
  const v3 T = Polygon_get(polygon, 0) - origin;
  const v3 N = Polygon_normal(polygon);
  return BACKFACE_ELIMINATION_SIGN * v3_dot(T, N) < 0;
}

float Polygon_calc_intensity(const Polygon *polygon, const v3 light_source_loc) {
  const v3 polygon_center = Polygon_center(polygon);
  const v3 to_light = v3_normalize(light_source_loc - polygon_center);

  v3 polygon_normal = Polygon_normal(polygon);
  // Make sure normal is going correct direction
  if (v3_dot(polygon_normal, to_light) < 0) {
    polygon_normal = -polygon_normal;
  }

  const float cos_alpha = v3_dot(polygon_normal, to_light);
  const float cos_alpha_0 = fmax(0, cos_alpha);

  const v3 observer = { 0, 0, 0 };
  const v3 to_observer = v3_normalize(observer - polygon_center);

  // If observer is on other side of polygongon, no reflected light is seen.
  const float A = v3_dot(polygon_normal, to_light);
  const float B = v3_dot(polygon_normal, to_observer);
  if (sgn(A) != sgn(B)) {
    return AMBIENT;
  }

  const v3 scaled_normal = polygon_normal * (float) (2 * cos_alpha);
  const v3 reflection = -to_light + scaled_normal;

  const float cos_beta = v3_dot(reflection, to_observer);
  const float cos_beta_0 = fmax(0, cos_beta);

  return AMBIENT
         + DIFFUSE_MAX * cos_alpha_0
         + (1 - AMBIENT - DIFFUSE_MAX) * pow(cos_beta_0, SPECULAR_POWER);
}

v3 Polygon_calc_color(const Polygon *polygon, const v3 light_source_loc, const v3 inherent_rgb) {
  const float intensity = Polygon_calc_intensity(polygon, light_source_loc);
  const float full = AMBIENT + DIFFUSE_MAX;

  if (intensity > full) {
    const float ratio = (intensity - full) / (1 - full);
    return inherent_rgb + (1 - inherent_rgb) * ratio;
  }

  if (intensity < full) {
    const float ratio = intensity / full;
    return inherent_rgb * ratio;
  }

  // intensity == full
  return inherent_rgb;
}

void floats_to_doubles_M(double *result, const float *xs, const int count) {
  for (int i = 0; i < count; i++) {
    result[i] = (double) xs[i];
  }
}

void _fill_polygongon(const float *xs, const float *ys, const int count) {
  double dxs[count];
  double dys[count];
  floats_to_doubles_M(dxs, xs, count);
  floats_to_doubles_M(dys, ys, count);
  G_fill_polygon(dxs, dys, count);
}

void Polygon_display_minimal(const Polygon *polygon) {
  float pxs[polygon->length];
  float pys[polygon->length];

  for (int point_idx = 0; point_idx < polygon->length; point_idx++) {
    const v3 point = Polygon_get(polygon, point_idx);
    const v2 pixel = pixel_coords(point);
    pxs[point_idx] = pixel[0];
    pys[point_idx] = pixel[1];
  }

  _fill_polygongon(pxs, pys, polygon->length);
}

void Polygon_display_as_halo(const Polygon *polygon) {
  const float scale_amt = 3;

  float pxs[polygon->length];
  float pys[polygon->length];
  for (int point_idx = 0; point_idx < polygon->length; point_idx++) {
    const v3 point = Polygon_get(polygon, point_idx);
    const v2 pixel = pixel_coords(point);
    pxs[point_idx] = pixel[0];
    pys[point_idx] = pixel[1];
  }

  float center_px, center_py;
  {
    float min_px = +DBL_MAX;
    float max_px = -DBL_MAX;
    float min_py = +DBL_MAX;
    float max_py = -DBL_MAX;

    for (int point_idx = 0; point_idx < polygon->length; point_idx++) {
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

  for (int point_idx = 0; point_idx < polygon->length; point_idx++) {
    pxs[point_idx] = (pxs[point_idx] - center_px) * scale_amt + center_px;
    pys[point_idx] = (pys[point_idx] - center_py) * scale_amt + center_py;
  }

  G_rgb(1, 0, 0);
  _fill_polygongon(pxs, pys, polygon->length);
}

void Polygon_clip_with_plane(Polygon *polygon, const Plane *plane) {
  // Clip a polygongon with a plane
  // The portion of the shape on the on the same side as the point
  //   (0, 0, (YON + HITHER)/2) will be kept
  //   (given that YON > HITHER)
  // Frees removed points

  // If HITHER >= YON, then all polyhedra are entirely clipped
  if (HITHER >= YON) {
    Polygon_clear(polygon);
    return;
  }

  Polygon result_polygon;
  Polygon_init(&result_polygon, polygon->length + 1);
  // We can gain at most one point from clipping, so polygon->length+1 is an upper bound

  // Create a point that's definitely inside the clipping region
  const v3 inside_point = { 0, 0, YON / 2 + HITHER / 2 };
  const int inside_side = Plane_side_of(plane, inside_point);

  for (int point_idx = 0; point_idx < polygon->length; point_idx++) {
    v3 this_point = Polygon_get(polygon, point_idx);
    v3 next_point = Polygon_get(polygon, (point_idx + 1) % polygon->length);

    const int this_is_inside = inside_side == Plane_side_of(plane, this_point);
    const int next_is_inside = inside_side == Plane_side_of(plane, next_point);

    Line this_to_next;
    Line_between(&this_to_next, this_point, next_point);

    if (this_is_inside) {
      Polygon_append(&result_polygon, this_point);
    }

    // If crosses over, add itersection
    if (this_is_inside != next_is_inside) {
      v3 intersection;
      const int found_intersection = Plane_intersect_line_M(&intersection, plane, &this_to_next);
      // We know there should be an intersection since the points are on opposite sides of the plane
      if (!found_intersection) {
        printf("Error in Polygon_clip_with_plane: intersection not found.\n");
        exit(1);
      }

      Polygon_append(&result_polygon, intersection);
    }

  }

  *polygon = result_polygon;
}

void Polygon_clip(Polygon *polygon) {
  // Clip a polygongon according to HALF_ANGLE, YON, and HITHER
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

  Polygon_clip_with_plane(polygon, &above_plane);
  Polygon_clip_with_plane(polygon, &left_plane);
  Polygon_clip_with_plane(polygon, &bottom_plane);
  Polygon_clip_with_plane(polygon, &right_plane);
  Polygon_clip_with_plane(polygon, &hither_plane);
  Polygon_clip_with_plane(polygon, &yon_plane);
}

void Polygon_display(const Polygon *polygon, const int is_focused, const int is_halo, const v3 light_source_loc) {
  // focused: is the polygongon part of the focused polyhedron? (NOT part of the halo)
  // halo: is the polygongon part of a halo?

  Polygon *clipped = Polygon_clone(polygon);
  if (DO_CLIPPING) {
    Polygon_clip(clipped);
  }

  if (clipped->length != 0) {
    // Only display if there are points
    // (Some display subroutines require a minimum point count)

    if (DO_POLY_FILL) {
      if (is_halo) {
        Polygon_display_as_halo(clipped);
      } else {
        v3 rgb = { 0.8, 0.5, 0.8 };

        if (DO_LIGHT_MODEL) {
          rgb = Polygon_calc_color(clipped, light_source_loc, rgb);
        }

        G_rgb(rgb[0], rgb[1], rgb[2]);
        Polygon_display_minimal(clipped);
      }
    }

    if (DO_WIREFRAME && !is_halo) {
      if ((!DO_POLY_FILL || !DO_HALO) && is_focused) {
        G_rgb(1, 0, 0);
      } else {
        G_rgb(.3, .3, .3);
      }

      for (int point_idx = 0; point_idx < clipped->length; point_idx++) {
        const v3 p0 = Polygon_get(clipped, point_idx);
        const v3 pf = Polygon_get(clipped, (point_idx + 1) % clipped->length);

        Line line;
        Line_between(&line, p0, pf);
        Line_display(&line);
      }
    }

  }

  Polygon_destroy(clipped);
}




typedef struct {
  Polygon *polygon;
  Polyhedron *belongs_to;
  int is_halo;
} DisplayPolygon;

int comparator(const void *_dPolygon0, const void *_dPolygon1) {
  const DisplayPolygon *dPolygon0 = (const DisplayPolygon *) _dPolygon0;
  const DisplayPolygon *dPolygon1 = (const DisplayPolygon *) _dPolygon1;

  v3 center0 = Polygon_center(dPolygon0->polygon);
  v3 center1 = Polygon_center(dPolygon1->polygon);

  const float dist0 = v3_mag(center0);
  const float dist1 = v3_mag(center1);

  // If they belong to the same polyhedron, place  focused polygongons at the back.
  if (dPolygon0->belongs_to == dPolygon1->belongs_to) {
    if (dPolygon0->is_halo && !dPolygon1->is_halo) return -1;
    if (dPolygon1->is_halo && !dPolygon0->is_halo) return +1;
  }

  // Place distant polygongons before
  if (dist0 > dist1) return -1;
  if (dist1 > dist0) return +1;

  return 0;
}

void display_polyhedra_aux(Polyhedron *polyhedra[], int polyhedron_count, Polyhedron *focused_polyhedron, Polyhedron *light_source) {
  v3 light_source_loc = Polyhedron_center(light_source);

  // We need to draw all polygongons at once, not polyhedron-by-polyhedron, in order to
  // correctly handle overlapping polyhedra.
  int total_polygon_count = 0;
  for (int polyhedron_idx = 0; polyhedron_idx < polyhedron_count; polyhedron_idx++) {
    const Polyhedron *polyhedron = polyhedra[polyhedron_idx];
    for (int polygon_idx = 0; polygon_idx < polyhedron->length; polygon_idx++) {
      const Polygon *polygon = Polyhedron_get(polyhedron, polygon_idx);
      if (shouldnt_display(polygon)) continue;
      total_polygon_count++;

      // Will have a focused duplicate
      if (DO_POLY_FILL && DO_HALO && polyhedron == focused_polyhedron) total_polygon_count++;
    }
  }

  DisplayPolygon aggregate_dPolygons[total_polygon_count];
  int aggregate_dPolygons_i = 0;

  for (int polyhedron_idx = 0; polyhedron_idx < polyhedron_count; polyhedron_idx++) {
    const Polyhedron *polyhedron = polyhedra[polyhedron_idx];
    const int is_focused = polyhedron == focused_polyhedron;
    for (int polygon_idx = 0; polygon_idx < polyhedron->length; polygon_idx++) {
      const Polygon *polygon = Polyhedron_get(polyhedron, polygon_idx);

      if (shouldnt_display(polygon)) continue;

      const DisplayPolygon dPolygon = {
        .polygon = (Polygon *) polygon,
        .belongs_to = (Polyhedron *) polyhedron,
        .is_halo = 0 };

      aggregate_dPolygons[aggregate_dPolygons_i] = dPolygon;
      aggregate_dPolygons_i++;

      // Create halo clone
      if (DO_POLY_FILL && DO_HALO && is_focused) {
        const DisplayPolygon focusedDPolygon = {
          .polygon = (Polygon *) polygon,
          .belongs_to = (Polyhedron *) polyhedron,
          .is_halo = 1 };

        aggregate_dPolygons[aggregate_dPolygons_i] = focusedDPolygon;
        aggregate_dPolygons_i++;
      }
    }
  }

  qsort((void *) aggregate_dPolygons, total_polygon_count, sizeof(DisplayPolygon), comparator);

  for (int dPolygon_idx = 0; dPolygon_idx < total_polygon_count; dPolygon_idx++) {
    DisplayPolygon dPolygon = aggregate_dPolygons[dPolygon_idx];
    Polygon_display(dPolygon.polygon, dPolygon.belongs_to == focused_polyhedron, dPolygon.is_halo, light_source_loc);
  }
}

void display_polyhedra(Polyhedron *polyhedra[], const int polyhedron_count, const Polyhedron *focused_polyhedron, const Polyhedron *light_source) {

  // Move from world space to eyespace

  // Will clone all polyhedra and store their transformed
  // versions in this array
  Polyhedron *in_eyespace[polyhedron_count];
  Polyhedron *focused_clone = NULL;
  Polyhedron *light_source_clone = NULL;

  _Mat to_eyespace;
  calc_eyespace_matrix_M(to_eyespace);

  for (int polyhedron_i = 0; polyhedron_i < polyhedron_count; polyhedron_i++) {
    const Polyhedron *polyhedron = polyhedra[polyhedron_i];
    Polyhedron *clone = Polyhedron_clone(polyhedron);
    Polyhedron_transform(clone, to_eyespace);
    in_eyespace[polyhedron_i] = clone;
    if (polyhedron == focused_polyhedron) focused_clone = clone;
    if (polyhedron == light_source) light_source_clone = clone;
  }

  // Pass to auxiliary after moving to eyespace
  display_polyhedra_aux(in_eyespace, polyhedron_count, focused_clone, light_source_clone);

  // Clean up
  for (int clone_i = 0; clone_i < polyhedron_count; clone_i++) {
    Polyhedron *clone = in_eyespace[clone_i];
    Polyhedron_destroy(clone);
  }

}


#endif // display_h_INCLUDED

