#include <QtGui>

#include "clouds.ui.h"


#include <gfx/Shader.h>
#include <gfx/Texture.h>
extern base::ShaderPtr cloudShader;
extern base::Texture2dPtr clouds_parmameters;
extern float *clouds_parameters_tex;

CloudsUI::CloudsUI( base::FCurve &P_theta, QWidget *parent) : QWidget(parent)
{
	ui.setupUi(this);

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
	connect( ui.reloadShaderButton, SIGNAL(clicked(bool)), this, SLOT(onReloadShaderButtonPressed(bool)) );

}

void CloudsUI::onMaxHeightValueChanged(int value)
{
	float t = (value/100.0f)*1000.0f;
	cloudShader->getUniform( "maxHeight" )->set( 0, t );
	emit makeDirty();
}

void CloudsUI::onMaxVertexHeightValueChanged(int value)
{
	float t = (value/100.0f)*1000.0f;
	cloudShader->getUniform( "maxVertexHeight" )->set( 0, t );
	emit makeDirty();
}

void CloudsUI::onSunValueChanged(int value)
{
	float t = value/100.0f;
	cloudShader->getUniform( "Csun" )->set( 0, t, t, t, 1.0f );
	emit makeDirty();
}
void CloudsUI::onSunDirChanged(float x, float y, float z)
{
	cloudShader->getUniform( "sunDir" )->set( 0, x, y, z );
	emit makeDirty();
}
void CloudsUI::onSkyValueChanged(int value)
{
	float t = value/100.0f;
	cloudShader->getUniform( "Csky" )->set( 0, t, t, t, 1.0f );
	emit makeDirty();
}
void CloudsUI::onGroundValueChanged(int value)
{
	float t = value/100.0f;
	cloudShader->getUniform( "Cground" )->set( 0, t, t, t, 1.0f );
	emit makeDirty();
}
void CloudsUI::onIr1ValueChanged(int value)
{
	float t = value/100.0f;
	cloudShader->getUniform( "Ir1Mult" )->set( 0, t );
	emit makeDirty();
}
void CloudsUI::onIr2ValueChanged(int value)
{
	float t = value/100.0f;
	cloudShader->getUniform( "Ir2Mult" )->set( 0, t );
	emit makeDirty();
}
void CloudsUI::onIr3ValueChanged(int value)
{
	float t = value/100.0f;
	cloudShader->getUniform( "Ir3Mult" )->set( 0, t );
	emit makeDirty();
}

void CloudsUI::onOctavesValueChanged(int value)
{
	cloudShader->getUniform( "pn_octaves" )->set( 0, value );
	emit makeDirty();
}
void CloudsUI::onFrequencyValueChanged(int value)
{
	cloudShader->getUniform( "pn_frequency" )->set( 0, (value/100.0f)*40.0f );
	emit makeDirty();
}

void CloudsUI::onPhiFunctionChanged(const QString &curveId)
{
	std::string _curveId = curveId.toStdString();
	composer::widgets::CurveItem *ci = m_curveEditor->getCurveItem(_curveId);
	if(ci)
	{
		base::FCurve &curve = ci->curve();
		int numSamples = 512;
		int numParameters = 2;
		for(int i =0; i<numSamples; ++i)
		{
			int row = numSamples*4;
			clouds_parameters_tex[row+i*4+1] = curve.eval((float)i/(float)numSamples);
		}
		clouds_parmameters->uploadRGBAFloat32(numSamples, numParameters, clouds_parameters_tex);


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
			cloudShader->getUniform( "Pf")->set(0, Pf );
			cloudShader->getUniform( "theta_f")->set( 0, theta_f );
		}
		emit makeDirty();
	}
}

void CloudsUI::onReloadShaderButtonPressed( bool checked )
{
	cloudShader->reload();
	emit makeDirty();
}