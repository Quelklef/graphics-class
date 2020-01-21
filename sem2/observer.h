#ifndef eye_h_INCLUDED
#define eye_h_INCLUDED

#include <math.h>

#include "pointvec.h"
#include "matrix.h"

// Points that define the observer
Point *eye;
Point *up_point;
Point *center_of_interest;

void init_observer() {
  eye = PointVec_new(0, 0, 0);
  up_point = PointVec_new(0, 1, 0);
  center_of_interest = PointVec_new(0, 0, 1);
}

void calc_eyespace_matrix_M(_Mat result) {
  /* Calculate the matrix that takes shapes
   * from world space to eye space
   */

  Point eye_clone;
  PointVec_overwrite(&eye_clone, eye);

  Point up_clone;
  PointVec_overwrite(&up_clone, up_point);

  Point coi_clone;
  PointVec_overwrite(&coi_clone, center_of_interest);


  // Move the observer to the origin
  _Mat observer_to_origin;
  Mat_translation_M(observer_to_origin, -eye->x, -eye->y, -eye->z);

  // Emulate doing this to the existing stuff
  PointVec_transform(&eye_clone, observer_to_origin);
  PointVec_transform(&up_clone, observer_to_origin);
  PointVec_transform(&coi_clone, observer_to_origin);

  // Rotate observer so that the center of interest is on the z-axis
  _Mat align_coi_1;
  const double theta1 = -atan2(coi_clone.x, coi_clone.z);
  Mat_y_rotation_M(align_coi_1, theta1);

  _Mat align_coi_2;
  const double theta2 = -atan2(coi_clone.y, coi_clone.z);
  Mat_x_rotation_M(align_coi_2, theta2);

  // Emulate doing this to the existing stuff
  PointVec_transform(&eye_clone, align_coi_1);
  PointVec_transform(&eye_clone, align_coi_2);
  PointVec_transform(&up_clone, align_coi_1);
  PointVec_transform(&up_clone, align_coi_2);
  PointVec_transform(&coi_clone, align_coi_1);
  PointVec_transform(&coi_clone, align_coi_2);

  // Rotate observer so that the up point is on the y-z plane
  _Mat align_up;
  const double theta3 = -atan2(up_clone.x, up_clone.z);
  Mat_y_rotation_M(align_up, theta3);

  // Compose all the matrices
  Mat_identity_M(result);
  Mat_mult_left(result, observer_to_origin);
  Mat_mult_left(result, align_coi_1);
  Mat_mult_left(result, align_coi_2);
  Mat_mult_left(result, align_up);
}

#endif // eye_h_INCLUDED

