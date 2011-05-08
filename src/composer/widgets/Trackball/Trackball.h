#pragma once

#include <iostream>

#include <QtGui>
#include <QGraphicsView>
#include <QGraphicsItem>

#include "ProjectorItem.h"



namespace composer
{
	namespace widgets
	{
		class Trackball : public QGraphicsView
		{
			Q_OBJECT        // must include this if you use Qt signals/slots

		public:
			Trackball(QWidget *parent = 0);

			typedef void (*ChangedCallback)( const math::Vec3f &vector );
			void setCallback( ChangedCallback callback );

		protected:
			void mouseMoveEvent( QMouseEvent * event );
			void drawBackground( QPainter * painter, const QRectF & rect );

		private:
			typedef std::map<QGraphicsItem *, math::Vec3f> ItemMap;
			void addItem( QGraphicsItem *item, const math::Vec3f &pos );
			void moveItems( float x, float y, float z );
			void rotateItems( math::Vec3f origin, float azimuth, float elevation );

			ChangedCallback m_callback;
			QPoint m_lastMousePos;
			QGraphicsItem *m_vecItem;

			ProjectorItem *m_projector;
		};
	}
}
