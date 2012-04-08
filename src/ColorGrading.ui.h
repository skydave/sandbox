#pragma once
#include <iostream>
#include <ui_ColorGrading.h>
#include "ColorGrading.h"

class ColorGradingUI : public QWidget
{
	Q_OBJECT

public:
	ColorGradingUI( ColorGradingPtr colgrad, QWidget *parent = 0);

public slots:
	void onReloadButtonReleased(void);
	/*
	void onTotalCrossSectionValueChanged(int value);
	void onTotalCrossSectionRangeMaxChanged( const QString & text );
	void onAlbedoValueChanged(int value);
	void onAbsorptionColorButtonReleased(void);
	void onScatteringColorButtonReleased(void);
	void absorptionColorChanged ( const QColor & color );
	void scatteringColorChanged ( const QColor & color );
	void onLight0ExposureValueChanged(int value);
	void onLight0ColorButtonReleased(void);
	void light0ColorChanged ( const QColor & color );
	*/

signals:
	void makeDirty(void);

public:
	Ui_Form ui;

private:
	ColorGradingPtr m_colgrad;
};
