#ifndef __H2OPENGL__
#define __H2OPENGL__

#include "r_local.h"
#include <GL/gl.h>


#ifdef __linux__
#define _alloca alloca
#endif


enum {VX, VY};	// Vertex indices.

typedef struct // For dynamic lighting.
{
	int		use;				
	mobj_t	*thing;
	float	top, height;
} lumobj_t;

// ScreenBits is currently unused.
extern int screenWidth, screenHeight, screenBits;

void I_InitGraphics(void);
void I_ShutdownGraphics(void);

void OGL_InitRenderer(void);
void OGL_InitData(void);
void OGL_ResetData(void);

void OGL_SwitchTo3DState();
void OGL_Restore2DState(int step);	// Step 1: matrices, 2: attributes.
void OGL_UseWhiteFog(int yes);

float PointDist2D(float c[2]);

void R_RenderSprite(vissprite_t *spr);

// 2D drawing routines.
void OGL_DrawPatch_CS(int x, int y, int lumpnum);
void OGL_DrawPatch(int x, int y, int lumpnum);
void OGL_DrawFuzzPatch(int x, int y, int lumpnum);
void OGL_DrawAltFuzzPatch(int x, int y, int lumpnum);
void OGL_DrawShadowedPatch(int x, int y, int lumpnum);
void OGL_DrawRawScreen(int lump);	// Raw screens are 320 x 200.
void OGL_DrawLine(float x1, float y1, float x2, float y2, 
				  float r, float g, float b, float a);
void OGL_DrawRect(float x, float y, float w, float h, float r, float g, float b, float a);
void OGL_DrawRectTiled(int x, int y, int w, int h, int tw, int th);
void OGL_DrawCutRectTiled(int x, int y, int w, int h, int tw, int th, 
						  int cx, int cy, int cw, int ch);
void OGL_SetColor(int palidx);
void OGL_SetColorAndAlpha(float r, float g, float b, float a);
void OGL_DrawPSprite(int x, int y, float scale, int flip, int lump);

// Filters.
void OGL_SetFilter(int filter);
int OGL_DrawFilter();


// ogl_tex.c
typedef struct
{
	unsigned short	w, h;
	short			offx, offy;
	unsigned short	w2;		// For split textures, width of the other part.
} texsize_t;

extern texsize_t *lumptexsizes;	// Sizes for all the lumps. 
extern unsigned short *spriteheights;

extern float		texw, texh;
extern int			texmask;	
extern unsigned int	curtex;
extern int			pallump;

int FindNextPower2(int num);
float NextPower2Ratio(int num);
void OGL_TexInit();
void OGL_TexReset();
void OGL_ResetLumpTexData();
void PalToRGB(byte *palidx, byte *rgb);
void PalIdxToRGB(byte *pal, int idx, byte *rgb);
unsigned int OGL_BindTexFlat(int lump);
void OGL_SetFlat(int idx);
void OGL_BindTexture(GLuint texname);

// Returns the OpenGL texture name.
GLuint OGL_PrepareTexture(int idx);
GLuint OGL_PrepareFlat(int idx);// Returns the OpenGL name of the texture.
GLuint OGL_PrepareLightTexture();	// The dynamic light map.

void OGL_SetTexture(int idx);
unsigned int OGL_PrepareSky(int idx, boolean zeroMask);
void GL_GetSkyTopColor(int texidx, byte *rgb);

void OGL_SetSprite(int pnum);
unsigned int OGL_PrepareSprite(int pnum);
void OGL_NewSplitTex(int lump, GLuint part2name);
GLuint OGL_GetOtherPart(int lump);

// Part is either 1 or 2. Part 0 means only the left side is loaded.
// No splittex is created in that case. Once a raw image is loaded
// as part 0 it must be deleted before the other part is loaded at the
// next loading.
void OGL_SetRawImage(int lump, int part);
void OGL_SetPatch(int lump);	// No mipmaps are generated.
void OGL_SetNoTexture();

int OGL_GetLumpTexWidth(int lump);
int OGL_GetLumpTexHeight(int lump);
int OGL_ValidTexHeight2(int width, int height);

void OGL_UpdateTexParams(int mipmode);
void OGL_UpdateRawScreenParams(int smoothing);


#include "m_bams.h"

// ogl_clip.c
typedef struct clipnode_s
{
	int					used;			// 1 if the node is in use.
	struct clipnode_s	*prev, *next;	// Previous and and nodes.
	binangle			start, end;		// The start and end angles (start < end).
} clipnode_t;

extern clipnode_t *clipnodes;	// The list of clipnodes.
extern clipnode_t *cliphead;	// The head node.

void C_Init();
void C_ClearRanges();
void C_Ranger();
void C_SafeAddRange(binangle startAngle, binangle endAngle);

// Add a segment relative to the current viewpoint.
void C_AddViewRelSeg(float x1, float y1, float x2, float y2);

// Check a segment relative to the current viewpoint.
int C_CheckViewRelSeg(float x1, float y1, float x2, float y2);

// Returns 1 if the specified angle is visible.
int C_IsAngleVisible(binangle bang);

clipnode_t *C_AngleClippedBy(binangle bang);

// Returns 1 if the subsector might be visible.
int C_CheckSubsector(subsector_t *ssec);


// ogl_sky.c

// Sky hemispheres.
#define SKYHEMI_UPPER		0x1
#define SKYHEMI_LOWER		0x2
#define SKYHEMI_JUST_CAP	0x4	// Just draw the top or bottom cap.
#define SKYHEMI_FADEOUT_BG	0x8	// Draw the fadeout bg when drawing the cap.

typedef struct
{
	float	rgb[3];				// The RGB values.
	short	set, use;			// Is this set? Should be used?
	float	limit;				// .3 by default.
} fadeout_t;

#define SLF_ENABLED		0x1		// Layer enabled.
#define SLF_MASKED		0x2		// Mask the layer texture.

typedef struct
{
	int			flags;
	int			texture;		// Not the OpenGL texture.
	float		offset;
	fadeout_t	fadeout;
} skylayer_t;

void R_RenderSky(int hemis);
void R_SkyInit();

#endif
