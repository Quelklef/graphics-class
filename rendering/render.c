#ifndef render_c_INCLUDED
#define render_c_INCLUDED

// Functions relating to rendering stuff

#include <math.h>

#include "observer.c"
#include "draw.c"
#include "../util/misc.c"
#include "../shapes/polygon.c"
#include "../shapes/polyhedron.c"
#include "../shapes/figure.c"
#include "../shapes/plane.c"
#include "../shapes/v2.c"



float clamp(float x, float lo, float hi);

void Line_render(const Line *line, Zbuf zbuf) {
  const v2 px0 = pixel_coords(line->p0);
  const v2 pxf = pixel_coords(line->pf);

  const int x_lo = (int) floor(px0[0]);
  const int x_hi = (int) ceil(pxf[0]);
  for (int x = x_lo; x <= x_hi; x++) {
    const float t = (x - px0[0]) / (pxf[0] - px0[0]);
    if (0 <= t && t <= 1) {
      const v3 point = line->p0 + t * (line->pf - line->p0);
      const v2 pixel = pixel_coords(point);
      zbuf_drawv(zbuf, pixel, point[2]);
    }
  }

  const int y_lo = (int) floor(px0[1]);
  const int y_hi = (int) ceil(pxf[1]);
  for (int y = y_lo; y <= y_hi; y++) {
    const float t = (y - px0[1]) / (pxf[1] - px0[1]);
    if (0 <= t && t <= 1) {
      const v3 point = line->p0 + t * (line->pf - line->p0);
      const v2 pixel = pixel_coords(point);
      zbuf_drawv(zbuf, pixel, point[2]);
    }
  }

}

int shouldnt_render(const Polygon *polygon) {
  // Implements backface elimination

  if (!DO_BACKFACE_ELIMINATION) return 0;

  const v3 origin = { 0, 0, 0 };
  const v3 T = Polygon_get(polygon, 0) - origin;
  const v3 N = Polygon_normal(polygon);
  return BACKFACE_ELIMINATION_SIGN * v3_dot(T, N) < 0;
}

float calc_intensity(const v3 p0, v3 normal, const v3 light_source_loc) {
  /* Given a point in space, a vector normal to the surface on which the point
   * lies, and the location of the light source, calculate the intensity of the poitn
   */
  const v3 to_light = v3_normalize(light_source_loc - p0);

  // Make sure normal is going correct direction
  if (v3_dot(normal, to_light) < 0) {
    normal *= -1;
  }

  const float cos_alpha = v3_dot(normal, to_light);
  const float cos_alpha_0 = fmax(0, cos_alpha);

  const v3 observer = { 0, 0, 0 };
  const v3 to_observer = v3_normalize(observer - p0);

  // If observer is on other side of polygongon, no reflected light is seen.
  const float A = v3_dot(normal, to_light);
  const float B = v3_dot(normal, to_observer);
  if (sgn(A) != sgn(B)) {
    return AMBIENT;
  }

  const v3 scaled_normal = normal * (float) (2 * cos_alpha);
  const v3 reflection = -to_light + scaled_normal;

  const float cos_beta = v3_dot(reflection, to_observer);
  const float cos_beta_0 = fmax(0, cos_beta);

  return AMBIENT
         + DIFFUSE_MAX * cos_alpha_0
         + (1 - AMBIENT - DIFFUSE_MAX) * pow(cos_beta_0, SPECULAR_POWER);
}

