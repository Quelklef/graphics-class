// Generic variable-length list

// Set the macro DYN_TYPE to be the type of the list
// then include this file and call DYN_INIT(DYN_NAME)
// For instance,
//   #define DYN_TYPE int
//   #include "dyn.h"
//   DYN_INIT(iArray)
// generates stuff like
//   iArray iArray_new() ;
// and
//   void iArray_append(int x);


typedef struct Dyn {

  // Content
  DYN_TYPE *items;

  // Size to reset to
  size_t starting_size;

  // Number of contained items
  size_t length;

  // Amount of memory allocated
  size_t size;

} Dyn;


static void Dyn_resize(Dyn *dyn, const size_t new_size) {
  dyn->items = realloc(dyn->items, new_size * sizeof(DYN_TYPE));
  dyn->size = new_size;
}

static void Dyn_clear(Dyn *dyn) {
  Dyn_resize(dyn, dyn->starting_size);
  dyn->length = 0;
}

static void Dyn_init(Dyn *dyn, const size_t starting_size) {
  dyn->starting_size = starting_size;
  dyn->length = 0;
  dyn->items = malloc(starting_size * sizeof(DYN_TYPE));
  dyn->size = starting_size;
}

static Dyn *Dyn_new(const size_t starting_size) {
  Dyn *dyn = malloc(sizeof(Dyn));
  Dyn_init(dyn, starting_size);
  return dyn;
}

static void Dyn_append(Dyn *dyn, DYN_TYPE item) {
  if (dyn->length == dyn->size) {
    Dyn_resize(dyn, 2 * dyn->size);
  }

  dyn->items[dyn->length] = item;
  dyn->length++;
}

static DYN_TYPE Dyn_get(const Dyn *dyn, const size_t idx) {
  return dyn->items[idx];
}

static void Dyn_destroy(Dyn *dyn) {
  free(dyn->items);
  free(dyn);
}

#define DYN_INIT(NAME) \
  typedef Dyn NAME; \
  Dyn*     NAME ## _new    (const size_t starting_size          ) { return Dyn_new(starting_size);    } \
  Dyn*     NAME ## _init   (Dyn *dyn, const size_t starting_size) { return Dyn_new(starting_size);    } \
  void     NAME ## _resize (Dyn *dyn, const size_t new_size     ) { return Dyn_resize(dyn, new_size); } \
  void     NAME ## _clear  (Dyn *dyn                            ) { return Dyn_clear(dyn);            } \
  void     NAME ## _append (Dyn *dyn, DYN_TYPE item             ) { return Dyn_append(dyn, item);     } \
  DYN_TYPE NAME ## _get    (const Dyn *dyn, const size_t idx    ) { return Dyn_get(dyn, idx);         } \
  void     NAME ## _destroy(Dyn *dyn                            ) { return Dyn_destroy(dyn);          }
