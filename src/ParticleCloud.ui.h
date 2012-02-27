#pragma once
#include <iostream>
#include <QSignalMapper>
#include <ui_ParticleCloud.h>
#include "ParticleCloud.h"

class ParticleCloudUI : public QWidget
{
	Q_OBJECT

public:
	ParticleCloudUI( ParticleCloudPtr particleCloud, QWidget *parent = 0);

public slots:

	void onPointNumValueChanged(int value);
	void onGlobalAlphaValueChanged(int value);
	void onGlobalScaleValueChanged(int value);
	void onProbabilityValueChanged(int value);
	void onScaleValueChanged(int value);
		/*
	void onTotalCrossSectionRangeMaxChanged( const QString & text );
	void onAlbedoValueChanged(int value);
	void onAbsorptionColorButtonReleased(void);
	void onScatteringColorButtonReleased(void);
	void absorptionColorChanged ( const QColor & color );
	void scatteringColorChanged ( const QColor & color );
	void onReloadButtonReleased(void);
	void onLight0ExposureValueChanged(int value);
	void onLight0ColorButtonReleased(void);
	void light0ColorChanged ( const QColor & color );
	*/
signals:
	void makeDirty(void);

public:
	Ui_Form ui;

private:
	ParticleCloudPtr m_particleCloud;
	QSignalMapper *m_probabilitySignalMapper;
	QSignalMapper *m_scaleSignalMapper;
};
