/* sane - Scanner Access Now Easy.

   Copyright (C) 2020 Thierry HUCHARD <thierry@ordissimo.com>

   This file is part of the SANE package.

   SANE is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 3 of the License, or (at your
   option) any later version.

   SANE is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License
   along with sane; see the file COPYING.  If not, write to the Free
   Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   This file implements a SANE backend for airscan scanners. */

#include "../airscan.h"

// gcc -o airscan-bmp airscan-bmp.c $(pkg-config --libs --cflags gtk+-3.0) -lm

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <math.h>
#include <gtk/gtk.h>

static unsigned char* 
get_data(FILE *fp, int *size)
{
    unsigned char *data = (unsigned char *)calloc(1, sizeof(char));
	char buffer[1024] = { 0 };
    int nx = 0;
    while(!feof(fp))
    {
      int n = fread(buffer,sizeof(char),1024,fp);
      unsigned char *t = realloc(data, nx + n + 1);
      if (t == NULL) {
        fprintf(stderr, "not enough memory (realloc returned NULL)\n");
        return NULL;
      }
      data = t;
      memcpy(&(data[nx]), buffer, n);
      nx = nx + n;
      data[nx] = 0;
    }
    *size = nx;
    return data;
}

int main (int argc, char *argv[])
{
    int Width = 0, Height = 0;
    int bps = 0;
    int size = 0;
    image_window win;
    SANE_Parameters params;
    image_decoder*decodeur = NULL;
    int i = 0;
    FILE *fp = NULL;
    GtkWidget *window;
    GtkWidget *scrolled_window;
    GtkWidget* image;
    unsigned char *data = NULL;
    
    if (argc != 3) {
		printf("Usage %s : %s <bmp|tiff> <filename>\n", argv[0], argv[0]);
		exit(-1);
	}
    
    fp = fopen(argv[2], "rb");
    if (!fp) {
		printf("Error opening the %s file.\n", argv[2]);
		exit(-1); 
	}
    data = get_data(fp, &size);
    fclose (fp);

    gtk_init (&argc, &argv);

    if (!strcasecmp(argv[1], "bmp"))
        decodeur = image_decoder_bmp_new ();
    else if (!strcasecmp(argv[1], "tiff"))
        decodeur = image_decoder_tiff_new ();
    else {
		printf("Usage %s : %s <bmp|tiff> <filename>\n", argv[0], argv[0]);
		exit(-1); 
	}

    decodeur->begin (decodeur, (const void *)data, (size_t)size);
    decodeur->set_window (decodeur, &win);
    int bytes_per_pixel = decodeur->get_bytes_per_pixel (decodeur);
    decodeur->get_params (decodeur, &params);

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    fprintf(stdout, "Taille image [%dx%dx%d]\n", win.wid, win.hei, bytes_per_pixel);
    int size_data = (win.hei * params.bytes_per_line);
    unsigned char * buffer = (unsigned char*)calloc(1, sizeof(unsigned char) * (params.bytes_per_line + 1));
    int size_buf = 0;
    unsigned char *buf = (unsigned char *)calloc(1, sizeof (unsigned char) * (size_data + 1));
    unsigned char t = *buf;
    for (i = 0; i < win.hei; i++) {
       decodeur->read_line (decodeur, buffer);
       // Reverse writing of the image.
       /*
       size_buf += params.bytes_per_line;
       memcpy(&(buf[(size_data - size_buf)]), buffer, params.bytes_per_line);
       */
       // Writing the image. 
       memcpy(&(buf[size_buf]), buffer, params.bytes_per_line);
       size_buf += params.bytes_per_line;
    }
    buf[size_buf] = 0;
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_data (buf, GDK_COLORSPACE_RGB,
        (bytes_per_pixel == 4 ? TRUE : FALSE),
        8,
        win.wid, win.hei, params.bytes_per_line, NULL, NULL);

    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_container_add(GTK_CONTAINER (window), scrolled_window);
    
    gtk_window_set_title (GTK_WINDOW (window), "Image Viewer");

    g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

    image = gtk_image_new_from_pixbuf (pixbuf);

    gtk_container_add(GTK_CONTAINER (scrolled_window), image);

    gtk_widget_show_all (window);

    gtk_main ();

    return 0;
}
