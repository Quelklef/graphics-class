#ifndef dyn_h_INCLUDED
#define dyn_h_INCLUDED

// Generic variable-length list

// Set the macro DYN_TYPE to be the type of the list
// then include this file and call DYN_INIT(NAME, TYPE)
// For instance,
//   #include "dyn.h"
//   DYN_INIT(iArray, int)
// generates stuff like
//   iArray iArray_new() ;
// and
//   void iArray_append(int x);


typedef struct Dyn {

  // Content
  void *items;

  // Size of contained type
  // (in bytes)
  size_t type_size;

  // Size to reset to
  // (in multiples of type_size)
  size_t starting_size;

  // Number of contained items
  size_t length;

  // Amount of memory allocated
  // (in multiples of type_size)
  size_t size;

} Dyn;


static void Dyn_resize(Dyn *dyn, const size_t new_size) {
  dyn->items = realloc(dyn->items, new_size * dyn->type_size);
  dyn->size = new_size;
}

static void Dyn_clear(Dyn *dyn) {
  Dyn_resize(dyn, dyn->starting_size);
  dyn->length = 0;
}

static void Dyn_init(Dyn *dyn, const size_t starting_size, const size_t type_size) {
  dyn->type_size = type_size;
  dyn->starting_size = starting_size;

  dyn->length = 0;
  dyn->items = malloc(starting_size * dyn->type_size);
  dyn->size = starting_size;
}

static Dyn *Dyn_new(const size_t starting_size, const size_t type_size) {
  Dyn *dyn = malloc(sizeof(Dyn));
  Dyn_init(dyn, starting_size, type_size);
  return dyn;
}

static void Dyn_set_(Dyn *dyn, const size_t idx, const void *item) {
  /* Dyn_set with no checks */
  memcpy(dyn->items + idx * dyn->type_size, item, dyn->type_size);
}

static void Dyn_set(Dyn *dyn, const size_t idx, const void *item) {
  if (idx < 0 || idx >= dyn->length) {
    printf("dyn access out of bounds\n");
    exit(1);
  }
  Dyn_set_(dyn, idx, item);
}

static void Dyn_append(Dyn *dyn, void *item) {
  if (dyn->length == dyn->size) {
    Dyn_resize(dyn, 2 * dyn->size);
  }

  Dyn_set_(dyn, dyn->length, item);
  dyn->length++;
}

/* Dyn_destroy is not automatically lifted
 * into the client's given type. This is to
 * force the client to write their own
 * destroy method to ensure that they're
 * not accidentally using the dyn shallow destroy
 * when they want a deep destructor.
 * Client destroy should call Dyn_destroy as
 * the last line.
 */
void Dyn_destroy(Dyn *dyn) {
  free(dyn->items);
  free(dyn);
}

#define DYN_INIT(NAME, TYPE) \
  typedef Dyn NAME; \
  void NAME ## _init   (Dyn *dyn, const size_t starting_size ) { return Dyn_init(dyn, starting_size, sizeof(TYPE));     } \
  Dyn* NAME ## _new    (const size_t starting_size           ) { return Dyn_new(starting_size, sizeof(TYPE));           } \
  void NAME ## _resize (Dyn *dyn, const size_t new_size      ) { return Dyn_resize(dyn, new_size);                      } \
  void NAME ## _clear  (Dyn *dyn                             ) { return Dyn_clear(dyn);                                 } \
  void NAME ## _append (Dyn *dyn, TYPE item                  ) { return Dyn_append(dyn, &item);                         } \
  TYPE NAME ## _get    (const Dyn *dyn, const size_t idx     ) { return *( (TYPE*) (dyn->items + idx * sizeof(TYPE)) ); } \
  void NAME ## _set    (Dyn *dyn, const size_t idx, TYPE item) { return Dyn_set(dyn, idx, &item);                       } \

#endif // dyn_h_INCLUDED
