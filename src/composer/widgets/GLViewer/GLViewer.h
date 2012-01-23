#pragma once

#include <QtOpenGL/QGLWidget>
#include <QMouseEvent>

#include <gfx/OrbitNavigator.h>
#include <gfx/Camera.h>

#include <ui/EventInfo.h>


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
			typedef void (*ShutdownCallback)( void );
			typedef void (*MouseMoveCallback)( base::MouseState state );
			typedef void (*KeyPressCallback)( base::KeyboardState &state );

			GLViewer( InitCallback init = 0, RenderCallback render = 0, ShutdownCallback shutdown = 0, QWidget *parent = 0 );
			virtual ~GLViewer();

			base::CameraPtr                                                               getCamera();
			void        setView( math::Vec3f lookat, float distance, float azimuth, float elevation );
			void                          setMouseMoveCallback( MouseMoveCallback mouseMoveCallback );
			void                             setKeyPressCallback( KeyPressCallback keyPressCallback );
		public slots:
			void setRenderInSeperateThread( bool state );
		public:
		protected:

			void                             initializeGL();
			void                     resizeGL(int w, int h);
			void                                  paintGL();
			void      mouseMoveEvent( QMouseEvent * event );
			void         keyPressEvent( QKeyEvent * event );
			void       keyReleaseEvent( QKeyEvent * event );
		private:
			InitCallback                             m_init;
			RenderCallback                         m_render;
			ShutdownCallback                     m_shutdown;
			MouseMoveCallback                   m_mouseMove;
			KeyPressCallback                     m_keyPress;
			base::OrbitNavigator           m_orbitNavigator;
			int                                     m_lastX;
			int                                     m_lastY;
			bool                            m_isInitialized;
			bool                   m_renderInSeperateThread;

			GLThread                         m_renderThread;

			base::KeyboardState             m_keyboardState;
			std::map<int, int>                      m_qtKey; // maps qt keys to our keys

			friend class GLThread;
		};
	}
}
