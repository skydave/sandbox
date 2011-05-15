#include "GLThread.h"
#include <iostream>
#include <gfx/context.h>

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
			m_glViewer->makeCurrent();
			m_stopWatch.start();
			/*
			srand(QTime::currentTime().msec());
			rotAngle = rand() % 360;

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();        
			glOrtho(-5.0, 5.0, -5.0, 5.0, 1.0, 100.0);
			glMatrixMode(GL_MODELVIEW);
			glViewport(0, 0, 200, 200);
			glClearColor(0.0, 0.0, 0.0, 1.0);
			glShadeModel(GL_SMOOTH);
			glEnable(GL_DEPTH_TEST);
    			*/
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
				std::cout << "rendering\n";
				msleep(40);
			}
			m_glViewer->doneCurrent();
		}
	}
}
