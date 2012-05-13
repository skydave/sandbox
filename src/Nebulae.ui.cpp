#include <QtGui>
#include <QFileDialog>

#include "Nebulae.ui.h"


NebulaeUI::NebulaeUI( NebulaePtr nebulae, QWidget *parent) : QWidget(parent), m_nebulae(nebulae)
{

	ui.setupUi(this);

	// set ui to reflect current values of nebulae
	ui.particleScale->setValue( (int)((m_nebulae->getParticleScale()/20.0f)*1000.0f) );
	ui.particleAlpha->setValue( (int)((m_nebulae->getParticleAlpha()/2.0f)*1000.0f) );
	ui.billboardScale->setValue( (int)((m_nebulae->getBillboardScale()/1.0f)*1000.0f) );
	ui.billboardAlpha->setValue( (int)((m_nebulae->getBillboardAlpha()/2.0f)*1000.0f) );
	ui.frequency->setValue( (int)((m_nebulae->getFrequency()/10.0f)*1000.0f) );
	ui.octaves->setValue( m_nebulae->getOctaves() );
	ui.lacunarity->setValue( (int)((m_nebulae->getLacunarity()/4.0f)*1000.0f) );
	ui.gain->setValue( (int)((m_nebulae->getGain()/1.0f)*1000.0f) );


	connect( ui.particleScale, SIGNAL(valueChanged(int)), this, SLOT(onParticleScaleValueChanged(int)) );
	connect( ui.particleAlpha, SIGNAL(valueChanged(int)), this, SLOT(onParticleAlphaValueChanged(int)) );
	connect( ui.attractor_a, SIGNAL(valueChanged(double)), this, SLOT(onAttractorCoefficientChanged(double)) );
	connect( ui.attractor_b, SIGNAL(valueChanged(double)), this, SLOT(onAttractorCoefficientChanged(double)) );
	connect( ui.attractor_c, SIGNAL(valueChanged(double)), this, SLOT(onAttractorCoefficientChanged(double)) );
	connect( ui.attractor_d, SIGNAL(valueChanged(double)), this, SLOT(onAttractorCoefficientChanged(double)) );
	connect( ui.billboardScale, SIGNAL(valueChanged(int)), this, SLOT(onBillboardScaleValueChanged(int)) );
	connect( ui.billboardAlpha, SIGNAL(valueChanged(int)), this, SLOT(onBillboardAlphaValueChanged(int)) );
	connect( ui.frequency, SIGNAL(valueChanged(int)), this, SLOT(onFrequencyValueChanged(int)) );
	connect( ui.octaves, SIGNAL(valueChanged(int)), this, SLOT(onOctavesValueChanged(int)) );
	connect( ui.lacunarity, SIGNAL(valueChanged(int)), this, SLOT(onLacunarityValueChanged(int)) );
	connect( ui.gain, SIGNAL(valueChanged(int)), this, SLOT(onGainValueChanged(int)) );
	connect( ui.exportParticlesButton, SIGNAL(released(void)), this, SLOT(onExportParticlesButtonReleased(void)) );


}


void NebulaeUI::onParticleScaleValueChanged(int value)
{
	float t = (value/1000.0f)*20.0f;
	m_nebulae->setParticleScale(t);
	emit makeDirty();
}

void NebulaeUI::onParticleAlphaValueChanged(int value)
{
	float t = (value/1000.0f)*2.0;
	m_nebulae->setParticleAlpha(t);
	emit makeDirty();
}

void NebulaeUI::onAttractorCoefficientChanged(double value)
{
	float a, b, c, d;
	a = (float)ui.attractor_a->value();
	b = (float)ui.attractor_b->value();
	c = (float)ui.attractor_c->value();
	d = (float)ui.attractor_d->value();
	m_nebulae->setGenerator_kingsdream(a, b, c, d);
	emit makeDirty();
}

void NebulaeUI::onBillboardScaleValueChanged(int value)
{
	float t = (value/1000.0f)*1.0f;
	m_nebulae->setBillboardScale(t);
	emit makeDirty();
}

void NebulaeUI::onBillboardAlphaValueChanged(int value)
{
	float t = (value/1000.0f)*2.0;
	m_nebulae->setBillboardAlpha(t);
	emit makeDirty();
}

void NebulaeUI::onFrequencyValueChanged(int value)
{
	float t = (value/1000.0f)*10.0f;
	m_nebulae->setFrequency(t);
	emit makeDirty();
}

void NebulaeUI::onOctavesValueChanged(int value)
{
	m_nebulae->setOctaves(value);
	emit makeDirty();
}

void NebulaeUI::onLacunarityValueChanged(int value)
{
	float t = (value/1000.0f)*4.0;
	m_nebulae->setLacunarity(t);
	emit makeDirty();
}

void NebulaeUI::onGainValueChanged(int value)
{
	float t = (value/1000.0f)*1.0;
	m_nebulae->setGain(t);
	emit makeDirty();
}


void NebulaeUI::onExportParticlesButtonReleased(void)
{
	// file dialog
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "untitled.prt", tr("krakatoa particle format (*.prt)"));

	std::string path = fileName.toAscii().constData();

	// export particles
	m_nebulae->writeParticles( path );
}