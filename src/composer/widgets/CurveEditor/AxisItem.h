#pragma once
#include <QObject>
#include <QGraphicsItem>
#include <QList>
#include <QFont>



namespace composer
{
	namespace widgets
	{
		class AxisItem : public QGraphicsItem
		{
		public:
			AxisItem();


		protected:
			QRectF boundingRect() const;
			void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

		private:

		};
	}
}
