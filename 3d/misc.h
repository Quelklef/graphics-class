#ifndef misc_h_INCLUDED
#define misc_h_INCLUDED

double sgn(double x) {
  if (x < 0) return -1;
  if (x > 0) return +1;
  return 0;
}

#endif // misc_h_INCLUDED

