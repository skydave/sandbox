#pragma once

#include <QtOpenGL/QGLWidget>
#include <QMouseEvent>

#include <gfx/OrbitNavigator.h>
#include <gfx/Camera.h>

#include "GLThread.h"

namespace composer
{
	namespace widgets
	{
		class GLViewer : public QGLWidget
		{
			Q_OBJECT        // must include this if you use Qt signals/slots

		public:
			typedef void (*InitCallback)( void );
			typedef void (*RenderCallback)( base::CameraPtr );

			GLViewer( InitCallback init = 0, RenderCallback render = 0, QWidget *parent = 0 );
			virtual ~GLViewer();

			base::CameraPtr getCamera();
		public slots:
			void setRenderInSeperateThread( bool state );
		public:
		protected:

			void initializeGL();
			void resizeGL(int w, int h);
			void paintGL();
			void mouseMoveEvent( QMouseEvent * event );
		private:
			InitCallback m_init;
			RenderCallback m_render;
			base::OrbitNavigator m_orbitNavigator;
			int m_lastX;
			int m_lastY;

			GLThread m_renderThread;

			friend class GLThread;
		};
	}
}
