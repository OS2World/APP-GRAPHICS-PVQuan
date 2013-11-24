typedef unsigned char PALETTE[256][3];
#define		RED	0
#define		GREEN	1
#define		BLUE	2

void        cdecl setvgapalette(PALETTE *palette, int colours);
void        cdecl WaitForVerticalRetrace(void);

void        cdecl putpixel_200(unsigned int x, unsigned int y, unsigned char colour);
void        cdecl set320x200x4mode(void);
void        cdecl set320x200x1mode(void);
void        cdecl end320x200mode(void);
void        cdecl setvis_200(int page);
char far *  cdecl setact_200(int page);

void        cdecl putpixel_400(unsigned int x, unsigned int y, unsigned char colour);
void        cdecl set320x400mode(void);
void        cdecl end320x400mode(void);
void        cdecl setvis_400(int page);
char far *  cdecl setact_400(int page);
