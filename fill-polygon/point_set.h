#ifndef point_set_INCLUDED
#define point_set_INCLUDED

#define POINT_SET_BUCKET_COUNT 256

int hash_point(double p[2]) {
  return (((int) p[0]) + 13 * ((int) p[1])) % POINT_SET_BUCKET_COUNT;
}

typedef struct PointSetItem {
  double point[2];
  struct PointSetItem *next;
} PointSetItem;

typedef struct {
  PointSetItem **buckets;
} PointSet;

void PointSet_put(PointSet *ps, double p[2]) {
  int hash = hash_point(p);
  PointSetItem *item = ps->buckets[hash];
  PointSetItem *prev = NULL;

  while (1) {
    if (item == NULL) break;
    if (point_eq(item->point, p)) return;  // Already contained
    prev = item;
    item = item->next;
  }

  PointSetItem *new = malloc(sizeof(PointSetItem));
  new->point[0] = p[0];
  new->point[1] = p[1];
  new->next = NULL;

  if (prev == NULL) {
    ps->buckets[hash] = new;
  } else {
    prev->next = new;
  }
}

int PointSet_contains(PointSet *ps, double p[2]) {
  PointSetItem *item = ps->buckets[hash_point(p)];

  while (1) {
    if (item == NULL) return 0;
    if (point_eq(item->point, p)) return 1;
    item = item->next;
  }
}

void PointSet_clear(PointSet *ps) {
  for (int b = 0; b < POINT_SET_BUCKET_COUNT; b++) {
    PointSetItem *head = ps->buckets[b];
    ps->buckets[b] = NULL;
    while (head != NULL) {
      PointSetItem *next = head->next;
      free(head);
      head = next;
    }
  }
}

PointSet *PointSet_create() {
  PointSet *ps = malloc(sizeof(PointSet));
  ps->buckets = malloc(POINT_SET_BUCKET_COUNT * sizeof(PointSetItem*));
  for (int b = 0; b < POINT_SET_BUCKET_COUNT; b++) {
    ps->buckets[b] = NULL;
  }
  return ps;
}

void PointSet_destroy(PointSet *ps) {
  PointSet_clear(ps);
  free(ps->buckets);
  free(ps);
}

int PointSet_empty(PointSet *ps) {
  for (int i = 0; i < POINT_SET_BUCKET_COUNT; i++) {
    if (ps->buckets[i] != NULL) return 0;
  }
  return 1;
}

#define PointSet_iterate(ps, var, code) \
  for (int b___ = 0; b___ < POINT_SET_BUCKET_COUNT; b___++) { \
    PointSetItem *item___ = ps->buckets[b___]; \
    while (item___ != NULL) { \
      double var[2]; \
      var[0] = item___->point[0]; \
      var[1] = item___->point[1]; \
      code ; \
      item___ = item___->next; \
    } \
  }

#endif // point_set_INCLUDED
