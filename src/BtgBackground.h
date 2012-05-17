//============================================================================
//
// Bgt is a fileformat introduced by homeworld for doing goraud shaded
// backgrounds (looks very stylish!). The code is copied straight from
// the homeworld engine which is opensourced.
//
//============================================================================


#pragma once
#include <string>
#include <util/shared_ptr.h>
#include <util/types.h>
#include <gfx/Camera.h>
#include <gfx/fwd/Geometry.h>
#include <gfx/fwd/Shader.h>



#define BTG_FILE_VERSION 0x600


/*=============================================================================
    Data:
=============================================================================*/
extern char btgLastBackground[128];
extern real32 btgFieldOfView;






BASE_DECL_SMARTPTR_STRUCT(BtgBackground);
struct BtgBackground
{
	BtgBackground();






	static BtgBackgroundPtr create();



	void render( base::CameraPtr cam );
	void       setAlpha( float alpha );






	// straight

	// force byte struct boundary aligment so that we can deal with btg stuff from homeworld
	#pragma pack(push, 4)
	// -----
	// structures
	// -----

	typedef struct btgHeader
	{
		udword  btgFileVersion;
		udword  numVerts;
		udword  numStars;
		udword  numPolys;
		sdword  xScroll, yScroll;
		real32  zoomVal;
		sdword  pageWidth, pageHeight;
		sdword  mRed, mGreen, mBlue;
		sdword  mBGRed, mBGGreen, mBGBlue;
		int  bVerts, bPolys, bStars, bOutlines, bBlends;
		sdword  renderMode;
	} btgHeader;

	typedef struct btgVertex
	{
		udword  flags;
		real64  x, y;
		sdword  red, green, blue, alpha, brightness;
	} btgVertex;


	typedef struct btgStar
	{
		udword  flags;
		real64  x, y;
		sdword  red, green, blue, alpha;
		char    filename[48];
		udword  glhandle;
		sdword  width, height;
	} btgStar;

	typedef struct btgPolygon
	{
		udword  flags;
		udword  v0, v1, v2;
	} btgPolygon;

	#pragma pack(pop)

	// -----
	// prototypes
	// -----

	void btgStartup(void);
	void btgReset(void);
	void btgShutdown(void);
	void btgSetTheta(real32 theta);
	void btgSetPhi(real32 theta);
	real32 btgGetTheta(void);
	real32 btgGetPhi(void);
	void btgCloseTextures(void);
	void btgLoadTextures(void);
	void btgLoad( const std::string filename );
	void btgConvertVerts(void);
	void btgRender(void);
	void btgRender2(void);
	void btgSetColourMultiplier(real32 t);


	base::GeometryPtr                 m_domeGeo;
	base::ShaderPtr         m_simpleColorShader;

	float                               m_alpha;
};