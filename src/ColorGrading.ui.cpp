#include <QtGui>
#include <QColor>
#include <QRgb>
#include <QColorDialog>
#include "ColorGrading.ui.h"


#include <gfx/Shader.h>
#include <gfx/Texture.h>


ColorGradingUI::ColorGradingUI( ColorGradingPtr colorgrading, QWidget *parent) : QWidget(parent), m_colorgrading(colorgrading)
{
	ui.setupUi(this);


	connect( ui.reloadShaderButton, SIGNAL(released(void)), this, SLOT(onReloadButtonReleased(void)) );

}

void ColorGradingUI::onReloadButtonReleased(void)
{
	m_colorgrading->reload();
	emit makeDirty();
}

