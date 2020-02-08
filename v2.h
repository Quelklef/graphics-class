#ifndef v2_h_INCLUDED
#define v2_h_INCLUDED

typedef float v2 __attribute__ (( vector_size(2 * sizeof(float)) ));

void v2_print(v3 v) {
  printf("v2 [ %f, %f ] v2", v[0], v[1]);
}

#endif // v2_h_INCLUDED

