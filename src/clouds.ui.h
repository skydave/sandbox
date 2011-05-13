#pragma once
#include <iostream>
#include <ui_clouds.h>

class CloudsUI : public QWidget
{
	Q_OBJECT

public:
	CloudsUI(QWidget *parent = 0);

private slots:
	void onSunValueChanged(int value);

private:
	Ui_Form ui;
};

