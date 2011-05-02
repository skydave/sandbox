#pragma once
#include <QObject>
#include <QGraphicsItem>
#include <QList>
#include <QFont>
#include <QLinkedList>

#include <gfx/FCurve.h>

#include "CurveHandleItem.h"




namespace composer
{
	namespace widgets
	{
		class CurveEditor;
		class CurveHandleItem;
		class CurveItem : public QGraphicsItem
		{
		public:
			CurveItem( CurveEditor *editor );
			CurveItem( CurveEditor *editor, const base::FCurve &curve );

			base::FCurve &curve();



		protected:
			QRectF boundingRect() const;
			void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

		private:
			void insertCP( const QPointF &p );
			void makeActive();
			void makeInactive();
			void updateScale( float newScaleX, float newScaleY );
			void update( CurveHandleItem *handle );

			CurveEditor *m_editor;
			base::FCurve m_curve;

			float m_scaleX;
			float m_scaleY;

			bool m_isActive;

			typedef QLinkedList<CurveHandleItem *> HandleList;
			HandleList m_handles;

			friend CurveEditor;
			friend CurveHandleItem;
		};
	}
}