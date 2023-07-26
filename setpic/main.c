/*
    Send a png to a sst2205 unit using libst2205.
    Copyright (C) 2008 Jeroen Domburg <jeroen@spritesmods.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/



#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <gd.h>
#include <string.h>

#include <st2205.h>


/*
This routine reads a png-file from disk and puts it into buff in a format
lib understands. (basically 24-bit rgb)
*/
int sendpic(st2205_handle *h, char* filename) {
    FILE *in;
    gdImagePtr im;
    char* pixels;
    int x,y;
    int p=0;
    int c,r,g,b;
    
    pixels=malloc(h->width*h->height*3);
    
    in=fopen(filename,"rb");
    if (in==NULL) return 0;
    //Should try other formats too
    im=gdImageCreateFromPng(in);
    fclose(in);
    if (im==NULL) {
	printf("%s: not a png-file.\n",filename);
	return 0;
    }

    for (y=0; y<h->width; y++) {
	for (x=0; x<h->height; x++) {
	    c = gdImageGetPixel(im, x, y);
	    if (gdImageTrueColor(im) ) {
		r=gdTrueColorGetRed(c);
		g=gdTrueColorGetGreen(c);
		b=gdTrueColorGetBlue(c);
	    } else {
		r=gdImageRed(im,c);
		g=gdImageGreen(im,c);
		b=gdImageBlue(im,c);
	    }
	    pixels[p++]=r;
	    pixels[p++]=g;
	    pixels[p++]=b;
	}
    }
    st2205_send_data(h,pixels);
    gdImageDestroy(im);
    return 1;
}


int main(int argc, char **argv) {
    st2205_handle *h;
    int y;
    if (argc<2) {
	printf("Usage: %s /dev/sdX pic.png\n",argv[0]);
	exit(0);
    }
    h=st2205_open(argv[1]);
    if (h==NULL) {
	printf("Open failed!\n");
	exit(1);
    }
    printf("Found device: %ix%i, %i bpp\n",h->width,h->height,h->bpp);

    y=sendpic(h,argv[2]);
    if (!y) {
        //p'rhaps a dir?
	DIR *dir;
	struct dirent *dp;
	char fn[2048];
	dir=opendir(argv[2]);
	if (dir==NULL) {
	    //Nope :/
	    printf("Couldn't open %s.\n",argv[2]);
	    exit(1);
	}
	while ((dp = readdir (dir)) != NULL) {
	    strcpy(fn,argv[2]);
	    strcat(fn,"/");
	    strcat(fn,dp->d_name);
	    sendpic(h,fn);
	}
    }
    st2205_close(h);
    return(0);
}

