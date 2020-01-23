#ifndef eye_h_INCLUDED
#define eye_h_INCLUDED

#include <math.h>

#include "v3.h"
#include "matrix.h"

// v3s that define the observer
v3 eye                = { 0, 0, 0 };
v3 up_point           = { 0, 1, 0 };
v3 center_of_interest = { 0, 0, 1 };

void calc_eyespace_matrix_M(_Mat result) {
  /* Calculate the matrix that takes shapes
   * from world space to eye space
   */

  // Local variables
  v3 l_eye = eye;
  v3 l_upp = up_point;
  v3 l_coi = center_of_interest;


  // Move the observer to the origin
  _Mat observer_to_origin;
  Mat_translation_M(observer_to_origin, -eye[0], -eye[1], -eye[2]);

  // Emulate doing this to the existing stuff
  l_eye = v3_transform(l_eye, observer_to_origin);
  l_upp = v3_transform(l_upp, observer_to_origin);
  l_coi = v3_transform(l_coi, observer_to_origin);

  // Rotate observer so that the center of interest is on the y-z plane
  _Mat align_coi_1;
  const float theta1 = -atan2(l_coi[0], l_coi[2]);
  Mat_y_rotation_M(align_coi_1, theta1);

  // Emulate doing this to the existing stuff
  l_eye = v3_transform(l_eye, align_coi_1);
  l_upp = v3_transform(l_upp, align_coi_1);
  l_coi = v3_transform(l_coi, align_coi_1);

  // Rotate observer so that the center of interest is on the z-axis
  _Mat align_coi_2;
  const float theta2 = +atan2(l_coi[1], l_coi[2]);
  Mat_x_rotation_M(align_coi_2, theta2);

  // Emulate doing this to the existing stuff
  l_eye = v3_transform(l_eye, align_coi_2);
  l_upp = v3_transform(l_upp, align_coi_2);
  l_coi = v3_transform(l_coi, align_coi_2);

  // Rotate observer so that the up point is on the y-z plane
  _Mat align_up;
  const float theta3 = +atan2(l_upp[0], l_upp[1]);
  Mat_z_rotation_M(align_up, theta3);

  // Compose all the matrices
  Mat_chain_M(
    result, 4,
    observer_to_origin,
    align_coi_1,
    align_coi_2,
    align_up
  );
}

#endif // eye_h_INCLUDED

