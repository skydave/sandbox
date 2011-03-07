//
//
//
#pragma once

#include "../TOP.h"

#include "BlurTOP.h"
#include "ResampleTOP.h"
#include "DOFCoCComputeTOP.h"
#include "DOFBlurTOP.h"
#include "DOFCombineTOP.h"
//
//
//
struct DOFTOP
{
	DOFTOP();

	void setInputs( Texture *color, Texture *normal_depth );
	void setFocus( float focalLength, float fallOffStart, float fallOffEnd );

	void render( float time = 0.0f );

	DOFCoCComputeTOP *computeCOC;
	BlurTOP             *blurCOC;

	ResampleTOP *downresDepth;
	BlurTOP        *blurDepth;

	DOFBlurTOP       *blurMed;
	ResampleTOP *downresColor;
	DOFBlurTOP     *blurLarge;

	DOFCombineTOP *dofCombine;

	Attribute *focalLengthAttr;
	Attribute *fallOffStartAttr;
	Attribute *fallOffEndAttr;

	Texture *out0;
};
