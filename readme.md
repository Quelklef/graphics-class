graphics-class
==============

Repo for my graphics class.

To run, execute `./build.sh main.c && ./a.out`. Play around with it for a bit.

Some predefind shapes are included in `xyz/`. To place them in the world, run like `./a.out xyz/<shape1>.xyz xyz/<shape2>.xyz ...`.

Project structure:
- `main.c` is the top-level file
- `matrix.c` is matrix code
- `globals.c` is most of the program state. Some also exists in `main.c`, `controls.c`, and `rendering/observer.c`.
- `controls.c` is for handling user input
- `libgfx/` contains an X11 wrapper that my professor supplied us. The main entry point is `libgfx/libgfx.h`. This code is very lightly modified by me from my professor's source. I mostly removed unused files, moved things around, and renamed it.
- `rendering/` contains rendering code:
  - `observer.c` is for transforming figures from world space into eye space
  - `draw.c` is low-level pixel drawing code
  - `render.c` is the bulk of the figure rendering code
- `shapes/` contains code for representing 2d and 3d objects:
  - `v2.c` is a 2d vector
  - `v3.c` is a 3d vector
  - `line.c` is a line embedded in 3d space
  - `plane.c` is a 2d plane embedded in 3d space
  - `polygon.c` is a polygon
  - `lattice.c` is a 2D square lattice deformed into a 3D shape
  - `polyhedron.c` is a collection of polygons
  - `intersector.c` is a representation of a shape as a function that takes a line and returns all intersections between the shape and that line
  - `figure.c` is a union type that combines loci, polyhedra, and intersectors.
- `util/` contains miscellaneous code
  - `dyn.c` is a generic-type variable-length heap-allocated list
  - `misc.c` is other miscellaneous stuff
- `xyz/` contains specifications of 3d shapes. Run `./a.out xyz/<name>.xyz` to place one of these shapes in the world.

Note: some functions are suffixed with `_M`. This is to note that they return a value via a passed-in pointer rather than via C return functionality. The relevant pointers will always be the first parameter(s) of the function. For instance,
```c
void add_M(double *result, double x, double y) {
  *result = x + y;
}
```

Note: I do not use header files in this project. Instead, I `#include` the `.c` source files. I recognize that this is (a) counter to typical C-programming style and (b) has some issues, such as allowing files to get away with not `#include`-ing all their dependencies. I may create header files in the future; however, for now, this is just easier for me.
