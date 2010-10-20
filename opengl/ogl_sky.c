
//**************************************************************************
//**
//** OGL_SKY.C
//**
//** $Revision$
//** $Date$
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "h2stdinc.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include <math.h>
#include <GL/gl.h>
#include "doomdef.h"
#include "r_local.h"
#include "ogl_def.h"
#include "ogl_rl.h"

// MACROS ------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern float		texw, texh;

extern float		vx, vy, vz;

extern byte		topLineRGB[3];

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int			skytexture;
int			skyflatnum;
int			skytexturemid;
fixed_t			skyiscale;

int			skyDetail = 1;
int			skyhemispheres;
fadeout_t		fadeOut[2];	/* For both skies. */

// PRIVATE DATA DEFINITIONS ------------------------------------------------

/* The texture offset to be applied to the texture coordinates in SkyVertex(). */
static float		maxSideAngle = (float) PI/3;
static float		texoff;
static int		rows, columns;
static float		scale = 32;	/* Fogging affects, thus close-by. */
static boolean		yflip;
static fadeout_t	*currentFO;

// CODE --------------------------------------------------------------------

// Calculate the vertex and texture coordinates.
static void SkyVertex(int r, int c)
{
	// The direction must be clockwise.
	float topAngle = c/(float)columns * 2*PI;
	float sideAngle = maxSideAngle * (rows - r)/(float)rows;
	float height = sin(sideAngle);
	float realRadius = scale*cos(sideAngle);
	float x = vx + realRadius*cos(topAngle), 
		z = vz + realRadius*sin(topAngle),
		y = vy + ((!yflip) ? scale*height : -scale*height);

	// And the texture coordinates.
	if (!yflip)	// Flipped Y is for the lower hemisphere.
		glTexCoord2f(4*c/(float)columns + texoff/texw, r/(float)rows*200.0/256.0);
	else
		glTexCoord2f(4*c/(float)columns + texoff/texw, (rows-r)/(float)rows*200.0/256.0);
	// Also the color.
	if (currentFO->use)
	{
		if (r == 0)
			glColor4f(1, 1, 1, 0);
		else
			glColor3f(1, 1, 1);
	}
	else
	{
		if (r == 0)
			glColor3f(0, 0, 0);
		else
			glColor3f(1, 1, 1);
	}
	// And finally the vertex.
	glVertex3f(x, y, z);
}

static void CapSideVertex(int r, int c)
{
	// The direction must be clockwise.
	float topAngle = c/(float)columns * 2*PI;
	float sideAngle = maxSideAngle * (rows-r)/(float)rows;
	float height = sin(sideAngle);
	float realRadius = scale*cos(sideAngle);

	glVertex3f(vx + realRadius*cos(topAngle),
		   vy + ((!yflip) ? scale*height : -scale*height),
		   vz + realRadius*sin(topAngle));
}

// Hemi is Upper or Lower. Zero is not acceptable.
// The current texture is used. SKYHEMI_NO_TOPCAP can be used.
void OGL_RenderSkyHemisphere(int hemi)
{
	int		r, c;

	if (hemi & SKYHEMI_LOWER)
		yflip = true;
	else
		yflip = false;

	// The top row (row 0) is the one that's faded out.
	// There must be at least 4 columns. The preferable number
	// is 4n, where n is 1, 2, 3... There should be at least
	// two rows because the first one is always faded.
	rows = 3;
	columns = 4*skyDetail;	// 4n
	if (hemi & SKYHEMI_JUST_CAP)
	{
		glDisable(GL_TEXTURE_2D);

		// Use the appropriate color.
		if (currentFO->use)
			glColor3fv(currentFO->rgb);
		else
			glColor3f(0, 0, 0);
		glBegin(GL_TRIANGLE_FAN);
		for (c = 0; c < columns; c++)
			CapSideVertex(0, c);
		glEnd();

		// If we are doing a colored fadeout...
		if (currentFO->use)
		{
			// We must fill the background for the top row since it'll
			// be partially translucent.
			glBegin(GL_TRIANGLE_STRIP);
			CapSideVertex(0, 0);
			for (c = 0; c < columns; c++)
			{
				CapSideVertex(1, c);	// One step down.
				CapSideVertex(0, c+1);	// And one step right.
			}
			CapSideVertex(1, c);
			glEnd();
		}

		glEnable(GL_TEXTURE_2D);
		return;
	}

	// The total number of triangles per hemisphere can be calculated
	// as follows: rows * columns * 2 + 2 (for the top cap).
	for (r = 0; r < rows; r++)
	{
		glBegin(GL_TRIANGLE_STRIP);
		// Add the start vertex.
		SkyVertex(r, 0);
		for (c = 0; c < columns; c++)
		{
			SkyVertex(r+1, c);	// One step down.
			SkyVertex(r, c+1);	// And one step right.
		}
		// Add the end vertex for this row. This vertex is also
		// the start vertex for the next row.
		SkyVertex(r+1, c);
		glEnd();
		/*
		glBegin(GL_QUADS);
		for (c = 0; c < columns; c++)
		{
			SkyVertex(r, c);
			SkyVertex(r+1, c);
			SkyVertex(r+1, c+1);
			SkyVertex(r, c+1);
		}
		glEnd();
		*/
	}
}

void OGL_HandleColoredFadeOut(int skynum)
{
	int			i;

	currentFO = fadeOut + skynum - 1;
	// We have to remember the top line average RGB.
	if (!currentFO->set)
	{
		currentFO->set = 1;	// Now it is.
		for (i = 0; i < 3; i++)
			currentFO->rgb[i] = topLineRGB[i]/255.0;
		// Determine if it should be used.
		for (currentFO->use = false, i = 0; i < 3; i++)
		{
			if (currentFO->rgb[i] > .3)
			{
				// Colored fadeout is needed.
				currentFO->use = true;
				break;
			}
		}
	}
}

static void OGL_DrawSkyhemi(int whichHemi)
{
	GLuint skyname;

	skyname = OGL_PrepareSky(skytexture, false);

	OGL_HandleColoredFadeOut(1);

	// First the top cap.
	OGL_RenderSkyHemisphere(whichHemi | SKYHEMI_JUST_CAP);
	glBindTexture(GL_TEXTURE_2D, skyname);

	// Render the front sky (sky1).
	OGL_RenderSkyHemisphere(whichHemi);
}

void R_RenderSkyHemispheres(int hemis)
{
	// IS there a sky to be rendered?
	if (!hemis)
		return;

	// We don't want anything written in the depth buffer, not yet.
	glDepthMask(GL_FALSE);
	glPushAttrib(GL_ENABLE_BIT);
	// Disable culling, all triangles face the viewer.
	glDisable(GL_CULL_FACE);
	glDisable(GL_FOG);
	// Draw the possibly visible hemispheres.
	if (hemis & SKYHEMI_UPPER)
		OGL_DrawSkyhemi(SKYHEMI_UPPER);
	if (hemis & SKYHEMI_LOWER)
		OGL_DrawSkyhemi(SKYHEMI_LOWER);
	// Enable the disabled things.
	glPopAttrib();
	glDepthMask(GL_TRUE);
}


// THESE COPIED OVER FROM R_PLANES.C ---------------------------------------

// R_InitSkyMap - Called whenever the view size changes.
void R_InitSkyMap (void)
{
	skyflatnum = R_FlatNumForName("F_SKY1");
	skytexturemid = 200*FRACUNIT;
	skyiscale = FRACUNIT;
}

// R_InitPlanes - Called at game startup.
void R_InitPlanes(void)
{
}

