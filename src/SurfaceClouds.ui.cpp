#include <QtGui>

#include "SurfaceClouds.ui.h"

#include <gfx/Shader.h>
#include <gfx/Texture.h>


SurfaceCloudsUI::SurfaceCloudsUI( SurfaceCloudsPtr surfaceClouds, QWidget *parent) : QWidget(parent), m_surfaceClouds(surfaceClouds)
{
	ui.setupUi(this);


	m_curveEditor = new composer::widgets::CurveEditor(ui.phaseFunctionFrame);
	m_curveEditor->setCoDomainScale( composer::widgets::CurveEditor::LOG10 );
	m_curveEditor->addCurve( "test", surfaceClouds->m_phaseFunction );
	m_curveEditor->addCurve( "test2", base::FCurve(base::FCurve::LINEAR) );
	m_curveEditor->frameAll();


	m_trackball = new composer::widgets::Trackball(ui.sunDirFrame);
	m_trackball->setVector( m_surfaceClouds->m_sunDir );

	connect( m_trackball, SIGNAL(vectorChanged(float,float,float)), this, SLOT(onSunDirChanged(float,float,float)) );
	connect( m_curveEditor, SIGNAL(curveChanged(const QString&)), this, SLOT(onPhiFunctionChanged(const QString&)) );
	connect( ui.maxVertexHeightSlider, SIGNAL(valueChanged(int)), this, SLOT(onMaxVertexHeightValueChanged(int)) );
	connect( ui.reloadShaderButton, SIGNAL(clicked(bool)), this, SLOT(onReloadShaderButtonPressed(bool)) );
	/*
	connect( ui.maxHeightSlider, SIGNAL(valueChanged(int)), this, SLOT(onMaxHeightValueChanged(int)) );
	connect( ui.sunSlider, SIGNAL(valueChanged(int)), this, SLOT(onSunValueChanged(int)) );
	connect( ui.skySlider, SIGNAL(valueChanged(int)), this, SLOT(onSkyValueChanged(int)) );
	connect( ui.groundSlider, SIGNAL(valueChanged(int)), this, SLOT(onGroundValueChanged(int)) );
	connect( ui.Ir1Slider, SIGNAL(valueChanged(int)), this, SLOT(onIr1ValueChanged(int)) );
	connect( ui.Ir2Slider, SIGNAL(valueChanged(int)), this, SLOT(onIr2ValueChanged(int)) );
	connect( ui.Ir3Slider, SIGNAL(valueChanged(int)), this, SLOT(onIr3ValueChanged(int)) );
	connect( ui.octavesSlider, SIGNAL(valueChanged(int)), this, SLOT(onOctavesValueChanged(int)) );
	connect( ui.frequencySlider, SIGNAL(valueChanged(int)), this, SLOT(onFrequencyValueChanged(int)) );
	*/

}

void SurfaceCloudsUI::onReloadShaderButtonPressed( bool checked )
{
	m_surfaceClouds->m_shader->reload();
	emit makeDirty();
}

void SurfaceCloudsUI::onSunDirChanged(float x, float y, float z)
{
	std::cout << x << " " << y << " " << z << std::endl;
	m_surfaceClouds->setSunDir( math::Vec3f(x, y, z) );
	emit makeDirty();
}

void SurfaceCloudsUI::onMaxVertexHeightValueChanged(int value)
{
	float t = value;
	m_surfaceClouds->setMaxVertexHeight( t );
	emit makeDirty();
}

void SurfaceCloudsUI::onPhiFunctionChanged(const QString &curveId)
{
	std::string _curveId = curveId.toLocal8Bit().data();
	composer::widgets::CurveItem *ci = m_curveEditor->getCurveItem(_curveId);
	if(ci)
	{
		base::FCurve &curve = ci->curve();
		int numSamples = 512;
		int numParameters = 2;
		for(int i =0; i<numSamples; ++i)
		{
			int row = numSamples*4;
			m_surfaceClouds->m_parameters_tex[row+i*4+1] = curve.eval((float)i/(float)numSamples);
		}
		m_surfaceClouds->m_parameters->uploadRGBAFloat32(numSamples, numParameters, m_surfaceClouds->m_parameters_tex);


		//
		// compute Pf
		{
			float theta_f = math::degToRad(5.0f); // in rad
			// integrate one slice
			float A_slice = 0.0f;

			float theta = 0.0f;
			int numS = 500;
			for(int n=0; n<numS;++n)
			{
				if( theta < theta_f )
					A_slice += curve.eval((float)n/(float)numS)*sin(theta);

				theta += (MATH_PIf/numS);
			}
			A_slice = (MATH_PIf/numS)*A_slice;
			float Pf = 2.0f * MATH_PIf * A_slice;

			// normalize over solid angle of sphere
			Pf /= 4.0f*MATH_PIf;

			//std::cout << "Pf=" << Pf << std::endl;
			m_surfaceClouds->m_shader->getUniform( "P_f")->set(0, Pf );
			m_surfaceClouds->m_shader->getUniform( "theta_f")->set( 0, theta_f );
		}
		emit makeDirty();
	}
}