/* PRCGS LOAD */
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>

#include "PRCGS.H"

static FILE* ReadFile=NULL;
PRCGSHeader Header;
static uint32_t fileSize=0;

int StartReadPRCGS(const char* filename)
{
	ReadFile=fopen(filename,"rb");
	if (ReadFile==NULL)
	{
		printf("Can't open %s\n",filename);
		return 1;
	}
	if (fread(&Header,sizeof(PRCGSHeader),1,ReadFile)<1)
	{
		printf("File is too small\n");
		fclose(ReadFile);
		ReadFile=NULL;
		return 1;
	}
	if ((Header.hdr[0]!='P')||(Header.hdr[1]!='_')||(Header.hdr[2]!='3'))
	{
		printf("Not PRCGS Data\n");
		fclose(ReadFile);
		ReadFile=NULL;
		return 1;
	}
	
	return 0;
}

const unsigned int GetWidth(void) // 現在のヘッダ情報に基づく幅
{
	return (Header.width[0]<<8)|(Header.width[1]);
}

const unsigned int GetHeight(void) // 現在のヘッダ情報に基づく高さ
{
	return (Header.height[0]<<8)|(Header.height[1]);
}

typedef enum tag_Plane
{
	PLANE_ALL=0, // 全プレーン一括書き込み
	PLANE_RED,
	PLANE_GREEN,
	PLANE_BLUE,
	PLANE_END
} Plane;


static void writeVarToPlane(Plane plane,unsigned char* r,unsigned char* g,unsigned char* b,unsigned char lvl,unsigned char length)
{
	switch(plane)
	{
		case PLANE_ALL:
			memset(r,lvl,length);
			memset(g,lvl,length);
			memset(b,lvl,length);
			break;
		case PLANE_RED:
			memset(r,lvl,length);
			break; 
		case PLANE_GREEN:
			memset(g,lvl,length);
			break; 
		case PLANE_BLUE:
			memset(b,lvl,length);
			break; 
		default:
			// do nothing
			break;
	}
}

#define FBUF_SIZE 1024
static unsigned char fbuf[FBUF_SIZE];

/* 指定されたバッファに8bit/pixelで展開する.バッファの確保と解放は呼出し側で面倒見ること
 0:正常終了 1:異常終了
 */
int Extract(unsigned char* r,unsigned char* g,unsigned char* b)
{
	if (!ReadFile)
	{
		printf("Not Open File\n");
		return 1;
	}
	unsigned char runLength[8];
	runLength[0]=1;
	for (int i=0;i<7;++i)
	{
		runLength[i+1]=Header.length[i]+1;
	}

	uint32_t writed=0;
	uint32_t planeSize=(uint32_t)GetWidth()*GetHeight();
	Plane plane=(Header.mono) ? PLANE_ALL : PLANE_RED;
	
	int readed=0;
	while ((readed=fread(fbuf,1,FBUF_SIZE,ReadFile))>0)
	{
		for (int c=0;c<readed;++c)
		{
			unsigned char dat=fbuf[c];
			unsigned char lvl=(dat&0x1f)<<3;
			lvl|=(lvl>>5);
			unsigned char length=runLength[dat>>5];

			if (writed+length>=planeSize)
			{
				unsigned char preWrite=planeSize-writed;
				length-=preWrite;
				writeVarToPlane(plane,r+writed,g+writed,b+writed,lvl,preWrite);
				if (plane==PLANE_ALL)
				{
					fclose(ReadFile);
					return 0;
				}
				else
				{
					plane++;writed=0;
					if (plane==PLANE_END)
					{
						fclose(ReadFile);
						return 0;
					}
				}
			}
			writeVarToPlane(plane,r+writed,g+writed,b+writed,lvl,length);
			writed+=length;
		}
	}

	fclose(ReadFile);
	if (writed!=planeSize)
	{
		printf("Maybe write miss?\n");
	}

	return 0;
}
