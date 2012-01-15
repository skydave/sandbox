#pragma once
#include <iostream>
#include <ui_Nebulae.h>

#include "Nebulae.h"

//#include "composer/widgets/CurveEditor/CurveEditor.h"
//#include "composer/widgets/Trackball/Trackball.h"

class NebulaeUI : public QWidget
{
	Q_OBJECT

public:
	NebulaeUI( QWidget *parent = 0);


	public slots:
	void onParticleScaleValueChanged(int value);
	void onParticleAlphaValueChanged(int value);
	void onAttractorAValueChanged(double value);
	void onAttractorBValueChanged(double value);
	void onAttractorCValueChanged(double value);
	void onAttractorDValueChanged(double value);
	void onBillboardScaleValueChanged(int value);
	void onBillboardAlphaValueChanged(int value);
	void onFrequencyValueChanged(int value);
	void onOctavesValueChanged(int value);
	void onLacunarityValueChanged(int value);
	void onGainValueChanged(int value);

	/*
	void onIr1ValueChanged(int value);
	void onIr2ValueChanged(int value);
	void onIr3ValueChanged(int value);
	void onOctavesValueChanged(int value);
	void onFrequencyValueChanged(int value);
	void onSunDirChanged(float x, float y, float z);
	void onPhiFunctionChanged(const QString &curveId);
	void onReloadShaderButtonPressed( bool checked );

	//private:
*/
	signals:
	void makeDirty(void);

	private:
	//NebulaePtr                 m_nebulae;
	public:
	Ui_Form ui;
};
