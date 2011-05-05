#include "CurveEditor.h"
#include <iostream>
#include <algorithm>


namespace composer
{
	namespace widgets
	{
		CurveEditor::CurveEditor(QWidget *parent) : QGraphicsView(parent), m_activeCurve(0), m_coDomainScale(LINEAR)
		{
			QGraphicsScene *scene = new QGraphicsScene(this);
			scene->setItemIndexMethod(QGraphicsScene::NoIndex);
			scene->setSceneRect(-200, -200, 400, 400);

			setScene(scene);
			setCacheMode(CacheBackground);
			//setViewportUpdateMode(BoundingRectViewportUpdate);
			setViewportUpdateMode(FullViewportUpdate);
			//setRenderHint(QPainter::Antialiasing);
			//setRenderHint(QPainter::);
			setTransformationAnchor(NoAnchor);
			//setAlignment( Qt::AlignJustify );


			setMinimumSize(400, 400);
			setWindowTitle(tr("Elastic Nodes"));
			setMouseTracking( true );

			setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
			setVerticalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );



			QGraphicsTextItem *text = scene->addText( "Deine Muddah" );
			m_rootItem = new QGraphicsItemGroup();
			m_rootItem->setHandlesChildEvents(false);
			m_rootItem->addToGroup( new AxisItem() );
			m_rootItem->addToGroup( text );
			scene->addItem(m_rootItem);

			m_minScale = .1f;
			m_maxScale = 100000.0f;
			scaleBy(1.0f);

		}

		void CurveEditor::setCoDomainScale( CoDomainScale scl )
		{
			m_coDomainScale = scl;
		}

		void CurveEditor::setCallback( CurveChangedCallback callback )
		{
			m_callback = callback;
		}

		void CurveEditor::addCurve( const std::string &id, const base::FCurve &fcurve )
		{
			CurveItem *ci = new CurveItem( this, fcurve );
			m_curves[id] = ci;
			m_rootItem->addToGroup( ci );

			makeActive( id );
		}

		void CurveEditor::removeCurve( const std::string &id )
		{
			CurveItem *ci = getCurveItem(id);
			delete ci;
		}

		void CurveEditor::makeActive( const std::string &id )
		{
			CurveItem *ci = getCurveItem(id);
			if( ci )
			{
				if(m_activeCurve)
					m_activeCurve->makeInactive();
				ci->makeActive();
				m_activeCurve = ci;
			}
		}

		CurveItem *CurveEditor::getCurveItem( const std::string &id )
		{
			CurveMap::iterator it = m_curves.find( id );
			if( it != m_curves.end() )
				return it->second;
			return 0;
		}

		std::string CurveEditor::getCurveId( CurveItem *curve )
		{
			for( CurveMap::iterator it = m_curves.begin(); it != m_curves.end(); ++it )
			{
				if( it->second == curve )
					return it->first;
			}
			return "";
		}

		void CurveEditor::paintEvent( QPaintEvent * event )
		{

			QGraphicsView::paintEvent(event);

			QPainter painter(viewport());
			QPen pen(Qt::black, 3, Qt::SolidLine);
			painter.setPen(pen);
			

			// draw scales
			int domain_spacing = 100;
			int length = 25;
			int w = width();
			int h = height();

			// domain
			for(int i=0;i<10;++i)
			{
				QPointF offset = QPointF(domain_spacing, h) + QPointF( (float)(i*domain_spacing), 0.0f );
				painter.drawLine( offset, offset - QPointF(0.0f, length) );
			}

			// co-domain
			for(int i=0;i<10;++i)
			{
				QPointF offset = QPointF( 0.0, domain_spacing) + QPointF( 0.0f, (float)(i*domain_spacing) );
				painter.drawLine( offset, offset + QPointF(length,0.0f) );
			}

		}

		QRectF CurveEditor::getBound( base::FCurve &curve )
		{
			//TODO: make it right
			QRectF result;
			result.setBottom( -99999999999.0f );
			result.setTop( 99999999999.0f );
			result.setLeft( 99999999999.0f );
			result.setRight( -99999999999.0f );
			for( int i=0; i<curve.m_x.size(); ++i )
			{
				result.setBottom( std::max( (float) result.bottom(), applyCoDomainScale(curve.m_values[i]) ) );
				result.setTop( std::min((float)result.top(), applyCoDomainScale(curve.m_values[i]) ) );
				result.setLeft( std::min((float)result.left(), (float)curve.m_x[i] ) );
				result.setRight( std::max((float)result.right(), (float)curve.m_x[i] ) );
			}

			return result;
		}

		void CurveEditor::keyPressEvent(QKeyEvent *event)
		{
			if( event->key() == Qt::Key_F )
			{
				// focus all
				QRectF newSceneRect;
				// focus
				if(this->scene()->selectedItems().size())
				{
					// focus selected
				}else
				{

					// compute bounding box of all curves
					for( CurveMap::iterator it = m_curves.begin(); it != m_curves.end(); ++it )
					{
						QRectF bound = getBound(it->second->curve());
						if( bound.isValid() )
						{
							newSceneRect = newSceneRect.united(bound);
						}
					}
				}

				if( newSceneRect.isValid() )
				{
					// move to center
					QList<QGraphicsItem*> childs = m_rootItem->childItems();
					for(QList<QGraphicsItem*>::iterator it = childs.begin(); it != childs.end(); ++it)
					{
						QGraphicsItem *gi = *it;
						gi->setPos( -newSceneRect.center().x(), -newSceneRect.center().y() );
					}

					// scale scene so that rect fits
					// get scene rect
					QPointF topLeft = mapToScene( 0, 0 );
					QPointF bottomRight = mapToScene( viewport()->width() - 1, viewport()->height() - 1 );
					QRectF rect = QRectF( topLeft, bottomRight );
					float sca = m_rootItem->transform().m11();

					if( newSceneRect.width() > newSceneRect.height() )
					{
						float newScale = (float)rect.width()/(float)newSceneRect.width();
						scaleBy(newScale/sca);
					}else
					{
						float newScale = (float)rect.height()/(float)newSceneRect.height();
						scaleBy(newScale/sca);
					}
				}

				event->accept();
			}
		}

