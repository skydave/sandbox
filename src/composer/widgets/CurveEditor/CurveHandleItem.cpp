#include "CurveHandleItem.h"
#include <iostream>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>

#include "CurveItem.h"

namespace composer
{
	namespace widgets
	{

		CurveHandleItem::CurveHandleItem(CurveItem *curve, float scaleX, float scaleY) : QGraphicsItem(), m_curve(curve), m_scaleX(scaleX), m_scaleY(scaleY), m_propagateChanges(true)
		{
			setFlag(ItemIsMovable);
			setFlag(ItemIsSelectable);
			setFlag(ItemSendsGeometryChanges);
			setAcceptedMouseButtons( Qt::LeftButton );
		}

		QRectF CurveHandleItem::boundingRect() const
		{
			float size = 1.0f/m_scaleX*5.0f;
			return QRectF(-1.0f*size, -1.0f*size, 2.0f*size, 2.0f*size);
		}

		void CurveHandleItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
		{
			QPen pen(Qt::red);
			painter->setPen(pen);
			float size = 1.0f/m_scaleX*5.0f;
			painter->scale( size, size );
			painter->fillRect(-1,-1,2,2, Qt::red);
			painter->drawPoint(0,0);
		}

		void CurveHandleItem::mousePressEvent( QGraphicsSceneMouseEvent * event )
		{
			return QGraphicsItem::mousePressEvent(event);
		}

		QVariant CurveHandleItem::itemChange( GraphicsItemChange change, const QVariant & value )
		{
			if (change == ItemPositionChange && scene())
			{
				//TODO: implement position constraint here
				if(m_propagateChanges)
					m_curve->update( this );
			}
			return QGraphicsItem::itemChange(change, value);
		}

		void CurveHandleItem::updateScale( float newScaleX, float newScaleY )
		{
			prepareGeometryChange();
			m_scaleX = newScaleX;
			m_scaleY = newScaleY;
		}

	}
}
