#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <conio.h>

#include "vbe32.h"
#include "prcgs.h"

int main(int argc,char* argv[])
{
    if (argc!=2)
    {
        printf("Usage: lookvbe prcfile\n");
        return -1;
    }
    if (StartReadPRCGS(argv[1]))
    {
        return -1;
    }

    unsigned int width=GetWidth();
    unsigned int height=GetHeight();
    if ((width>640)||(height>240))
    {
        printf("Picture Size over 640*480\n");
        return -1;
    }
    int bufsize=width*height;
    unsigned char* red=malloc(bufsize);
    unsigned char* green=malloc(bufsize);
    unsigned char* blue=malloc(bufsize);

    if ((!red)||(!green)||(!blue))
    {
        printf("Buffer Allocate Failed\n");
        free(red);free(green);free(blue);

        return -1;
    }
    printf("Picture Size(%d,%d)\n",width,height);

    if (Extract(red,green,blue))
    {
        printf("Extract Failed\n");
        free(red);free(green);free(blue);
        return -1;
    }
    unsigned char* fbuf=GetVideoFrameBuffer(0x110); // VGA 32KCol
    if (!fbuf)
    {
        printf("VBE Initialize Failed\n");
        free(red);free(green);free(blue);
        return -1;
    }


	unsigned char* r=red;
	unsigned char* g=green;
	unsigned char* b=blue;

	int base=0;
    for (int y=0;y<height;++y)
    {
    	int w=base;
    	for (int x=0;x<width;++x)
    	{
    		unsigned char rr=*r++>>3;
    		unsigned char gg=*g++>>3;
    		unsigned char bb=*b++>>3;
	        uint16_t writeval=(uint16_t)(rr)<<10|
	        				(uint16_t)(gg)<<5|
	        				(uint16_t)(bb);

	        fbuf[w++]=writeval&0xff;
			fbuf[w++]=writeval>>8;
        }
        base+=640*2;
    }

    free(red);free(green);free(blue);

    getch();
    RestoreVideo();
    return 0;
}
