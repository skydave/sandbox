#pragma once
#include <iostream>
#include <ui_TestEffect.h>
#include "TestEffect.h"

class TestEffectUI : public QWidget
{
	Q_OBJECT

public:
	TestEffectUI( TestEffectPtr effect, QWidget *parent = 0);

public slots:
	void onReloadButtonReleased(void);

	void onKaValueChanged(int value);
	void onKdValueChanged(int value);
	void onKsValueChanged(int value);

signals:
	void makeDirty(void);

public:
	Ui_TestEffect ui;

private:
	TestEffectPtr m_effect;
};