		void CurveEditor::drawBackground ( QPainter * painter, const QRectF & rect )
		{
			// gray
			painter->fillRect( rect, Qt::darkGray );
		}



		void CurveEditor::mousePressEvent( QMouseEvent * event )
		{
			QGraphicsView::mousePressEvent(event);

			// if a mousebutton had been pressed
			if( !event->isAccepted() && (event->buttons() == Qt::LeftButton) )
			{
				//create CP
				if(m_activeCurve)
				{
					QPointF pscene = mapToScene(event->pos());
					QPointF p;
					p = m_activeCurve->mapFromScene( pscene );
					m_activeCurve->insertCP(p);
					m_activeCurve->updateScale(m_rootItem->transform().m11(),m_rootItem->transform().m22());
				}

				event->accept();
				update();
			}
		}

		void CurveEditor::mouseMoveEvent( QMouseEvent * event )
		{
			Qt::MouseButtons buttons = event->buttons();

			// widget coordinates
			QPoint pos_widget = event->pos();
			QPoint dpos_widget = pos_widget - m_lastMousePos;

			// scene coordinates
			QPointF pos_scene = mapToScene(pos_widget);
			QPointF dpos_scene = pos_scene - mapToScene(m_lastMousePos);

			m_lastMousePos = pos_widget;

			// if a mousebutton had been pressed
			if( buttons != Qt::NoButton )
			{

				if( buttons & Qt::MidButton )
				{
					// pan
					float sc = m_rootItem->transform().m11();

					if( sc>1.0f )
					{
						sc = 1.0f/sc*3.0f;
					}else
						sc = 1.0f;

					QList<QGraphicsItem*> childs = m_rootItem->childItems();
					for(QList<QGraphicsItem*>::iterator it = childs.begin(); it != childs.end(); ++it)
					{
						(*it)->moveBy( dpos_scene.x()*sc,dpos_scene.y()*sc );
					}
					event->accept();
					update();
				}else
				if( buttons & Qt::RightButton )
				{
					// zoom
					float l = sqrt(dpos_scene.x()*dpos_scene.x() + dpos_scene.y()*dpos_scene.y());
					float sign = 1.0f;
					if(dpos_scene.x() < 0.0f)
						sign = -1.0f;
					l = l*0.015f;
					scaleBy(1.0 + sign*l);

					event->accept();
					update();
				}
			}

			QGraphicsView::mouseMoveEvent(event);
		}

		void CurveEditor::curveChangedEvent( CurveItem *curve )
		{
			if( m_callback )
				m_callback( getCurveId(curve), curve->curve() );
		}


		void CurveEditor::scaleBy(qreal scaleFactor)
		{
			qreal curScaleFactor = m_rootItem->transform().m11();

			if (((curScaleFactor == m_minScale) && (scaleFactor < 1.0)) ||
				((curScaleFactor == m_maxScale) && (scaleFactor > 1.0))) return;
 
			qreal sc = scaleFactor;
 
			if ((curScaleFactor * sc < m_minScale)&&(sc < 1.0))
			{
				sc = m_minScale / curScaleFactor;
			}
			else
				if ((curScaleFactor * sc > m_maxScale)&&(sc > 1.0))
				{
				sc = m_maxScale / curScaleFactor;
			}

			this->m_rootItem->scale(sc, sc);

			// adjust bounding rect of all handles
			// this is a workaround for items which shall draw itsself scale invariant (while being transformed properly) like cv handles
			for( CurveMap::iterator it = m_curves.begin(); it != m_curves.end(); ++it )
			{
				CurveItem *ci = it->second;
				ci->updateScale(m_rootItem->transform().m11(),m_rootItem->transform().m22());
			}
		}
 
		void CurveEditor::setZoom(int percentZoom)
		{
			qreal targetScale = (qreal)percentZoom / 100.0;
			qreal scaleFactor = targetScale / transform().m11();
			scaleBy(scaleFactor);
		}

		float CurveEditor::applyCoDomainScale( float codomainValue )
		{
			float codomainScaleValue = codomainValue;

			switch(m_coDomainScale)
			{
			case LOG10:
				codomainScaleValue = log10(codomainScaleValue+1.0f);
				break;
			case LINEAR:
			default:
				// nop
				break;
			};

			// due to the nature of qtgraphicsscene coordinate system we flip verticals
			codomainScaleValue = -codomainScaleValue;

			return codomainScaleValue;
		}

		float CurveEditor::applyCoDomainScaleInverse( float codomainScaleValue )
		{
			float codomainValue = codomainScaleValue;

			// due to the nature of qtgraphicsscene coordinate system we flip verticals
			codomainValue = -codomainValue;

			switch(m_coDomainScale)
			{
			case LOG10:
				codomainValue = pow(10.0f, codomainValue)-1.0f;
				break;
			case LINEAR:
			default:
				// nop
				break;
			};
			

			return codomainValue;
		}
	}
}
