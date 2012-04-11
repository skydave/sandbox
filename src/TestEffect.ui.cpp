#include <QtGui>
#include <QColor>
#include <QRgb>
#include <QColorDialog>
#include "TestEffect.ui.h"


#include <gfx/Shader.h>
#include <gfx/Texture.h>


TestEffectUI::TestEffectUI( TestEffectPtr effect, QWidget *parent) : QWidget(parent), m_effect(effect)
{
	ui.setupUi(this);

	ui.kaSlider->setValue( int( effect->getKa()*1000.0f ) );
	ui.kdSlider->setValue( int( effect->getKd()*1000.0f ) );
	ui.ksSlider->setValue( int( effect->getKs()*1000.0f ) );

	connect( ui.reloadShaderButton, SIGNAL(released(void)), this, SLOT(onReloadButtonReleased(void)) );
	connect( ui.kaSlider, SIGNAL(valueChanged(int)), this, SLOT(onKaValueChanged(int)) );
	connect( ui.kdSlider, SIGNAL(valueChanged(int)), this, SLOT(onKdValueChanged(int)) );
	connect( ui.ksSlider, SIGNAL(valueChanged(int)), this, SLOT(onKsValueChanged(int)) );
}

void TestEffectUI::onReloadButtonReleased(void)
{
	m_effect->reload();
	emit makeDirty();
}

void TestEffectUI::onKaValueChanged(int value)
{
	float ka = (value/1000.0f);
	m_effect->setKa(ka*10.0f);
	emit makeDirty();
}

void TestEffectUI::onKdValueChanged(int value)
{
	float kd = (value/1000.0f);
	m_effect->setKd(kd);
	emit makeDirty();

}

void TestEffectUI::onKsValueChanged(int value)
{
	float ks = (value/1000.0f);
	m_effect->setKs(ks);
	emit makeDirty();

}
