#!/bin/sh

gcc -o test test.c ../tinybox/common/stringop.c -I../include
./test ~/.sway/styles/Nyz