#include <QtGui>

#include "sky.ui.h"


#include <gfx/Shader.h>
#include <gfx/Texture.h>
extern base::ShaderPtr skyShader;
extern base::Texture2dPtr clouds_parmameters;
extern float *clouds_parameters_tex;

CloudsUI::CloudsUI( QWidget *parent) : QWidget(parent)
{
	ui.setupUi(this);
	/*
	m_curveEditor = new composer::widgets::CurveEditor(ui.phaseFunctionFrame);
	m_curveEditor->setCoDomainScale( composer::widgets::CurveEditor::LOG10 );
	m_curveEditor->addCurve( "test", P_theta );
	m_curveEditor->addCurve( "test2", base::FCurve(base::FCurve::LINEAR) );
	m_curveEditor->frameAll();


	m_trackball = new composer::widgets::Trackball(ui.sunDirFrame);

	connect( ui.maxHeightSlider, SIGNAL(valueChanged(int)), this, SLOT(onMaxHeightValueChanged(int)) );
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
	*/
	connect( ui.reloadShaderButton, SIGNAL(clicked(bool)), this, SLOT(onReloadShaderButtonPressed(bool)) );
}

void CloudsUI::onMaxHeightValueChanged(int value)
{
	//float t = (value/100.0f)*1000.0f;
	//cloudShader->getUniform( "maxHeight" )->set( 0, t );
	//emit makeDirty();
}

void CloudsUI::onMaxVertexHeightValueChanged(int value)
{
	//float t = (value/100.0f)*1000.0f;
	//cloudShader->getUniform( "maxVertexHeight" )->set( 0, t );
	//emit makeDirty();
}

void CloudsUI::onSunValueChanged(int value)
{
	//float t = value/100.0f;
	//cloudShader->getUniform( "Csun" )->set( 0, t, t, t, 1.0f );
	//emit makeDirty();
}
void CloudsUI::onSunDirChanged(float x, float y, float z)
{
	//cloudShader->getUniform( "sunDir" )->set( 0, x, y, z );
	//emit makeDirty();
}
void CloudsUI::onSkyValueChanged(int value)
{
	//float t = value/100.0f;
	//cloudShader->getUniform( "Csky" )->set( 0, t, t, t, 1.0f );
	//emit makeDirty();
}
void CloudsUI::onGroundValueChanged(int value)
{
	//float t = value/100.0f;
	//cloudShader->getUniform( "Cground" )->set( 0, t, t, t, 1.0f );
	//emit makeDirty();
}
void CloudsUI::onIr1ValueChanged(int value)
{
	//float t = value/100.0f;
	//cloudShader->getUniform( "Ir1Mult" )->set( 0, t );
	//emit makeDirty();
}
void CloudsUI::onIr2ValueChanged(int value)
{
	//float t = value/100.0f;
	//cloudShader->getUniform( "Ir2Mult" )->set( 0, t );
	//emit makeDirty();
}
void CloudsUI::onIr3ValueChanged(int value)
{
	//float t = value/100.0f;
	//cloudShader->getUniform( "Ir3Mult" )->set( 0, t );
	//emit makeDirty();
}

void CloudsUI::onOctavesValueChanged(int value)
{
	//cloudShader->getUniform( "pn_octaves" )->set( 0, value );
	//emit makeDirty();
}
void CloudsUI::onFrequencyValueChanged(int value)
{
	//cloudShader->getUniform( "pn_frequency" )->set( 0, (value/100.0f)*40.0f );
	//emit makeDirty();
}

void CloudsUI::onPhiFunctionChanged(const QString &curveId)
{
}

void CloudsUI::onReloadShaderButtonPressed( bool checked )
{
	skyShader->reload();
	emit makeDirty();
}