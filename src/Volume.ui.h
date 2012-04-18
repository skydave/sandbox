#pragma once
#include <iostream>
#include <ui_Volume.h>
#include "Volume.h"

class VolumeUI : public QWidget
{
	Q_OBJECT

public:
	VolumeUI( VolumePtr volume, QWidget *parent = 0);

public slots:
	void onTotalCrossSectionValueChanged(int value);
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

signals:
	void makeDirty(void);

public:
	Ui_Form ui;

private:
	VolumePtr m_volume;
};
