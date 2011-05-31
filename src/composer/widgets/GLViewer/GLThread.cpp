//
//
//
// linux:
// Multithreading opengl is straight forward when making sure that the thread which is doing the glCalls
// has done makeCurrent on the right rendercontext. The thread also has to do doneCurrent in order to allow
// other threads to pick it up again.
//
// The problem with linux + qt multithreading + opengl is that XLib is not threadsafe. This has the effect
// that calls to e.g. makeCurrent which results in X11 calls screw up the protocol because different threads
// might make X11 calls concurrently. To concurrent X11 calls XLib is to be prepared for multithreading which
// is done by calling XInitThreads. Then XLockDisplay and XUnlockDisplay functions are used to serialise
// concurrent X11 calls. Check the run method of GLThread.
//
// One problem remains, which is occasional "QGLContext::makeCurrent(): Failed" errors. This happens when QGLWidget
// tries to aquire the rendercontext which has been taken by another thread (using make current) and which is not
// released (doneCurrent).
// One occasion happens when we kick of the render thread at the end of the GLViewer::initializeGL function. The next
// thing qt tries to do after calling this function is a glPaint or glResize of something similar. Since the thread has
// taken the renderContext (for rendering) makeCurrent now fails for qt. It hasnt been an issue so far.
// Another occasion is when closing the window. The thread is still running but the renderContext has been destroyed already.
//
//
//
#include "GLThread.h"
#include <iostream>
#include <gfx/Context.h>

#include "GLViewer.h"

#ifdef linux
#include <QX11Info>
#endif

extern base::ContextPtr context;


namespace composer
{
	namespace widgets
	{

		#ifdef linux
		// we need to call XInitThreads.
		// doing it here before main in order to prevent users from forgetting to put it in
		struct XInitThreadsInitialiser
		{
		   XInitThreadsInitialiser()
		   {
				XInitThreads();
				std::cout << "GLThread.cpp::XInitThreads" << std::endl;
		   }
		};
		XInitThreadsInitialiser g_XInitThreadsInitialiser;
		#endif



		GLThread::GLThread(GLViewer *glViewer) : QThread(), m_glViewer(glViewer)
		{
			doRendering = true;
			doResize = false;
		}
    
		void GLThread::stop()
		{
			doRendering = false;
		}
    
		void GLThread::resizeViewport(const QSize &size)
		{
			w = size.width();
			h = size.height();
			doResize = true;
		}


		void GLThread::run()
		{
			doRendering = true;


			#ifdef linux
			Display *d = m_glViewer->x11Info().display();
			XLockDisplay(d);
			#endif
			m_glViewer->makeCurrent();
			#ifdef linux
			XUnlockDisplay(d);
			#endif

			m_stopWatch.start();

			while (doRendering)
			{
				if (doResize)
				{
					glViewport(0, 0, w, h);
					doResize = false;
				}

				context->getUniform("time")->set(0, m_stopWatch.elapsedSeconds());

				// Rendering code goes here
				if(m_glViewer->m_render)
					m_glViewer->m_render(m_glViewer->m_orbitNavigator.m_camera);

				m_glViewer->swapBuffers();
				msleep(40);
			}

			#ifdef linux
			XLockDisplay(d);
			#endif
			m_glViewer->doneCurrent();
			#ifdef linux
			XUnlockDisplay(d);
			#endif
		}
	}
}
