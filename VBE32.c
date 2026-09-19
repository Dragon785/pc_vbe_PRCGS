#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <i86.h>
#include <conio.h>

#pragma pack(push,1)
// DPMI structure for calling real-mode interrupts
typedef struct {
    unsigned long  edi;
    unsigned long  esi;
    unsigned long  ebp;
    unsigned long  reserved;
    unsigned long  ebx;
    unsigned long  edx;
    unsigned long  ecx;
    unsigned long  eax;
    unsigned short flags;
    unsigned short es, ds, fs, gs;
    unsigned short ip, cs, sp, ss;
}  DPMI_REGS;

// VBE Mode Information Structure (Partial for simplification)
typedef struct {
    unsigned short ModeAttributes;
    unsigned char  WinAAttributes, WinBAttributes;
    unsigned short WinGranularity, WinSize, WinSegmentA, WinSegmentB;
    unsigned long  WinFuncPtr;
    unsigned short BytesPerScanLine;
    unsigned short XResolution, YResolution;
    unsigned char  XCharSize, YCharSize, NumberOfPlanes, BitsPerPixel;
    unsigned char  NumberOfBanks, MemoryModel, BankSize, NumberOfImagePages;
    unsigned char  Reserved1;
    unsigned char  RedMaskSize, RedFieldPosition;
    unsigned char  GreenMaskSize, GreenFieldPosition;
    unsigned char  BlueMaskSize, BlueFieldPosition;
    unsigned char  RsvdMaskSize, RsvdFieldPosition;
    unsigned char  DirectColorModeInfo;
    unsigned long  PhysBasePtr; // Physical address of the linear frame buffer
    unsigned long  Reserved2[54];
} VBE_MODE_INFO;
#pragma pack(pop)

// Call Real Mode Interrupt via DPMI
static int dpmi_int(int int_num, DPMI_REGS* regs) {
    union REGS r;
    struct SREGS s;

    memset(&r, 0, sizeof(r));
    segread(&s);

    r.x.eax = 0x0300;         // DPMI Simulate Real Mode Interrupt
    r.x.ebx = int_num;
    r.x.ecx = 0;
    s.es = FP_SEG(regs);
    r.x.edi = FP_OFF(regs);

    int386x(0x31, &r, &r, &s);
    return r.x.cflag;         // Returns carry flag status
}

// Maps a physical memory address to a linear pointer in 32-bit Protected Mode
static void* map_physical_memory(unsigned long phys_addr, unsigned long size) {
    union REGS r;
    r.x.eax = 0x0800;         // DPMI Physical Address Mapping
    r.x.ebx = phys_addr >> 16;
    r.x.ecx = phys_addr & 0xFFFF;
    r.x.esi = size >> 16;
    r.x.edi = size & 0xFFFF;

    int386(0x31, &r, &r);

    if (r.x.cflag) return NULL;
    return (void*)((r.x.ebx << 16) | (r.x.ecx & 0xFFFF));
}

static void unmap_physical_memory(unsigned long phys_addr)
{
    union REGS r;
    r.x.eax=0x0801; // DPMI Unmap Address
    r.x.ebx=phys_addr>>16;
    r.x.ecx=phys_addr&0xffff;

    if (r.x.cflag)
    {
        printf("Warning: Unmap Physical Memory Failed\n");
    }

    int386(0x31,&r,&r);
}

unsigned char* framebuffer=NULL;
unsigned char* GetVideoFrameBuffer(unsigned short mode) {
    DPMI_REGS dregs;
    VBE_MODE_INFO* mode_info;
    unsigned long dos_buffer;
    int x, y;

    // 1. Allocate a chunk of low DOS memory (below 1MB) for VBE struct using DPMI
    union REGS r;
    r.x.eax = 0x0100; // DPMI Allocate DOS Memory Block
    r.x.ebx = (sizeof(VBE_MODE_INFO) + 15) >> 4; // Paragraphs
    int386(0x31, &r, &r);

    if (r.x.cflag) {
        printf("Failed to allocate real-mode memory block.\n");
        return NULL;
    }

    // Calculate linear and real-mode segment pointers
    dos_buffer = (r.x.eax & 0xFFFF) << 4;
    unsigned short dos_seg = r.x.eax & 0xFFFF;
    mode_info = (VBE_MODE_INFO*)dos_buffer;

    // 2. Query VBE Mode Info for 640x480x32-bit (Mode: 0x112)
    // We add 0x4000 to the mode ID to enable the Linear Frame Buffer flag
    unsigned short mode_id = mode | 0x4000;

    memset(&dregs, 0, sizeof(dregs));
    dregs.eax = 0x4F01;       // VBE Return Mode Information
    dregs.ecx = mode_id & 0x3FFF;
    dregs.es = dos_seg;
    dregs.edi = 0x0000;

    if (dpmi_int(0x10, &dregs) || (dregs.eax & 0xFFFF) != 0x004F) {
        printf("VBE Mode %x not supported or call failed.\n",mode);
        return NULL;
    }

    // Capture the physical address before clearing text screen output
    unsigned long phys_fb = mode_info->PhysBasePtr;
    unsigned long fb_size = mode_info->XResolution * mode_info->YResolution * 4;

    // 3. Set the VBE Video Mode
    memset(&dregs, 0, sizeof(dregs));
    dregs.eax = 0x4F02;       // VBE Set Mode
    dregs.ebx = mode_id;
    dpmi_int(0x10, &dregs);

    // 4. Map the physical framebuffer into our 32-bit memory space
    framebuffer=(unsigned char*)map_physical_memory(phys_fb, fb_size);

    return framebuffer;
}

void RestoreVideo(void)
{
    unmap_physical_memory((unsigned long)(framebuffer));

    DPMI_REGS dregs;

    memset(&dregs, 0, sizeof(dregs));
    dregs.eax = 0x0003;
    dpmi_int(0x10, &dregs);
}
