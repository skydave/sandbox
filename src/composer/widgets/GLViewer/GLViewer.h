#pragma once
#include <iostream>
#include <ui/GLViewer.h>
#include <QtOpenGL/QGLWidget>
#include <QMouseEvent>
#include <gfx/OrbitNavigator.h>


namespace composer
{
	namespace widgets
	{
		class GLViewer : public QGLWidget
		{
			Q_OBJECT        // must include this if you use Qt signals/slots

		public:
			GLViewer(base::GLViewer::InitCallback init = 0, base::GLViewer::RenderCallback render = 0, QWidget *parent = 0 ) : QGLWidget(parent), m_init(init), m_render(render), m_lastX(0), m_lastY(0)
			{
				setMouseTracking( true );
			}

			base::CameraPtr  getCamera()
			{
				return m_orbitNavigator.m_camera;
			}

		protected:

			void initializeGL()
			{
				if(m_init)
					m_init();
			}

			void resizeGL(int w, int h)
			{
				// setup viewport, projection etc.:
				glViewport(0, 0, (GLint)w, (GLint)h);
			}

			void paintGL()
			{
				if(m_render)
					m_render(m_orbitNavigator.m_camera);
			}

			void mouseMoveEvent( QMouseEvent * event )
			{
				Qt::MouseButtons buttons = event->buttons();
				int dx = event->x() - m_lastX;
				int dy = event->y() - m_lastY;
				m_lastX = event->x();
				m_lastY = event->y();
				//std::cout << dx << " " << dy << std::endl;


				// if a mousebutton had been pressed
				if( buttons != Qt::NoButton )
				{

					if( buttons & Qt::LeftButton )
					{
						m_orbitNavigator.orbitView( (float)(dx)*0.5f,(float) (dy)*0.5f );
						//m_orbitNavigator.orbitView( (float)(dx)*100.0f,(float) (dy)*100.0f );
					}else
					if( buttons & Qt::RightButton )
					{
						// Alt + RMB => move camera along lookat vector
						m_orbitNavigator.zoomView( -dx*m_orbitNavigator.getDistance()*0.005f );
					}else
					if( buttons & Qt::MiddleButton )
					{// MMBUTTON
						m_orbitNavigator.panView( (float)dx, (float)-dy );
					}

					//printf( "%f  %f   %f\n", dbgNav.azimuth, dbgNav.elevation, cam->focalLength );
					update();
				}
			}
		private:
			base::GLViewer::InitCallback m_init;
			base::GLViewer::RenderCallback m_render;
			base::OrbitNavigator m_orbitNavigator;
			int m_lastX;
			int m_lastY;
		};
	}
}