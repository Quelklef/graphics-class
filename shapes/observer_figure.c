#ifndef observer_figure_c_INCLUDED
#define observer_figure_c_INCLUDED

#include "v3.c"

// Special-case figure containing the information
// for the observer, which is given by a position,
// an interest (where the observer is looking),
// and an up point (where "up" is to the observer)

typedef struct Observer {
  v3 position;
  v3 interest;
  v3 up_point;
} Observer;

Observer *Observer_new() {
  Observer *ob = malloc(sizeof(Observer));
  ob->position = (v3) { 0, 0, 0 };
  ob->up_point = (v3) { 0, 1, 0 };
  ob->interest = (v3) { 0, 0, 1 };
  return ob;
}

void Observer_destroy(Observer *observer) {
  free(observer);
}

void Observer_transform(Observer *observer, const _Mat transformation) {
  observer->position = v3_transform(observer->position, transformation);
  observer->interest = v3_transform(observer->interest, transformation);
  observer->up_point = v3_transform(observer->up_point, transformation);
}

void Observer_bounds_M(v3 *lows, v3 *highs, const Observer *observer) {
  // The actual "area" the observer takes up is just the point of its position
  *lows = observer->position;
  *highs = observer->position;
}



#endif // observer_figure_c_INCLUDED
