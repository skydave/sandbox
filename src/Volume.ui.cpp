#include <QtGui>
#include <QColor>
#include <QRgb>
#include <QColorDialog>
#include "Volume.ui.h"


#include <gfx/Shader.h>
#include <gfx/Texture.h>


class MyDoubleValidator : public QDoubleValidator 
{ 
public: 
   MyDoubleValidator( double bottom, double top, int decimals, QObject* parent = 0) 
      : QDoubleValidator( bottom, top, decimals, parent) 
   {} 
  
   QValidator::State validate(QString & input, int &) const 
   { 
      const double b = bottom(); 
      const double t = top(); 
      const int d = decimals(); 

      QRegExp empty(QString::fromLatin1("-?\\.?")); 
      if (input.contains(' ')) 
         return Invalid; 
      if (b >= 0 && input.startsWith(QLatin1Char('-'))) 
         return Invalid; 
      if (empty.exactMatch(input)) 
         return Intermediate; 

      double entered = input.toDouble(); 
      int nume = input.count('e', Qt::CaseInsensitive); 

      int i;
      if (input.contains(',')) 
         i = input.indexOf(','); 
      else 
         i = input.indexOf('.'); 
    
      if (i >= 0 && nume == 0) { 
         i++; 
         int j = i; 
         while(input[j].isDigit()) 
            j++; 
         if (j - i > d) 
            return Invalid; 
      } 

      if (entered < b || entered > t) 
         return Invalid; 

      return Acceptable;          
   } 
}; 

VolumeUI::VolumeUI( VolumePtr volume, QWidget *parent) : QWidget(parent), m_volume(volume)//, m_nebulae(nebulae)
{
	ui.setupUi(this);
	ui.totalCrossSectionRangeMax->setValidator(new MyDoubleValidator( 0, 5000, 2, ui.totalCrossSectionRangeMax)); 
	ui.totalCrossSectionRangeMax->setText( QString::number( m_volume->getTotalCrossSection()*2.0f ) );
	ui.albedo->setValue( (int)(m_volume->getAlbedo()*5000.0f) );

	// update color button with current selected color
	const QString colorStyle("QPushButton { background-color : %1; }");

	QColor absorptionColor;
	absorptionColor.setRgbF( m_volume->getAbsorptionColor().x, m_volume->getAbsorptionColor().y, m_volume->getAbsorptionColor().z );
	ui.absorptionColor->setStyleSheet(colorStyle.arg(absorptionColor.name()));
	QColor scatteringColor;
	scatteringColor.setRgbF( m_volume->getScatteringColor().x, m_volume->getScatteringColor().y, m_volume->getScatteringColor().z );
	ui.scatteringColor->setStyleSheet(colorStyle.arg(scatteringColor.name()));

	connect( ui.totalCrossSection, SIGNAL(valueChanged(int)), this, SLOT(onTotalCrossSectionValueChanged(int)) );
	connect( ui.totalCrossSectionRangeMax, SIGNAL(textChanged(const QString &)), this, SLOT(onTotalCrossSectionRangeMaxChanged(const QString &)) );
	connect( ui.albedo, SIGNAL(valueChanged(int)), this, SLOT(onAlbedoValueChanged(int)) );
	connect( ui.absorptionColor, SIGNAL(released(void)), this, SLOT(onAbsorptionColorButtonReleased(void)) );
	connect( ui.scatteringColor, SIGNAL(released(void)), this, SLOT(onScatteringColorButtonReleased(void)) );
	connect( ui.reload, SIGNAL(released(void)), this, SLOT(onReloadButtonReleased(void)) );
	connect( ui.light0Exposure, SIGNAL(valueChanged(int)), this, SLOT(onLight0ExposureValueChanged(int)) );	
	connect( ui.light0Color, SIGNAL(released(void)), this, SLOT(onLight0ColorButtonReleased(void)) );
}


void VolumeUI::onTotalCrossSectionValueChanged(int value)
{
	float rangeMax = ui.totalCrossSectionRangeMax->text().toFloat();
	float totalCrossSection = (value/1000.0f)*rangeMax;
	m_volume->setTotalCrossSection(totalCrossSection);
	emit makeDirty();
}

void VolumeUI::onTotalCrossSectionRangeMaxChanged( const QString & text )
{
	float tmax = text.toFloat();
	float totalCrossSection = ui.totalCrossSection->value()/1000.0f * tmax;
	m_volume->setTotalCrossSection(totalCrossSection);
	emit makeDirty();
}

