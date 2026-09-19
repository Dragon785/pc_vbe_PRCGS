// PRCGSƒwƒbƒ_’è‹`
#if (!defined(__PRCGS_DEF_H__))
#define __PRCGS_DEF_H__

// #pragma align(1)
#define PRCGS_HEADER_SIZE 128

typedef struct tag_PRCGS
{
	unsigned char hdr[3]; // 'P_3'
	unsigned char ver[2]; // Encoder Version
	unsigned char datMachine[16]; // Encoder Machine 
    unsigned char appAuthor[8]; // Amateur Radio Callsign of Crete Encoder
	unsigned char datAuthor[8]; // Amateur Radio Callsign of Create Data
	unsigned char createYMD[8]; // When Create Year-Month-Dath(Text)
	unsigned char createHMS[8]; // When Create Hour-Minute-Second(Text)
	unsigned char width[2]; // Pixel Width (Big Endian)
	unsigned char height[2]; // Pixel Height (Big Endian)
	unsigned char arc; // Data is Compressed when 1
	unsigned char length[7]; // Runlength Table:0 means length 1.
	unsigned char mono; // Data is Monochrome?(1plane)
	unsigned char reserved[62]; // reserved
} PRCGSHeader /* __attribute__((__packed__)) */;


#endif
