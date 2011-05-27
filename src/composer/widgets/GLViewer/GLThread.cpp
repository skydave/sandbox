#include "GLThread.h"
#include <iostream>
#include <gfx/Context.h>

#include "GLViewer.h"


extern base::ContextPtr context;



namespace composer
{
	namespace widgets
	{
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
			m_glViewer->makeCurrent();
			m_stopWatch.start();


			while (doRendering)
			{
				if (doResize)
				{
					//glViewport(0, 0, w, h);
					doResize = false;
				}
				context->getUniform("time")->set(0, m_stopWatch.elapsedSeconds());
				// Rendering code goes here
				if(m_glViewer->m_render)
					m_glViewer->m_render(m_glViewer->m_orbitNavigator.m_camera);
				m_glViewer->swapBuffers();
				msleep(40);
			}
			m_glViewer->doneCurrent();
		}
	}
}
