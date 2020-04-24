#!/bin/bash

gcc -o airscan-image airscan-image.c  ../airscan*.c ../sane_strstatus.c $(pkg-config --libs --cflags avahi-client avahi-glib libjpeg libsoup-2.4 libxml-2.0 libtiff-4 gtk+-3.0 libpng) -lm -g