v3 calc_color(const v3 p0, const v3 normal, const v3 light_source_loc, const v3 inherent_rgb) {
  const float intensity = calc_intensity(p0, normal, light_source_loc);
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

v3 Polygon_calc_color(const Polygon *polygon, const v3 light_source_loc, const v3 inherent_rgb) {
  return calc_color(
    Polygon_center(polygon),
    Polygon_normal(polygon),
    light_source_loc,
    inherent_rgb
  );
}

void Polygon_render_as_is(const Polygon *polygon, Zbuf zbuf, Zbuf zrecord) {

  // Find the pixel coordinates of all the points of the polygon
  v2 pixels[polygon->length];
  float min_px = +DBL_MAX;
  float max_px = -DBL_MAX;
  float min_py = +DBL_MAX;
  float max_py = -DBL_MAX;
  for (int i = 0; i < polygon->length; i++) {
    const v3 point = Polygon_get(polygon, i);
    const v2 pixel = pixel_coords(point);
    pixels[i] = pixel;
    const float x = pixel[0];
    const float y = pixel[1];
    if (x < min_px) min_px = x;
    if (x > max_px) max_px = x;
    if (y < min_py) min_py = y;
    if (y > max_py) max_py = y;
  }

  // Range of iteration
  const int x_lo = (int) clamp(floor(min_px), 0, SCREEN_WIDTH  - 1);
  const int x_hi = (int) clamp( ceil(max_px), 0, SCREEN_WIDTH  - 1);
  const int y_lo = (int) clamp(floor(min_py), 0, SCREEN_HEIGHT - 1);
  const int y_hi = (int) clamp( ceil(max_py), 0, SCREEN_HEIGHT - 1);

  // Find the plane of the polygon
  Plane polygon_plane;
  Plane_from_polygon(&polygon_plane, polygon);

  for (int x = x_lo; x <= x_hi; x++) {

    // Loop through lines and find intersection betwen line
    // and the vertical line at the current x
    v2 intersections[polygon->length];  // polygon->length is an upper bound
    int intersections_len = 0;

    for (int i = 0; i < polygon->length; i++) {
      // Line segment from p0 to pf
      const v2 p0 = pixels[i];
      const v2 pf = pixels[(i + 1) % polygon->length];

      // Find intersection between this line semement and
      // the vertical line at the current x
      const float t = (x - p0[0]) / (pf[0] - p0[0]);
      if (0 <= t && t <= 1) {
        intersections[intersections_len] = p0 + t * (pf - p0);
        intersections_len++;
      }
    }

    // Now given these intersections we are able to paint the polygon

    for (int y = y_lo; y <= y_hi; y++) {

      // Find the number of intersections physically above the current y
      int above_intersection_count = 0;
      for (int i = 0; i < intersections_len; i++) {
        const v2 intersection = intersections[i];
        if (intersection[1] < y) above_intersection_count++;
      }

      // The point is inside the polygon iff the intersection count is odd
      // If it's even, continue to next polygon
      if (above_intersection_count % 2 == 0) continue;

      // We want to calculate the z value corresponding to this pixel
      const v2 px = { (float) x, (float) y };

      // There is an infinite line of values with
      // the desired pixel coordinates.
      Line *line = pixel_coords_inv(px);

      // Now find the point on it that intersects with the polygon
      v3 intersection;
      const int found_intersection =
        Plane_intersect_line_M(&intersection, &polygon_plane, line);

#ifdef DEBUG
      if (!found_intersection) {
        printf("internal error: didn't find intersection\n");
        exit(1);
      }
#endif

      const float z = intersection[2];
      zbuf_draw(zbuf, x, y, z);
      zrecord[x][y] = z;

    }
  }

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

#ifdef DEBUG
      // We know there should be an intersection since the points are on opposite sides of the plane
      if (!found_intersection) {
        printf("Error in Polygon_clip_with_plane: intersection not found.\n");
        exit(1);
      }
#endif

      Polygon_append(&result_polygon, intersection);
    }

  }

  *polygon = result_polygon;
}

