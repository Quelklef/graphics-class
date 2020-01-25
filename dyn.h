#ifndef dyn_h_INCLUDED
#define dyn_h_INCLUDED

// Dynamically-sized array

int starting_size = 8;

typedef struct Dyn {

  // Contained items
  void **items;

  // Amount of memory allocated
  size_t size;

  // Number of items containd
  int length;

} Dyn;

void Dyn_init(Dyn *dyn) {
  dyn->length = 0;
  dyn->size = starting_size * sizeof(void*);
  dyn->items = malloc(dyn->size);
}

Dyn *Dyn_new() {
  Dyn *result = malloc(sizeof(Dyn));
  Dyn_init(result);
  return result;
}

void Dyn_resize(Dyn *dyn, size_t new_size) {
  realloc(dyn->items, new_size);
  dyn->size = new_size;
}

void Dyn_append(Dyn *dyn, void *value) {
  if (dyn->length == dyn->size - 1) {
    Dyn_resize(dyn, 2 * dyn->size);
  }

  dyn->items[dyn->length] = &value;
  dyn->length++;
}

void Dyn_clear(Dyn *dyn) {
  dyn->length = 0;
  Dyn_resize(dyn, starting_size);
}

void *Dyn_get(Dyn *dyn, int idx) {
#ifdef DEBUG
  if (idx < 0 || idx >= dyn->length) {
    printf("Dyn_get called with out-of-bounds index.");
  }
#endif

  return dyn->items[idx];
}

#endif // dyn_h_INCLUDED