void VolumeUI::onAlbedoValueChanged(int value)
{
	float albedo = (value/1000.0f);
	m_volume->setAlbedo(albedo);
	emit makeDirty();
}

void VolumeUI::onAbsorptionColorButtonReleased(void)
{
	// get current color
	QColor absorptionColor;
	absorptionColor.setRgbF( m_volume->getAbsorptionColor().x, m_volume->getAbsorptionColor().y, m_volume->getAbsorptionColor().z );

	static QColorDialog *qcd = 0;
	if(!qcd)
	{
		// assuming it will be destroyed with its parent
		qcd = new QColorDialog(absorptionColor, this);
		qcd->setOption( QColorDialog::NoButtons );
		connect(qcd, SIGNAL(colorSelected(QColor)), this, SLOT(absorptionColorChanged(QColor)));
		connect(qcd, SIGNAL(currentColorChanged(QColor)), this, SLOT(absorptionColorChanged(QColor)));
	}
	qcd->setCurrentColor( absorptionColor );
	qcd->open();
}

void VolumeUI::onScatteringColorButtonReleased(void)
{
	// get current color
	QColor scatteringColor;
	scatteringColor.setRgbF( m_volume->getScatteringColor().x, m_volume->getScatteringColor().y, m_volume->getScatteringColor().z );

	static QColorDialog *qcd = 0;
	if(!qcd)
	{
		// assuming it will be destroyed with its parent
		qcd = new QColorDialog(scatteringColor, this);
		qcd->setOption( QColorDialog::NoButtons );
		connect(qcd, SIGNAL(colorSelected(QColor)), this, SLOT(scatteringColorChanged(QColor)));
		connect(qcd, SIGNAL(currentColorChanged(QColor)), this, SLOT(scatteringColorChanged(QColor)));
	}
	qcd->setCurrentColor( scatteringColor );
	qcd->open();
}

void VolumeUI::absorptionColorChanged ( const QColor & absorptionColor )
{
	// update button color
	const QString colorStyle("QPushButton { background-color : %1; }");
	ui.absorptionColor->setStyleSheet(colorStyle.arg(absorptionColor.name()));
	// update volume
	m_volume->setAbsorptionColor( math::Vec3f( absorptionColor.redF(), absorptionColor.greenF(), absorptionColor.blueF() ) );
	emit makeDirty();
}
void VolumeUI::scatteringColorChanged ( const QColor & scatteringColor )
{
	// update button color
	const QString colorStyle("QPushButton { background-color : %1; }");
	ui.scatteringColor->setStyleSheet(colorStyle.arg(scatteringColor.name()));

	// update volume
	m_volume->setScatteringColor( math::Vec3f( scatteringColor.redF(), scatteringColor.greenF(), scatteringColor.blueF() ) );
	emit makeDirty();
}

void VolumeUI::onReloadButtonReleased(void)
{
	m_volume->reload();
	emit makeDirty();
}

void VolumeUI::onLight0ExposureValueChanged(int value)
{
	float rangeMax = 5.0f;
	float exposure = (value/1000.0f)*rangeMax;
	Light l = m_volume->getLight( 0 );
	l.exposure = exposure;
	m_volume->setLight(0,l);
	emit makeDirty();
}

void VolumeUI::onLight0ColorButtonReleased(void)
{
	// get current color
	Light l = m_volume->getLight( 0 );
	QColor lightColor;
	lightColor.setRgbF( l.color.x, l.color.y, l.color.z );

	static QColorDialog *qcd = 0;
	if(!qcd)
	{
		// assuming it will be destroyed with its parent
		qcd = new QColorDialog(lightColor, this);
		qcd->setOption( QColorDialog::NoButtons );
		connect(qcd, SIGNAL(colorSelected(QColor)), this, SLOT(light0ColorChanged(QColor)));
		connect(qcd, SIGNAL(currentColorChanged(QColor)), this, SLOT(light0ColorChanged(QColor)));
	}
	qcd->setCurrentColor( lightColor );
	qcd->open();
}
void VolumeUI::light0ColorChanged ( const QColor & lightColor )
{
	Light l = m_volume->getLight( 0 );
	// update button color
	const QString colorStyle("QPushButton { background-color : %1; }");
	ui.light0Color->setStyleSheet(colorStyle.arg(lightColor.name()));

	// update volume
	l.color = math::Vec3f( lightColor.redF(), lightColor.greenF(), lightColor.blueF() );
	m_volume->setLight( 0, l );
	emit makeDirty();
}
