#include "TOP.h"
#include "FBO.h"





TOP::TOP( int xres, int yres ) : m_xres(xres), m_yres(yres), out0(0), out1(0), out2(0), out3(0)
{
	m_fbo = new FBO( m_xres, m_yres );
}

void TOP::setSize(int newXres, int newYres )
{
	if( (m_xres != newXres)||(m_yres != newYres) )
	{
		if(out0)
			out0->setSize( newXres, newYres );
		if(out1)
			out1->setSize( newXres, newYres );
		if(out2)
			out2->setSize( newXres, newYres );
		if(out3)
			out3->setSize( newXres, newYres );

		m_fbo->setSize(newXres, newYres);

		m_xres = newXres;
		m_yres = newYres;
	}
}


void TOP::setOutputs( Texture *_out0, Texture *_out1, Texture *_out2, Texture *_out3 )
{
	out0 = _out0;
	out1 = _out1;
	out2 = _out2;
	out3 = _out3;
	m_fbo->setOutputs( out0, out1, out2, out3 );
}

