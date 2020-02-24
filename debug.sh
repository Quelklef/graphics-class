#!/bin/sh
./build.sh -g -DDEBUG main.c && (echo "file ./a.out\nb exit" && cat) | gdb
