#pragma once
#include <QObject>
#include <QGraphicsItem>
#include <QList>
#include <QFont>

 

namespace composer
{
	namespace widgets
	{
		class CurveItem;
		class CurveEditor;
		class CurveHandleItem : public QGraphicsItem
		{
		public:
			CurveHandleItem( CurveItem *curve, float scaleX=1.0f, float scaleY=1.0f );
			void updateScale( float newScaleX, float newScaleY );


		protected:
			QRectF boundingRect() const;
			void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

			void mousePressEvent(QGraphicsSceneMouseEvent *event);

			QVariant itemChange( GraphicsItemChange change, const QVariant & value );

		private:
			CurveItem *m_curve;
			bool m_propagateChanges;
			float m_scaleX;
			float m_scaleY;

			friend class CurveItem;
			friend class CurveEditor;
		};
	}
}
