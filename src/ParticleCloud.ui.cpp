#include <QtGui>
#include <QColor>
#include <QRgb>
#include <QColorDialog>
#include "ParticleCloud.ui.h"


#include <gfx/Shader.h>
#include <gfx/Texture.h>


ParticleCloudUI::ParticleCloudUI( ParticleCloudPtr particleCloud, QWidget *parent) : QWidget(parent), m_particleCloud(particleCloud)
{
	ui.setupUi(this);

	m_probabilitySignalMapper = new QSignalMapper(this);
	m_scaleSignalMapper = new QSignalMapper(this);

	connect( ui.pointNum, SIGNAL(valueChanged(int)), this, SLOT(onPointNumValueChanged(int)) );
	connect( ui.globalAlpha, SIGNAL(valueChanged(int)), this, SLOT(onGlobalAlphaValueChanged(int)) );
	connect( ui.globalScale, SIGNAL(valueChanged(int)), this, SLOT(onGlobalScaleValueChanged(int)) );

	//connect( ui.probability_0, SIGNAL(valueChanged(int)), this, SLOT(onProbabilityValueChanged(int)) );

	m_probabilitySignalMapper->setMapping( ui.probability_0, 0 );
	connect(ui.probability_0, SIGNAL(valueChanged(int)), m_probabilitySignalMapper, SLOT(map()));
	m_scaleSignalMapper->setMapping( ui.scale_0, 0 );
	connect(ui.scale_0, SIGNAL(valueChanged(int)), m_scaleSignalMapper, SLOT(map()));

	m_probabilitySignalMapper->setMapping( ui.probability_1, 5 );
	connect(ui.probability_1, SIGNAL(valueChanged(int)), m_probabilitySignalMapper, SLOT(map()));
	m_scaleSignalMapper->setMapping( ui.scale_1, 5 );
	connect(ui.scale_1, SIGNAL(valueChanged(int)), m_scaleSignalMapper, SLOT(map()));


	m_probabilitySignalMapper->setMapping( ui.probability_2, 10 );
	connect(ui.probability_2, SIGNAL(valueChanged(int)), m_probabilitySignalMapper, SLOT(map()));
	m_scaleSignalMapper->setMapping( ui.scale_2, 10 );
	connect(ui.scale_2, SIGNAL(valueChanged(int)), m_scaleSignalMapper, SLOT(map()));


	connect( m_probabilitySignalMapper, SIGNAL(mapped(int)), this, SLOT(onProbabilityValueChanged(int)) );
	connect( m_scaleSignalMapper, SIGNAL(mapped(int)), this, SLOT(onScaleValueChanged(int)) );

	
	/*
	ui.totalCrossSectionRangeMax->setValidator(new MyDoubleValidator( 0, 5000, 2, ui.totalCrossSectionRangeMax)); 
	ui.totalCrossSectionRangeMax->setText( QString::number( m_volume->getTotalCrossSection()*2.0f ) );
	ui.albedo->setValue( (int)(m_volume->getAlbedo()*5000.0f) );


	// update color button with current selected color
	const QString colorStyle("QPushButton { background-color : %1; }");

	QColor absorptionColor;
	absorptionColor.setRgbF( m_volume->getAbsorptionColor().x, m_volume->getAbsorptionColor().y, m_volume->getAbsorptionColor().z );
	ui.absorptionColor->setStyleSheet(colorStyle.arg(absorptionColor.name()));
	QColor scatteringColor;
	scatteringColor.setRgbF( m_volume->getScatteringColor().x, m_volume->getScatteringColor().y, m_volume->getScatteringColor().z );
	ui.scatteringColor->setStyleSheet(colorStyle.arg(scatteringColor.name()));


	connect( ui.totalCrossSectionRangeMax, SIGNAL(textChanged(const QString &)), this, SLOT(onTotalCrossSectionRangeMaxChanged(const QString &)) );
	connect( ui.albedo, SIGNAL(valueChanged(int)), this, SLOT(onAlbedoValueChanged(int)) );
	connect( ui.absorptionColor, SIGNAL(released(void)), this, SLOT(onAbsorptionColorButtonReleased(void)) );
	connect( ui.scatteringColor, SIGNAL(released(void)), this, SLOT(onScatteringColorButtonReleased(void)) );
	connect( ui.reload, SIGNAL(released(void)), this, SLOT(onReloadButtonReleased(void)) );
	connect( ui.light0Exposure, SIGNAL(valueChanged(int)), this, SLOT(onLight0ExposureValueChanged(int)) );	
	connect( ui.light0Color, SIGNAL(released(void)), this, SLOT(onLight0ColorButtonReleased(void)) );
	*/
}
void ParticleCloudUI::onPointNumValueChanged(int value)
{
	float pointNum = (value/1000.0f);
	m_particleCloud->setParticleNumScale(pointNum);
	m_particleCloud->buildGeometry();
	emit makeDirty();
}
void ParticleCloudUI::onGlobalAlphaValueChanged(int value)
{
	float alpha = (value/1000.0f);
	m_particleCloud->setGlobalAlpha(alpha);
	emit makeDirty();
}

void ParticleCloudUI::onGlobalScaleValueChanged(int value)
{
	float scale = (value/1000.0f)*2.0f;
	m_particleCloud->setGlobalScale(scale);
	emit makeDirty();
}

void ParticleCloudUI::onProbabilityValueChanged(int billboardTypeId)
{
	QSlider *slider = qobject_cast<QSlider *>(m_probabilitySignalMapper->mapping( billboardTypeId ));
	float prob = (slider->value()/1000.0f);
	m_particleCloud->setupBillboardTypeProbability( billboardTypeId, prob);
	emit makeDirty();
}

void ParticleCloudUI::onScaleValueChanged(int billboardTypeId)
{
	QSlider *slider = qobject_cast<QSlider *>(m_scaleSignalMapper->mapping( billboardTypeId ));
	float scale = (slider->value()/1000.0f)*2.0f;
	m_particleCloud->setupBillboardTypeScale( billboardTypeId, scale);
	emit makeDirty();
}