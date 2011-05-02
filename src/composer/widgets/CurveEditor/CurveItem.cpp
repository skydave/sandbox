#include "CurveItem.h"
#include <QPainter>

#include "CurveEditor.h"



namespace composer
{
	namespace widgets
	{
		CurveItem::CurveItem(CurveEditor *editor) : QGraphicsItem(), m_editor(editor), m_scaleX(1.0f), m_scaleY(1.0f), m_isActive(false)
		{
			setHandlesChildEvents(false);
		}
		CurveItem::CurveItem( CurveEditor *editor, const base::FCurve &curve ) : QGraphicsItem(), m_editor(editor), m_curve(curve), m_scaleX(1.0f), m_scaleY(1.0f), m_isActive(false)
		{
			setHandlesChildEvents(false);
		}

		QRectF CurveItem::boundingRect() const
		{
			//TODO: compute from handles?
			return QRectF(-10000.0f, -10000.0f, 20000.0f, 20000.0f);
		}

		void CurveItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
		{
			QPen pen(Qt::lightGray, 2, Qt::SolidLine);
			pen.setCosmetic ( true );
			painter->setPen(pen);



			// draw from handles if active
			if( m_isActive )
			{
				if( m_handles.isEmpty() )
					return;

				switch(m_curve.m_type)
				{
				case base::FCurve::LINEAR:
				default:
					{
						HandleList::iterator it = m_handles.begin();
						QPainterPath path;
						{
							CurveHandleItem *chi = *it;
							path.moveTo(chi->pos().x(), chi->pos().y());
						}
						for(;it != m_handles.end(); ++it)
						{
							CurveHandleItem *chi = *it;
							path.lineTo(chi->pos().x(), chi->pos().y());
						}
						painter->drawPath(path);
					}
					break;
				}
			}else
			{
				if( m_curve.isEmpty() )
					return;

				// draw from m_curve
				switch(m_curve.m_type)
				{
				case base::FCurve::LINEAR:
				default:
					{
						QPainterPath path;
						{
							path.moveTo(m_curve.m_x[0],m_editor->applyCoDomainScale(m_curve.m_values[0]));
						}
						for(int i=1;i<m_curve.m_x.size(); ++i)
						{
							path.lineTo(m_curve.m_x[i],m_editor->applyCoDomainScale(m_curve.m_values[i]));
						}
						painter->drawPath(path);
					}
					break;
				}
			}
		}

		base::FCurve &CurveItem::curve()
		{
			if( m_isActive )
			{
				m_curve.clear();

				// update from curvehandles
				for( HandleList::iterator it = m_handles.begin(); it != m_handles.end();++it )
				{
					CurveHandleItem *chi = *it;
					m_curve.addCP( chi->pos().x(), m_editor->applyCoDomainScaleInverse( chi->pos().y() ) );
				}
			}
			return m_curve;
		}

		void CurveItem::updateScale( float newScaleX, float newScaleY )
		{
			// update scale of all handles
			for( HandleList::iterator it = m_handles.begin(); it != m_handles.end(); ++it )
			{
				(*it)->updateScale( newScaleX, newScaleY );
			}
		}

		void CurveItem::insertCP( const QPointF &p )
		{
			CurveHandleItem *chi = new CurveHandleItem(this, m_scaleX, m_scaleY);
			chi->m_propagateChanges = false;
			chi->setPos(p);
			chi->setParentItem(this);
			chi->m_propagateChanges = true;

			m_handles.push_back( chi );

			update(chi);
		}

		void CurveItem::makeActive()
		{
			// create handles
			for( int i=0;i<m_curve.m_x.size();++i )
			{
				CurveHandleItem *chi = new CurveHandleItem(this);
				chi->m_propagateChanges = false;
				chi->setPos(m_curve.m_x[i],m_editor->applyCoDomainScale(m_curve.m_values[i]));
				chi->setParentItem(this);
				chi->m_propagateChanges = true;

				m_handles.push_back( chi );
			}

			m_isActive = true;
		}

		void CurveItem::makeInactive()
		{
			for(HandleList::iterator it = m_handles.begin();it != m_handles.end(); ++it)
			{
				CurveHandleItem *chi = *it;
				delete chi;
			}
			m_handles.clear();
			// clear handles
			m_isActive = false;
		}

		void CurveItem::update( CurveHandleItem *handle )
		{
			prepareGeometryChange();

			//if( !m_handles.isEmpty() )
			{
				// make sure handle list is still sorted
				m_handles.removeOne( handle );
				HandleList::iterator it = m_handles.begin();
				for( ; it != m_handles.end();++it )
				{
					CurveHandleItem *chi = *it;
					if( handle->pos().x() < chi->pos().x() )
					{
						m_handles.insert( it, handle );
						break;
					}
				}
				if( it == m_handles.end() )
					m_handles.push_back(handle);
			}

			m_editor->curveChangedEvent( this );
		}
	}
}