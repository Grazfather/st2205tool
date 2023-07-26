/*
    ST2205U image library
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

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <asm/fcntl.h>
#include "st2205.h"
#include <sys/mman.h>

#define BUFF_SIZE 0x10000

/*
Two routines to allocate/deallocate page-aligned memory, for use with the
O_DIRECT-opened files.
*/

static void *malloc_aligned(long size) {
    int f;
    char *buff;
    f=open("/dev/zero",O_RDONLY);
    buff=mmap(0,size,PROT_READ|PROT_WRITE,MAP_PRIVATE,f,0);
    close(f);
    return buff;
}

static int free_aligned(void *addr, long size) {
    munmap(addr,size);
}


typedef struct {
    char sig[4];
    char version;
    unsigned char width;
    unsigned char height;
    char bpp;
    char proto;
    char offx;
    char offy;
} fw_descriptor;

/*
Checks if the device is a photo frame by reading the first 512 bytes and
comparing against the known string that's there
*/
static int is_photoframe(int f) {
    int y,res;
    char id[]="SITRONIX CORP.";
    char *buff;
    buff=malloc_aligned(0x200);
    lseek(f,0x0,SEEK_SET);
    y=read(f,buff,0x200);
    buff[15]=0;
//    fprintf(stderr,"ID=%s\n",buff);
    res=strcmp(buff,id)==0?1:0;
    free_aligned(buff,0x200);
    return res;
}

/*
The interface works by writing bytes to the raw 'disk' at certain positions.
Commands go to offset 0x6200, data to be read from the device comes from 0xB000,
data to be written goes to 0x6600. Hacked firmware has an extra address,
0x4200: bytes written there will go straight to the LCD.
*/

#define POS_CMD 0x6200
#define POS_WDAT 0x6600
#define POS_RDAT 0xb000

static int sendcmd(int f,int cmd, unsigned int arg1, unsigned int arg2, unsigned char arg3) {
    unsigned char *buff;
    buff=malloc_aligned(0x200);
    buff[0]=cmd;
    buff[1]=(arg1>>24)&0xff;
    buff[2]=(arg1>>16)&0xff;
    buff[3]=(arg1>>8)&0xff;
    buff[4]=(arg1>>0)&0xff;
    buff[5]=(arg2>>24)&0xff;
    buff[6]=(arg2>>16)&0xff;
    buff[7]=(arg2>>8)&0xff;
    buff[8]=(arg2>>0)&0xff;
    buff[9]=(arg3);
    lseek(f,POS_CMD,SEEK_SET);
    return write(f,buff,0x200);
}

static int read_data(int f, char* buff, int len) {
    lseek(f,POS_RDAT,SEEK_SET);
    return read(f,buff,len);
}

static int write_data(int f,char* buff, int len) {
    lseek(f,0x6600,SEEK_SET);
    return write(f,buff,len);
}

/*
Debugging routine to dump a buffer in a hexdump-like fashion.
*/
static void dumpmem(unsigned char* mem, int len) {
    int x,y;
    for (x=0; x<len; x+=16) {
	printf("%04x: ",x);
	for (y=0; y<16; y++) {
	    if ((x+y)>len) {
		printf("   ");
	    } else {
		printf("%02hhx ",mem[x+y]);
	    }
	}
	printf("- ");
	for (y=0; y<16; y++) {
	    if ((x+y)<=len) {
		if (mem[x+y]<32 || mem[x+y]>127) {
		    printf(".");
		} else {
		    printf("%c",mem[x+y]);
		}
	    }
	}
	printf("\n");
    }
}
#define FW_PAGE_OFFSET ((2048-64)/32)

static fw_descriptor *get_parm_block(int fd, char* buff) {
    int a,p;
    char lookfor[]="H4CK\000";
    //read 64K of firmware into buff
    sendcmd(fd,4,FW_PAGE_OFFSET,0x8000,0);
    read_data(fd,buff,0x8000);
    sendcmd(fd,4,FW_PAGE_OFFSET+1,0x8000,0);
    read_data(fd,buff+0x8000,0x8000);
    //look for 'H4CK' string
    for (a=0; a<0x10000-8; a++) {
	p=0;
	while (lookfor[p]!=0 && buff[a+p]==lookfor[p]) p++;
	if (lookfor[p]==0) {
	    return (fw_descriptor*)((int)buff+a);
	}
    }
    return NULL;
}



static int enddata(char *buff, int p) {
    int pageaddr,offset;
    offset=p&63;
    pageaddr=p-offset;
    if (offset==0) return;
    buff[pageaddr+1]=offset-1;
    p=pageaddr+64;
    return p;
}


static int pcf8833_setxy(st2205_handle *h,char *buff, int p, int xs, int xe, int ys, int ye) {
    p=enddata(buff,p);
    buff[p]=1;
    buff[p+1]=xs+h->offx;
    buff[p+2]=xe+h->offx;
    buff[p+3]=ys+h->offy;
    buff[p+4]=ye+h->offy;
    p+=64;
    return p;
}

static int adddata(char *buff, int p, char d) {
    if ((p&63)==0) {
	buff[p]=0;
	p+=2;
    }
    buff[p]=d;
    if ((p&63)==63) p=enddata(buff,p); else p++;
    return p;
}

#define PROTO_PCF8833 0

static unsigned int getpixel(st2205_handle *h, unsigned char *pix, int x, int y) {
    unsigned int r,a;
    if (x<0 || y<0 || x>=h->width || y>=h->height) return 0;
    a=(x+h->width*y)*3;
    r=pix[a]+(pix[a+1]<<8)+(pix[a+2]<<16);
    return r;
} 

