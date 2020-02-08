#ifndef misc_c_INCLUDED
#define misc_c_INCLUDED

float sgn(float x) {
  if (x < 0) return -1;
  if (x > 0) return +1;
  return 0;
}

#endif // misc_c_INCLUDED

