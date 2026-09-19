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


    for (int i=0;i<bufsize;++i)
    {
        unsigned char r=red[i]>>3;
        unsigned char g=green[i]>>3;
        unsigned char b=blue[i]>>3;

        uint16_t writeval=(uint16_t)(r)<<10|(uint16_t)(g)<<5|(uint16_t)(b);

        fbuf[i*2+0]=writeval&0xff;
        fbuf[i*2+1]=writeval>>8;
    }

    free(red);free(green);free(blue);

    getch();
    RestoreVideo();
    return 0;
}
