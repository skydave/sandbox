#include <QtGui>

#include "SurfaceClouds.ui.h"

#include <gfx/Shader.h>
#include <gfx/Texture.h>


SurfaceCloudsUI::SurfaceCloudsUI( SurfaceCloudsPtr surfaceClouds, QWidget *parent) : QWidget(parent), m_surfaceClouds(surfaceClouds)
{
	ui.setupUi(this);

	/*
	m_curveEditor = new composer::widgets::CurveEditor(ui.phaseFunctionFrame);
	m_curveEditor->setCoDomainScale( composer::widgets::CurveEditor::LOG10 );
	m_curveEditor->addCurve( "test", P_theta );
	m_curveEditor->addCurve( "test2", base::FCurve(base::FCurve::LINEAR) );
	m_curveEditor->frameAll();
	*/


	m_trackball = new composer::widgets::Trackball(ui.sunDirFrame);
	m_trackball->setVector( m_surfaceClouds->m_sunDir );

	connect( m_trackball, SIGNAL(vectorChanged(float,float,float)), this, SLOT(onSunDirChanged(float,float,float)) );
	/*
	connect( ui.maxHeightSlider, SIGNAL(valueChanged(int)), this, SLOT(onMaxHeightValueChanged(int)) );
	connect( ui.maxVertexHeightSlider, SIGNAL(valueChanged(int)), this, SLOT(onMaxVertexHeightValueChanged(int)) );
	connect( ui.sunSlider, SIGNAL(valueChanged(int)), this, SLOT(onSunValueChanged(int)) );
	connect( ui.skySlider, SIGNAL(valueChanged(int)), this, SLOT(onSkyValueChanged(int)) );
	connect( ui.groundSlider, SIGNAL(valueChanged(int)), this, SLOT(onGroundValueChanged(int)) );
	connect( ui.Ir1Slider, SIGNAL(valueChanged(int)), this, SLOT(onIr1ValueChanged(int)) );
	connect( ui.Ir2Slider, SIGNAL(valueChanged(int)), this, SLOT(onIr2ValueChanged(int)) );
	connect( ui.Ir3Slider, SIGNAL(valueChanged(int)), this, SLOT(onIr3ValueChanged(int)) );
	connect( ui.octavesSlider, SIGNAL(valueChanged(int)), this, SLOT(onOctavesValueChanged(int)) );
	connect( ui.frequencySlider, SIGNAL(valueChanged(int)), this, SLOT(onFrequencyValueChanged(int)) );
	connect( m_curveEditor, SIGNAL(curveChanged(const QString&)), this, SLOT(onPhiFunctionChanged(const QString&)) );
	connect( ui.reloadShaderButton, SIGNAL(clicked(bool)), this, SLOT(onReloadShaderButtonPressed(bool)) );
	*/

}


void SurfaceCloudsUI::onSunDirChanged(float x, float y, float z)
{
	//std::cout << x << " " << y << " " << z << std::endl;
	m_surfaceClouds->setSunDir( math::Vec3f(x, y, z) );
	emit makeDirty();
}