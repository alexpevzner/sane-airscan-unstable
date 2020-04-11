#!/bin/bash

gcc -o airscan-image airscan-image.c ../airscan-bmp.c ../airscan-tiff.c  $(pkg-config --libs --cflags gtk+-3.0  libtiff-4) -lm -I./ -I../

