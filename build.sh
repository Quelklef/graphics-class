#!/bin/bash
command="clang -I./libgfx -Werror $@ main.c ./libgfx/*.o -lm -lX11"
echo "build command: $command"
eval "$command"

