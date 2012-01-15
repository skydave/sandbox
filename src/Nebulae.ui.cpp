#include <QtGui>

#include "Nebulae.ui.h"


#include <gfx/Shader.h>
#include <gfx/Texture.h>
//extern base::ShaderPtr cloudShader;
//extern base::Texture2dPtr clouds_parmameters;
//extern float *clouds_parameters_tex;
extern NebulaePtr nebulae;

NebulaeUI::NebulaeUI( QWidget *parent) : QWidget(parent)//, m_nebulae(nebulae)
{

	ui.setupUi(this);

	connect( ui.particleScale, SIGNAL(valueChanged(int)), this, SLOT(onParticleScaleValueChanged(int)) );
	connect( ui.particleAlpha, SIGNAL(valueChanged(int)), this, SLOT(onParticleAlphaValueChanged(int)) );
	connect( ui.attractor_a, SIGNAL(valueChanged(double)), this, SLOT(onAttractorAValueChanged(double)) );
	connect( ui.attractor_b, SIGNAL(valueChanged(double)), this, SLOT(onAttractorBValueChanged(double)) );
	connect( ui.attractor_c, SIGNAL(valueChanged(double)), this, SLOT(onAttractorCValueChanged(double)) );
	connect( ui.attractor_d, SIGNAL(valueChanged(double)), this, SLOT(onAttractorDValueChanged(double)) );
	connect( ui.billboardScale, SIGNAL(valueChanged(int)), this, SLOT(onBillboardScaleValueChanged(int)) );
	connect( ui.billboardAlpha, SIGNAL(valueChanged(int)), this, SLOT(onBillboardAlphaValueChanged(int)) );
	connect( ui.frequency, SIGNAL(valueChanged(int)), this, SLOT(onFrequencyValueChanged(int)) );



	/*
	m_curveEditor = new composer::widgets::CurveEditor(ui.phaseFunctionFrame);
	m_curveEditor->setCoDomainScale( composer::widgets::CurveEditor::LOG10 );
	m_curveEditor->addCurve( "test", P_theta );
	m_curveEditor->addCurve( "test2", base::FCurve(base::FCurve::LINEAR) );
	m_curveEditor->frameAll();


	m_trackball = new composer::widgets::Trackball(ui.sunDirFrame);

	connect( ui.maxVertexHeightSlider, SIGNAL(valueChanged(int)), this, SLOT(onMaxVertexHeightValueChanged(int)) );
	connect( ui.sunSlider, SIGNAL(valueChanged(int)), this, SLOT(onSunValueChanged(int)) );
	connect( m_trackball, SIGNAL(vectorChanged(float,float,float)), this, SLOT(onSunDirChanged(float,float,float)) );
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


void NebulaeUI::onParticleScaleValueChanged(int value)
{
	float t = (value/1000.0f)*100.0f;
	nebulae->m_particleShader->setUniform( "scale", t );
	emit makeDirty();
}

void NebulaeUI::onParticleAlphaValueChanged(int value)
{
	float t = (value/1000.0f);
	nebulae->m_particleShader->setUniform( "alpha", t );
	emit makeDirty();
}

void NebulaeUI::onAttractorAValueChanged(double value)
{
	nebulae->m_attractor.a = value;
	nebulae->generate();
	emit makeDirty();
}

void NebulaeUI::onAttractorBValueChanged(double value)
{
	nebulae->m_attractor.b = value;
	nebulae->generate();
	emit makeDirty();
}

void NebulaeUI::onAttractorCValueChanged(double value)
{
	nebulae->m_attractor.c = value;
	nebulae->generate();
	emit makeDirty();
}

void NebulaeUI::onAttractorDValueChanged(double value)
{
	nebulae->m_attractor.d = value;
	nebulae->generate();
	emit makeDirty();
}

void NebulaeUI::onBillboardScaleValueChanged(int value)
{
	float t = (value/1000.0f)*10.0f;
	nebulae->m_billboardShader->setUniform( "scale", t );
	emit makeDirty();
}

void NebulaeUI::onBillboardAlphaValueChanged(int value)
{
	float t = (value/1000.0f);
	nebulae->m_billboardShader->setUniform( "alpha", t );
	emit makeDirty();
}

void NebulaeUI::onFrequencyValueChanged(int value)
{
	float t = (value/1000.0f)*10.0f;
	nebulae->m_perlinNoiseShader->setUniform( "frequency", t );
	nebulae->applyPerlinNoise();
	emit makeDirty();
}