static int write_stream(st2205_handle *h,char *buff,int len) {
    //pad to 512-byte boundary, with FFs
    while((len&511)!=0) {
	buff[len++]=0xff;
    }
//    printf("Writing 0x%x bytes.\n",len);
    lseek(h->fd,0x4400,SEEK_SET);
    return write(h->fd,buff,len);
}


//Sends image (xs,ys)-(xe,ye), inclusive.
static void pcf8833_send_partial(st2205_handle *h,unsigned char *pixinfo,int xs, int ys, int xe, int ye) {
    int p,x,y,z;
    unsigned int r,g,b,c;
    long tr;
    //if bpp=12, make width and xstart even
    if (h->bpp==12) {
	xs-=(xs&1);
	xe+=(xe-xs+1)&1;
    }
    p=0;
    p=pcf8833_setxy(h,h->buff,0,xs,xe,ys,ye);
    for (y=ys; y<=ye; y++) {
	for (x=xs; x<=xe; x++) {
	    if (h->bpp==16) {
		c=getpixel(h,pixinfo,x,y);
		r=(c&0xff); g=(c>>8)&0xff; b=(c>>16)&0xff;
		r>>=3; g>>=2; b>>=3;
		c=(r<<11)+(g<<5)+b;
		p=adddata(h->buff,p,(c>>8));
		p=adddata(h->buff,p,(c&255));
	    } else if (h->bpp==12) {
		tr=0;
		for (z=0; z<2; z++) {
		    c=getpixel(h,pixinfo,x+z,y);
		    r=(c&0xff); g=(c>>8)&0xff; b=(c>>16)&0xff;
		    r>>=4;g>>=4;b>>=4;
		    tr=(tr<<12)+(r<<8)+(g<<4)+(b);
		}
		p=adddata(h->buff,p,(tr>>16)&0xff);
		p=adddata(h->buff,p,(tr>>8)&0xff);
		p=adddata(h->buff,p,(tr)&0xff);
		x++; //because we handle 2 pixels at a time
	    } else {
		printf("Unknown bpp for this display: %i\n",h->bpp);
		exit(1);
	    }
	}
    }
    p=enddata(h->buff,p);
    write_stream(h,h->buff,p);
}


//pixinfo is a char array containing r,g,b triplets.
void st2205_send_data(st2205_handle *h,unsigned char *pixinfo) {
//    printf("libst2205: sending, ox=%i, oy=%i, bpp=%i\n",h->offx,h->offy,h->bpp);
    int x,y,xs,xe,ys,ye,c1,c2;
    if (h->proto==PROTO_PCF8833) {
	//PCF8833 has the possibility to do partial transfers into a certain bounding
	//box. Optimize for that by looking for the smallest bounding box containing
	//all changes.
	if (h->oldpix==NULL) {
	    pcf8833_send_partial(h,pixinfo,0,0,h->width-1,h->height);
	} else {
	    //go send incremental image
	    //Algorithm: go find biggest bounding box.
	    //It's semi-efficient: usually it works, but it could be that
	    //dividing the difference in multiple bounding boxes works better.
	    xe=0; ye=0; xs=h->width; ys=h->height;
	    for (x=0; x<h->width; x++) {
		for (y=0; y<h->height; y++) {
		    c1=getpixel(h,pixinfo,x,y);
		    c2=getpixel(h,h->oldpix,x,y);
		    if (c1!=c2) {
			if (x<xs) xs=x;
			if (y<ys) ys=y;
			if (x>xe) xe=x;
			if (y>ye) ye=y;
		    }
		}
	    }
	    pcf8833_send_partial(h,pixinfo,xs,ys,xe,ye);
	}
    } else {
	printf("Unrecognized protocol: 0x%x!\n",h->proto);
    }
    //store old buffer in case we need to calculate differences next.
    if (h->oldpix==NULL) {
	h->oldpix=malloc(h->width*h->height*3);
    }
    memcpy(h->oldpix,pixinfo,h->width*h->height*3);
}


void st2205_backlight(st2205_handle *h, int on) {
    //send command to turn bl on or off
    if (on) {
	h->buff[0]=2;
    } else {
	h->buff[0]=3;
    }
    write_stream(h,h->buff,1);
}

void st2205_close(st2205_handle *h) {
    close(h->fd);
    free_aligned(h->buff,BUFF_SIZE);
    if (h->oldpix!=NULL) free(h->oldpix);
    free(h);
}


st2205_handle *st2205_open(char *dev) {
    st2205_handle *r;
    fw_descriptor *b;
    r=malloc(sizeof(st2205_handle));
    r->fd=open(dev,O_RDWR|O_DIRECT);
    if (r->fd<0) {
	free(r);
	return NULL;
    }
    if (!is_photoframe(r->fd)) {
	close(r->fd);
	free(r);
	return(0);
    }
    r->buff=malloc_aligned(BUFF_SIZE);
    b=get_parm_block(r->fd,r->buff);
    if (b->version!=1) {
	printf("Unknown version %hhi\n",b->version);
	close(r->fd);
	free(r->buff);
	free(r);
	return(0);	
    }
    r->width=b->width;
    r->height=b->height;
    r->bpp=b->bpp;
    r->proto=b->proto;
    r->oldpix=NULL;
    r->offx=b->offx;
    r->offy=b->offy;
    return r;
}
