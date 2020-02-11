#!/bin/bash

if [ "$#" = 0 ]; then
  echo "Please give input files."
  exit
fi

clang -I./libgfx "$@" ./libgfx/*.o -lm -lX11

