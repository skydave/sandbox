#pragma once

#include <map>
#include <QtGui>
#include <QGraphicsView>
#include "AxisItem.h"
#include "CurveItem.h"



namespace composer
{
	namespace widgets
	{
		class CurveEditor : public QGraphicsView
		{
			Q_OBJECT

		public:
			typedef void (*CurveChangedCallback)( const std::string&, const base::FCurve & );
			enum CoDomainScale
			{
				LINEAR,
				LOG10
			};

			CurveEditor(QWidget *parent = 0);

			void setCallback( CurveChangedCallback callback );
			void addCurve( const std::string &id, const base::FCurve &fcurve );
			void removeCurve( const std::string &id );
			void makeActive( const std::string &id );

			void setCoDomainScale( CoDomainScale scl );

		protected:
			void keyPressEvent(QKeyEvent *event);

			void drawBackground( QPainter * painter, const QRectF & rect );

			void mousePressEvent ( QMouseEvent * event );
			void mouseMoveEvent( QMouseEvent * event );
			void paintEvent ( QPaintEvent * event );

		private:
			void scaleBy(qreal scaleFactor);
			void setZoom(int percentZoom);
			QRectF getBound( base::FCurve &curve );
			CurveItem *getCurveItem( const std::string &id );
			std::string getCurveId( CurveItem *curve );
			float applyCoDomainScale( float domainValue );
			float applyCoDomainScaleInverse( float domainScaleValue );
			void curveChangedEvent( CurveItem *curve );

			CoDomainScale m_coDomainScale;
			CurveChangedCallback m_callback;
			QPoint m_lastMousePos;
			QGraphicsItemGroup *m_rootItem;
			float m_minScale;
			float m_maxScale;

			typedef std::map<std::string, CurveItem *> CurveMap;
			CurveMap m_curves;

			CurveItem *m_activeCurve;

			friend CurveItem;
		};
	}
}