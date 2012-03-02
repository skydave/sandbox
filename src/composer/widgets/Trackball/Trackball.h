#pragma once

#include <iostream>
#include <map>

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

			void setVector( float x, float y, float z );
			void setVector( const math::Vec3f v );

			typedef void (*ChangedCallback)( const math::Vec3f &vector );
			void setCallback( ChangedCallback callback );

		signals:
			void vectorChanged(float x, float y, float z);

		protected:
			void mouseMoveEvent( QMouseEvent * event );
			void drawBackground( QPainter * painter, const QRectF & rect );
			void resizeEvent ( QResizeEvent * event );

		private:
			typedef std::map<QGraphicsItem *, math::Vec3f> ItemMap;
			void addItem( QGraphicsItem *item, const math::Vec3f &pos );
			void moveItems( float x, float y, float z );
			void rotateItems( math::Vec3f origin, float azimuth, float elevation );

			ChangedCallback m_callback;
			QPoint      m_lastMousePos;

			QGraphicsItem                                    *m_vecItem; // this item identifies the vector we are tracking

			ProjectorItem                                  *m_projector; // main item to which we will add all other items. projector will set the 2d position from 3d positions using a projection
		};
	}
}
