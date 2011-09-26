#include "GLViewer.h"

#include <iostream>



namespace composer
{
	namespace widgets
	{
		GLViewer::GLViewer( InitCallback init, RenderCallback render, QWidget *parent ) : QGLWidget(parent), m_init(init), m_render(render), m_lastX(0), m_lastY(0), m_renderThread(this)
		{
			setMouseTracking( true );
			// this will make sure swapbuffers is not called by qt when doing double buffering
			setAutoBufferSwap(false);
		}


		GLViewer::~GLViewer()
		{
			if( m_renderThread.isRunning() )
			{
				m_renderThread.stop();
				m_renderThread.wait();
			}
		}

		base::CameraPtr GLViewer::getCamera()
		{
			return m_orbitNavigator.m_camera;
		}

		void GLViewer::setView( math::Vec3f lookat, float distance, float azimuth, float elevation )
		{
			m_orbitNavigator.m_lookAt = lookat;
			m_orbitNavigator.m_distance = distance;
			m_orbitNavigator.m_azimuth = azimuth;
			m_orbitNavigator.m_elevation = elevation;
			m_orbitNavigator.update();

			if( !m_renderThread.isRunning() )
				update();
		}

		void GLViewer::initializeGL()
		{
			if(m_init)
				m_init();
		}

		void GLViewer::resizeGL(int w, int h)
		{
			// setup viewport, projection etc.:
			if( m_renderThread.isRunning() )
			{
				m_renderThread.resizeViewport( QSize(w,h) );
			}else
				glViewport(0, 0, (GLint)w, (GLint)h);
		}

		void GLViewer::paintGL()
		{
			if( !m_renderThread.isRunning() )
			{
				if(m_render)
					m_render(m_orbitNavigator.m_camera);
				swapBuffers();
			}
		}
		void GLViewer::mouseMoveEvent( QMouseEvent * event )
		{
			Qt::MouseButtons buttons = event->buttons();
			int dx = event->x() - m_lastX;
			int dy = event->y() - m_lastY;
			m_lastX = event->x();
			m_lastY = event->y();


			// if a mousebutton had been pressed
			if( buttons != Qt::NoButton )
			{

				if( buttons & Qt::LeftButton )
				{
					m_orbitNavigator.orbitView( (float)(dx)*0.5f,(float) (dy)*0.5f );
				}else
				if( buttons & Qt::RightButton )
				{
					// Alt + RMB => move camera along lookat vector
					m_orbitNavigator.zoomView( -dx*m_orbitNavigator.getDistance()*0.005f );
				}else
				if( buttons & Qt::MidButton )
				{// MMBUTTON
					m_orbitNavigator.panView( (float)dx, (float)-dy );
				}

				if( !m_renderThread.isRunning() )
					update();
			}
		}

		void GLViewer::setRenderInSeperateThread( bool state )
		{
			if(state)
			{
				doneCurrent();
				// start seperate render thread
				m_renderThread.start();
			}else
			{
				// notify to stop seperate render thread
				m_renderThread.stop();
				// wait for renderthread to finish
				m_renderThread.wait();
				makeCurrent();
			}
		}
	}
}
