#pragma once
#include <iostream>
#include <ui_clouds.h>

#include "composer/widgets/CurveEditor/CurveEditor.h"
#include "composer/widgets/Trackball/Trackball.h"

class CloudsUI : public QWidget
{
	Q_OBJECT

public:
	CloudsUI(base::FCurve &P_theta, QWidget *parent = 0);

public slots:
	void onMaxHeightValueChanged(int value);
	void onMaxVertexHeightValueChanged(int value);
	void onSunValueChanged(int value);
	void onSkyValueChanged(int value);
	void onGroundValueChanged(int value);
	void onIr1ValueChanged(int value);
	void onIr2ValueChanged(int value);
	void onIr3ValueChanged(int value);
	void onOctavesValueChanged(int value);
	void onFrequencyValueChanged(int value);
	void onSunDirChanged(float x, float y, float z);
	void onPhiFunctionChanged(const QString &curveId);
	void onReloadShaderButtonPressed( bool checked );
signals:
	void makeDirty(void);

//private:
public:
	Ui_Form ui;
	composer::widgets::CurveEditor *m_curveEditor;
	composer::widgets::Trackball *m_trackball;
};

