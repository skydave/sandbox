#include "ProjectorItem.h"
#include <iostream>
#include <QPainter>


namespace composer
{
	namespace widgets
	{
		ProjectorItem::ProjectorItem() : QGraphicsItem()
		{
			//setFlag(ItemIsMovable);
			//setFlag(ItemSendsGeometryChanges);
			m_imagePlaneDistance = 100.0f;
			m_eyePos = math::Vec3f(0.0f, 0.0f,0.0f);

			m_camera.m_transform.translate(0.0f, 0.0f, 1.5f);
			m_camera.m_fov = 30.0f;
			m_camera.update();
		}

		void ProjectorItem::addItem( QGraphicsItem *item, const math::Vec3f &pos )
		{
			set3dPos( item, pos );
			item->setParentItem( this );
		}

		void ProjectorItem::set3dPos( QGraphicsItem *item, const math::Vec3f &pos )
		{
			m_items[item] = pos;

			// update screenpos
			math::Vec3f camPos = mapToCamera(pos);
			math::Vec3f clipPos = project(camPos);
			//std::cout << "worldspace: " << pos.x << " " << pos.y << " " << pos.z << std::endl;
			//std::cout << "camspace: " << camPos.x << " " << camPos.y << " " << camPos.z << std::endl;
			//std::cout << "clipspace: " << clipPos.x << " " << clipPos.y << " " << clipPos.z << std::endl;

			if( clipPos.z > 0.0f )
			{
				// http://homepages.uni-paderborn.de/prefect/glmath/spaces.html
				math::Vec3f ndcPos = clipPos / (-clipPos.z);
				//std::cout << "ndcspace: " << ndcPos.x << " " << ndcPos.y << " " << ndcPos.z <<  std::endl;

				item->show();

				QPointF newPos(ndcPos.x, ndcPos.y);
				item->setPos(newPos);
			}else
				item->hide();


		}

		math::Vec3f ProjectorItem::get3dPos( QGraphicsItem *item )
		{
			ItemMap::iterator it = m_items.find(item);
			if(it != m_items.end())
				return it->second;
			return math::Vec3f();
		}


		/*
		void ProjectorItem::rotateItems( float azimuth, float elevation )
		{
			QList<QGraphicsItem*> childs = childItems();
			for(QList<QGraphicsItem*>::iterator it = childs.begin(); it != childs.end(); ++it)
			{
				QGraphicsItem *item = *it;
				math::Vec3f pos;
				if( is3d(item, &pos) )
				{
					//(*it)->moveBy( dpos_scene.x()*sc,dpos_scene.y()*sc );
					math::Matrix44f m = math::Matrix44f::Identity();

					float lookAtZ = 100.0f;
					//pos = pos - math::Vec3f(0.0f, 0.0f, lookAtZ);

					m.translate( pos );	

					//m.rotateZ( twist ); // not used
					//m.rotateX( math::degToRad(elevation) );
					m.rotateY( math::degToRad(azimuth) );

					//m.translate( math::Vec3f(0.0f, 0.0f, 0.0f) );

					pos = m.getTranslation();

					//pos = pos + math::Vec3f(0.0f, 0.0f, lookAtZ);

					if( pos.z > 0.0f )
					{
						if( !item->isVisible() )
							item->show();

						QPointF newScreenPos = project(pos);
						item->setPos(newScreenPos);
						std::cout << "pos2d: " << newScreenPos.x() << " " << newScreenPos.y() << std::endl;
					}else
					{
						item->hide();
					}
					//pos = pos + math::Vec3f(0.0f, 0.0f, lookAtZ);
					m_3dPos[item] = pos;
					std::cout << "pos3d: " << pos.x << " " << pos.y << " " << pos.z << std::endl;
				}
			}
		}
		*/


		math::Vec3f ProjectorItem::mapToCamera( const math::Vec3f &pos )
		{
			return math::transform( pos, m_camera.m_viewMatrix );
		}

		math::Vec3f ProjectorItem::project( const math::Vec3f &pos )
		{
			return math::transform( pos, m_camera.m_projectionMatrix );
		}

		QRectF ProjectorItem::boundingRect() const
		{
			return QRectF(-10000.0f, -10000.0f, 20000.0f, 20000.0f);
		}


		void ProjectorItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
		{
			//QGraphicsItem::paint(painter, option, widget);
			QPen pen(Qt::lightGray, 2, Qt::SolidLine);
			pen.setCosmetic ( true );
			painter->setPen(pen);

			painter->drawLine( -10000, 0, 10000, 0 );
			painter->drawLine( 0, -10000, 0, 10000 );
		}
	}
}
