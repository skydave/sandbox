#include <QtGui>

#include "clouds.ui.h"





CloudsUI::CloudsUI(QWidget *parent) : QWidget(parent)
{
	ui.setupUi(this);

	connect( ui.sunSlider, SIGNAL(valueChanged(int)), this, SLOT(onSunValueChanged(int)) );

}


void CloudsUI::onSunValueChanged(int value)
{
	std::cout << "!!!!!!!!!!!!! SFASFASF !!!!!!!!!!\n";
}
