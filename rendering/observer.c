#ifndef observer_c_INCLUDED
#define observer_c_INCLUDED

#include <math.h>

#include "../shapes/v3.c"
#include "../shapes/observer_figure.c"
#include "../matrix.c"

void calc_eyespace_matrix_M(_Mat result, const Observer *ob) {
  /* Calculate the matrix that takes shapes from world space to eye space */

  // Local copy of observer
  Observer clone;
  memcpy(&clone, ob, sizeof(Observer));

  // Move the observer to the origin
  _Mat observer_to_origin = Mat_translate_v(-ob->position);

  // Emulate doing this to the existing stuff
  Observer_transform(&clone, observer_to_origin);

  // Rotate observer so that the interest is on the y-z plane
  const float theta1 = -atan2(clone.interest[0], clone.interest[2]);
  const _Mat align_interest_1 = Mat_y_rot(theta1);

  // Emulate doing this to the existing stuff
  Observer_transform(&clone, align_interest_1);

  // Rotate observer so that the interest is on the z-axis
  const float theta2 = +atan2(clone.interest[1], clone.interest[2]);
  const _Mat align_interest_2 = Mat_x_rot(theta2);

  // Emulate doing this to the existing stuff
  Observer_transform(&clone, align_interest_2);

  // Rotate observer so that the up point is on the y-z plane
  const float theta3 = +atan2(clone.up_point[0], clone.up_point[1]);
  const _Mat align_up_point = Mat_z_rot(theta3);

  // Compose all the matrices
  Mat_chain_M(
    result, 4,
    observer_to_origin,
    align_interest_1,
    align_interest_2,
    align_up_point
  );
}

#endif // observer_c_INCLUDED