void Polygon_clip(Polygon *polygon) {
  // Clip a polygon according to HALF_ANGLE, YON, and HITHER

  v3 observer = { 0, 0, 0 };

  const float tha = tan(HALF_ANGLE);

  const v3 screen_top_left     = { -tha,  tha, 1 };
  const v3 screen_top_right    = {  tha,  tha, 1 };
  const v3 screen_bottom_left  = { -tha, -tha, 1 };
  const v3 screen_bottom_right = {  tha, -tha, 1 };

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
    const v3 p0 = { 0, 0, HITHER };
    const v3 normal = { 0, 0, 1 };
    Plane_from_point_normal(&hither_plane, p0, normal);
  }

  Plane yon_plane;
  {
    const v3 p0 = { 0, 0, YON };
    const v3 normal = { 0, 0, -1 };
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

void Polygon_render(
  const Polygon *polygon,
  const int is_focused,
  const v3 light_source_loc,
  Zbuf zbuf,
  Zbuf zrecord
) {
  // record all z-values on zrecord, whether or not they get drawn

  // focused: is the polygongon part of the focused polyhedron? (NOT part of the halo)

  Polygon clipped;
  memcpy(&clipped, polygon, sizeof(Polygon));

  if (DO_CLIPPING) {
    Polygon_clip(&clipped);
  }

  v3 color = { .8, .5, .8 };
  if (DO_LIGHT_MODEL) {
    color = Polygon_calc_color(&clipped, light_source_loc, color);
  }
  G_rgbv(color);

  // Only render if there are points
  // (Some render subroutines require a minimum point count)
  if (clipped.length == 0) return;

  if (DO_POLY_FILL) {
    Polygon_render_as_is(&clipped, zbuf, zrecord);
  }

  if (DO_WIREFRAME) {
    const int line_is_red = is_focused && !(DO_POLY_FILL && DO_HALO);
    const v3 line_color = line_is_red ? (v3) { 1, 0, 0 } : (v3) { .3, .3, .3 };
    G_rgbv(line_color);

    for (int point_idx = 0; point_idx < clipped.length; point_idx++) {
      const v3 p0 = Polygon_get(&clipped, point_idx);
      const v3 pf = Polygon_get(&clipped, (point_idx + 1) % clipped.length);

      Line line;
      Line_between(&line, p0, pf);
      Line_render(&line, zbuf);
      // No need to record line in the zrecord
      // because it's all the same (x, y, z) as the already drawn polygons
    }
  }

}

int iclamp(int x, int lo, int hi) {
  if (x < lo) return lo;
  if (x > hi) return hi;
  return x;
}

void zbuf_bounding_box(int *min_x, int *max_x, int *min_y, int *max_y, const Zbuf zbuf) {

  for (int x = 0; x < SCREEN_WIDTH; x++) {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
      if (zbuf[x][y] != INFINITY) {
        *min_x = x;
        goto min_x_found;
      }
    }
  }
  *min_x = SCREEN_WIDTH - 1;
  min_x_found: ;

  for (int x = SCREEN_WIDTH - 1; x >= 0; x--) {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
      if (zbuf[x][y] != INFINITY) {
        *max_x = x;
        goto max_x_found;
      }
    }
  }
  *max_x = 0;
  max_x_found: ;

  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
      if (zbuf[x][y] != INFINITY) {
        *min_y = y;
        goto min_y_found;
      }
    }
  }
  *min_y = SCREEN_HEIGHT - 1;
  min_y_found: ;

  for (int y = SCREEN_HEIGHT - 1; y >= 0; y--) {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
      if (zbuf[x][y] != INFINITY) {
        *max_y = y;
        goto max_y_found;
      }
    }
  }
  *max_y = 0;
  max_y_found: ;

}

