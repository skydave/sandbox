#pragma once
#include <map>

#include <QObject>
#include <QGraphicsItem>
#include <QList>
#include <QFont>

#include <math/Math.h>

#include <gfx/OrbitNavigator.h>
#include <gfx/Camera.h>


namespace composer
{
	namespace widgets
	{
		class Trackball;
		class ProjectorItem : public QGraphicsItem
		{
		public:
			ProjectorItem();

			void addItem( QGraphicsItem *item, const math::Vec3f &pos );
			void set3dPos( QGraphicsItem *item, const math::Vec3f &pos );
			math::Vec3f get3dPos( QGraphicsItem *item );

		protected:
			QRectF boundingRect() const;
			void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

		private:
			math::Vec3f project( const math::Vec3f &pos );
			math::Vec3f mapToCamera( const math::Vec3f &pos );

			typedef std::map<QGraphicsItem *, math::Vec3f> ItemMap;
			ItemMap m_items; // maps containing items to their 3d positions
			float m_imagePlaneDistance; // fov
			math::Vec3f m_eyePos;
			//float m_elevation;
			//float m_azimuth;

			base::Camera m_camera;

			friend class Trackball;
		};
	}
}
