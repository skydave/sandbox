#include "DOFTOP.h"

#include "../FBO.h"


DOFTOP::DOFTOP() : out0(0)
{
	// create setup
	computeCOC = new DOFCoCComputeTOP();
	blurCOC = new BlurTOP();

	downresDepth = new ResampleTOP();
	blurDepth = new BlurTOP();

	blurMed = new DOFBlurTOP();
	downresColor = new ResampleTOP();
	blurLarge = new DOFBlurTOP();

	dofCombine = new DOFCombineTOP();

	focalLengthAttr = new Attribute(1);
	focalLengthAttr->appendElement( 5.0f );
	fallOffStartAttr = new Attribute(1);
	fallOffStartAttr->appendElement( 1.0f );
	fallOffEndAttr = new Attribute(1);
	fallOffEndAttr->appendElement( 4.0f );
}

void DOFTOP::setInputs( Texture *color, Texture *normal_depth )
{
	computeCOC->setInputs( normal_depth, focalLengthAttr, fallOffStartAttr, fallOffEndAttr );
	blurCOC->setInputs( computeCOC->out0 );

	// create downresed and blurred version of the depth map
	downresDepth->setInputs( normal_depth, normal_depth->m_xres/4, normal_depth->m_yres/4 );
	blurDepth->setInputs( downresDepth->out0 );

	blurMed->setInputs( color, computeCOC->out0 );

	blurLarge->setOutputs( computeCOC->out0 );
	blurLarge->setInputs( color, computeCOC->out0 );

	dofCombine->setInputs( color, normal_depth, blurMed->out0, blurLarge->out0, blurCOC->out0, blurDepth->out0, focalLengthAttr, fallOffStartAttr, fallOffEndAttr );

	//out0 = downres->out0;
	//out0 = computeCOC->out0;
	//out0 = blurCOC->out0;
	//out0 = blurMed->out0;
	//out0 = blurLarge->out0;
	out0 = dofCombine->out0;
}


void DOFTOP::render( float time )
{
	computeCOC->render();
	blurCOC->render();

	downresDepth->render();
	blurDepth->render();

	blurMed->render();
	blurLarge->render();

	dofCombine->render();
}

void DOFTOP::setFocus( float focalLength, float fallOffStart, float fallOffEnd )
{
	focalLengthAttr->setElement( 0, &focalLength );
	fallOffStartAttr->setElement( 0, &fallOffStart );
	fallOffEndAttr->setElement( 0, &fallOffEnd );
}
