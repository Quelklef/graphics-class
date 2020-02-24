#!/bin/sh
./build.sh -g -DDEBUG
(echo "file ./a.out\nb exit" && cat) | gdb
