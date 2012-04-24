#pragma once
#include <iostream>
#include <ui_ColorGrading.h>
#include "ColorGrading.h"

class ColorGradingUI : public QWidget
{
	Q_OBJECT

public:
	ColorGradingUI( ColorGradingPtr colorgrading, QWidget *parent = 0);

public slots:
	void onReloadButtonReleased(void);


signals:
	void makeDirty(void);

public:
	Ui_ColorGrading ui;

private:
	ColorGradingPtr m_colorgrading;
};
