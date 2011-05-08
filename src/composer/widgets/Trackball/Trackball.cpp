#include "Trackball.h"


#include <math/Math.h>



namespace composer
{
	namespace widgets
	{
		Trackball::Trackball(QWidget *parent) : QGraphicsView(parent), m_callback(0)
		{
			QGraphicsScene *scene = new QGraphicsScene(this);
			scene->setItemIndexMethod(QGraphicsScene::NoIndex);
			scene->setSceneRect(-200, -200, 400, 400);
			//scene->setSceneRect(-1, -1, 2, 2);

			setScene(scene);


			setCacheMode(CacheBackground);
			//setViewportUpdateMode(BoundingRectViewportUpdate);
			setViewportUpdateMode(FullViewportUpdate);
			//setRenderHint(QPainter::Antialiasing);
			setTransformationAnchor(NoAnchor);
			//setAlignment( Qt::AlignJustify );


			setMinimumSize(400, 400);
			setWindowTitle(tr("Elastic Nodes"));
			setMouseTracking( true );

			setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
			setVerticalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );


			//this->fitInView(-1,-1,2,2);
			scale(50.0f, 50.0f);
			centerOn(0.0f, 0.0f);

			
			m_projector = new ProjectorItem();
			scene->addItem(m_projector);




			int uSubdivisions = 10;
			int vSubdivisions = 10;
			float radius = 1.0f;
			float dPhi = MATH_2PIf/uSubdivisions;
			float dTheta = MATH_PIf/vSubdivisions;
			float theta, phi;
			math::Vec3f sphereCenter(0.0f, 0.0f, 0.0f);

			// y
			for (theta=MATH_PIf/2.0f+dTheta;theta<=(3.0f*MATH_PIf)/2.0f-dTheta;theta+=dTheta)
			{
				math::Vec3f p;
				float y = sin(theta);
				// x-z
				phi = 0.0f;
				for( int j = 0; j<uSubdivisions; ++j  )
				{
					p.x = cos(theta) * cos(phi);
					p.y = y;
					p.z = cos(theta) * sin(phi);

					p = radius*p + sphereCenter;

					//positions->appendElement( p );
					QGraphicsRectItem *p1 = new QGraphicsRectItem(0,0,0.15f,0.15f);
					p1->setBrush( QBrush( Qt::black ) );
					addItem( p1, p );

					phi+=dPhi;
				}
			}
			// poles
			{
				QGraphicsRectItem *p1 = new QGraphicsRectItem(0,0,0.15f,0.15f);
				p1->setBrush( QBrush( Qt::red ) );
				addItem( p1, math::Vec3f(0.0f, 1.0f, 0.0f)*radius + sphereCenter );
				m_vecItem = p1;
				QGraphicsRectItem *p2 = new QGraphicsRectItem(0,0,0.15f,0.15f);
				p2->setBrush( QBrush( Qt::black ) );
				addItem( p2, math::Vec3f(0.0f, -1.0f, 0.0f)*radius + sphereCenter );
			}
			//int pole1 = positions->appendElement( math::Vec3f(0.0f, 1.0f, 0.0f)*radius + center );
			//int pole2 = positions->appendElement( math::Vec3f(0.0f, -1.0f, 0.0f)*radius + center );

			QGraphicsTextItem *text = new QGraphicsTextItem();
			QGraphicsRectItem *p1 = new QGraphicsRectItem(0,0,0.5f,0.5f);
			QGraphicsRectItem *p2 = new QGraphicsRectItem(0,0,0.5f,0.5f);
			text->setHtml("deine muddah");
			//addItem( p1, math::Vec3f( 1.0f, 0.0f, 0.0f) );
			//addItem( p2, math::Vec3f(-.2f, 0.0f, -1.0f) );
		}

		void Trackball::setCallback( ChangedCallback callback )
		{
			m_callback = callback;
		}

		void Trackball::addItem( QGraphicsItem *item, const math::Vec3f &pos )
		{
			m_projector->addItem(item, pos);
		}

		void Trackball::mouseMoveEvent( QMouseEvent * event )
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
					float azimuth = dpos_scene.x()*0.1f;
					float elevation = dpos_scene.y()*0.1f;
					//std::cout << "azimuth.elevation: " << azimuth << " " << elevation << std::endl;
					// rotate
					//m_projector->rotateItems( azimuth, elevation );
					moveItems( -dpos_scene.x()*0.1f, -dpos_scene.y()*0.1f, 0.0f );
					//m_projector->moveBy(dpos_scene.x()*0.8f, dpos_scene.y()*0.8f);
					event->accept();
					//update();
				}
				if( buttons & Qt::RightButton )
				{
					float azimuth = dpos_scene.x()*0.1f;
					float elevation = dpos_scene.y()*0.1f;
					//std::cout << "azimuth.elevation: " << azimuth << " " << elevation << std::endl;
					// rotate
					//m_projector->rotateItems( azimuth, elevation );
					//rotateItems( math::Vec3f( 0.0f, 0.0f, -1.0f ), dpos_scene.x()*0.1f, 0.0f );
					moveItems(0.0f, 0.0f, dpos_scene.x()*0.1f);
					event->accept();
					//update();
				}
				if( buttons & Qt::LeftButton )
				{
					float azimuth = -dpos_scene.x()*20.0f;
					float elevation = dpos_scene.y()*20.0f;
					//std::cout << "azimuth: " << azimuth << std::endl;
					// rotate
					//m_projector->rotateItems( azimuth, elevation );
					rotateItems( math::Vec3f( 0.0f, 0.0f, 0.0f ), azimuth, elevation );
					event->accept();
					//update();

					if( m_callback )
						m_callback( m_projector->get3dPos(m_vecItem) );
				}

			}
		}


		void Trackball::drawBackground ( QPainter * painter, const QRectF & rect )
		{
			// gray
			painter->fillRect( rect, Qt::darkGray );
		}

		void Trackball::moveItems( float x, float y, float z )
		{
			for(ProjectorItem::ItemMap::iterator it = m_projector->m_items.begin(); it != m_projector->m_items.end(); ++it)
			{
				QGraphicsItem *item = it->first;
				math::Vec3f pos = it->second;
				pos.x += x;
				pos.y += y;
				pos.z += z;
				m_projector->set3dPos( item, pos );
			}
		}

		void Trackball::rotateItems( math::Vec3f origin, float azimuth, float elevation )
		{
			math::Matrix44f m = math::Matrix44f::Identity();

			m.translate( -origin );
			m.rotateX(math::degToRad(elevation));
			m.rotateY(math::degToRad(azimuth));
			m.translate( origin );

			for(ProjectorItem::ItemMap::iterator it = m_projector->m_items.begin(); it != m_projector->m_items.end(); ++it)
			{
				QGraphicsItem *item = it->first;
				math::Vec3f pos = it->second;

				pos = math::transform( pos, m );

				m_projector->set3dPos( item, pos );
			}
		}
	}
}
