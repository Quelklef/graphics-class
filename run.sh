#!/bin/bash

# Call like e.g.
# ./run.sh -O3 -- xyz/sphere.xyz
# to pass '-O3' to clang and 'xyz/sphere.xyz' to the executable

compile_args=""
exec_args=""

state=compile_args
for arg in "$@"; do

  if [ "$arg" = "--" ]; then
    if [ "$state" = exec_args ]; then
      echo "Cannot have more than one '--' in CLI"
      exit
    fi
    state=exec_args;
  else   

    if [ "$state" = compile_args ]; then
      compile_args="$compile_args $arg"
    else
      exec_args="$exec_args $arg"
    fi

  fi

done

command="clang -I./libgfx $compile_args main.c ./libgfx/*.o -lm -lX11 && ./a.out $exec_args"
echo "command: $command"
eval "$command"