void display_halo(Zbuf zbuf, Zbuf zrecord) {

  G_rgb(1, 0, 0);

  int min_x, max_x, min_y, max_y;
  zbuf_bounding_box(&min_x, &max_x, &min_y, &max_y, zrecord);

  static const int halo_width = 5;

  for (int x = min_x; x <= max_x; x++) {
    for (int y = min_y; y <= max_y; y++) {

      // Only iterate over drawn pixels
      if (zrecord[x][y] == INFINITY) continue;

      {
        // Also only iterate over pixels that are not surrounded on all sides
        // with another drawn pixel
        const int sx_lo = iclamp(x - 1, 0, SCREEN_WIDTH  - 1);
        const int sx_hi = iclamp(x + 1, 0, SCREEN_WIDTH  - 1);
        const int sy_lo = iclamp(y - 1, 0, SCREEN_HEIGHT - 1);
        const int sy_hi = iclamp(y + 1, 0, SCREEN_HEIGHT - 1);

        for (int sx = sx_lo; sx <= sx_hi; sx++) {
          for (int sy = sy_lo; sy <= sy_hi; sy++) {
            if (sx == 0 && sy == 0) continue;
            if (zrecord[sx][sy] == INFINITY) goto draw;
          }
        }

        continue;
      }

      draw:
      {
        // Iterate over surrounding pixels, denoted by the prefix s-
        const int sx_lo = iclamp(x - halo_width, 0, SCREEN_WIDTH  - 1);
        const int sx_hi = iclamp(x + halo_width, 0, SCREEN_WIDTH  - 1);
        const int sy_lo = iclamp(y - halo_width, 0, SCREEN_HEIGHT - 1);
        const int sy_hi = iclamp(y + halo_width, 0, SCREEN_HEIGHT - 1);

        for (int sx = sx_lo; sx <= sx_hi; sx++) {
          for (int sy = sy_lo; sy <= sy_hi; sy++) {

            // Don't overwrite existing point
            if (zrecord[sx][sy] != INFINITY) continue;

            const float z = zrecord[x][y];
            zbuf_draw(zbuf, sx, sy, z);

          }
        }
      }

    }
  }

}

void Polyhedron_render(const Polyhedron *polyhedron, const int is_focused, const v3 light_source_loc, Zbuf zbuf) {

  Zbuf zrecord;
  zbuf_init(zrecord);

  for (int i = 0; i < polyhedron->length; i++) {
    const Polygon *polygon = Polyhedron_get(polyhedron, i);
    if (shouldnt_render(polygon)) continue;
    Polygon_render(polygon, is_focused, light_source_loc, zbuf, zrecord);
  }

  if (is_focused && DO_HALO) {
    display_halo(zbuf, zrecord);
  }

}

int point_in_bounds(const v3 point) {
  /* Is the point in bounds according to YON, HITHER, and HALF_ANGLE ? */
  if (point[2] < HITHER) return 0;
  if (point[2] > YON) return 0;
  if (fabs(atan2(point[1], point[2])) > HALF_ANGLE) return 0;
  return 1;
}

void Locus_render(const Locus *locus, const int is_focused, const v3 light_source_loc, Zbuf zbuf) {
  if (is_focused) G_rgb(1, 0, 0);
  else G_rgb(.8, .5, .8);

  for (int i = 0; i < locus->length; i++) {
    const v3 point = Locus_get(locus, i);
    if (DO_CLIPPING && !point_in_bounds(point)) continue;

    const v2 pixel = pixel_coords(point);
    zbuf_drawv(zbuf, pixel, point[2]);
  }
}

void Figure_render(const Figure *figure, const int is_focused, const v3 light_source_loc, Zbuf zbuf) {
  switch (figure->kind) {
    case fk_Polyhedron: return Polyhedron_render(figure->impl.polyhedron, is_focused, light_source_loc, zbuf);
    case fk_Locus: return Locus_render(figure->impl.locus, is_focused, light_source_loc, zbuf);
  }
}



void render_figures(Figure *figures[], const int figure_count, const Figure *focused_figure, const Figure *light_source) {

  _Mat to_eyespace;
  calc_eyespace_matrix_M(to_eyespace);

  v3 light_source_loc;
  if (light_source == NULL) {
    // TODO: better solution
    light_source_loc = (v3) { -DBL_MAX, -DBL_MAX, -DBL_MAX };
  } else {
    light_source_loc = Figure_center(light_source);
  }

  Zbuf zbuf;
  zbuf_init(zbuf);

  for (int figure_i = 0; figure_i < figure_count; figure_i++) {
    const Figure *figure = figures[figure_i];

    Figure clone;
    memcpy(&clone, figure, sizeof(Figure));

    Figure_transform(&clone, to_eyespace);
    Figure_render(&clone, figure == focused_figure, light_source_loc, zbuf);
  }

}


#endif // render_c_INCLUDED
