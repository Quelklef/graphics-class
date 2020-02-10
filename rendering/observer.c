#ifndef observer_c_INCLUDED
#define observer_c_INCLUDED

#include <math.h>

#include "../shapes/v3.c"
#include "../matrix.c"

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
  _Mat observer_to_origin = Mat_translate_v(-eye);

  // Emulate doing this to the existing stuff
  l_eye = v3_transform(l_eye, observer_to_origin);
  l_upp = v3_transform(l_upp, observer_to_origin);
  l_coi = v3_transform(l_coi, observer_to_origin);

  // Rotate observer so that the center of interest is on the y-z plane
  const float theta1 = -atan2(l_coi[0], l_coi[2]);
  const _Mat align_coi_1 = Mat_y_rot(theta1);

  // Emulate doing this to the existing stuff
  l_eye = v3_transform(l_eye, align_coi_1);
  l_upp = v3_transform(l_upp, align_coi_1);
  l_coi = v3_transform(l_coi, align_coi_1);

  // Rotate observer so that the center of interest is on the z-axis
  const float theta2 = +atan2(l_coi[1], l_coi[2]);
  const _Mat align_coi_2 = Mat_x_rot(theta2);

  // Emulate doing this to the existing stuff
  l_eye = v3_transform(l_eye, align_coi_2);
  l_upp = v3_transform(l_upp, align_coi_2);
  l_coi = v3_transform(l_coi, align_coi_2);

  // Rotate observer so that the up point is on the y-z plane
  const float theta3 = +atan2(l_upp[0], l_upp[1]);
  const _Mat align_upp = Mat_z_rot(theta3);

  // Compose all the matrices
  Mat_chain_M(
    result, 4,
    observer_to_origin,
    align_coi_1,
    align_coi_2,
    align_upp
  );
}

#endif // observer_c_INCLUDED

