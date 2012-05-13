#pragma once
#include <iostream>
#include <ui_Nebulae.h>

#include "Nebulae.h"


class NebulaeUI : public QWidget
{
	Q_OBJECT

public:
	NebulaeUI( NebulaePtr nebulae, QWidget *parent = 0);


	public slots:
	void onParticleScaleValueChanged(int value);
	void onParticleAlphaValueChanged(int value);
	void onAttractorCoefficientChanged(double value);
	void onBillboardScaleValueChanged(int value);
	void onBillboardAlphaValueChanged(int value);
	void onFrequencyValueChanged(int value);
	void onOctavesValueChanged(int value);
	void onLacunarityValueChanged(int value);
	void onGainValueChanged(int value);
	void onExportParticlesButtonReleased(void);


	signals:
	void makeDirty(void);

	private:
	NebulaePtr                 m_nebulae;
	public:
	Ui_Form ui;
};
