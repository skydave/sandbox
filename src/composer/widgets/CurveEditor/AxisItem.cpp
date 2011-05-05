#include "AxisItem.h"
#include <QPainter>


namespace composer
{
	namespace widgets
	{
		AxisItem::AxisItem() : QGraphicsItem()
		{
			//setFlag(ItemIsMovable);
			//setFlag(ItemSendsGeometryChanges);
		}

		QRectF AxisItem::boundingRect() const
		{
			return QRectF(-10000.0f, -10000.0f, 20000.0f, 20000.0f);
		}

		void AxisItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
		{
			QPen pen(Qt::lightGray, 2, Qt::SolidLine);
			pen.setCosmetic ( true );
			painter->setPen(pen);

			painter->drawLine( -10000, 0, 10000, 0 );
			painter->drawLine( 0, -10000, 0, 10000 );
		}
	}
}
