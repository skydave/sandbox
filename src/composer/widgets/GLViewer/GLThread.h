#pragma once

#include <QSize>
#include <QThread>

#include <util/StopWatch.h>

namespace composer
{
	namespace widgets
	{
		class GLViewer;
		class GLThread : public QThread
		{
			Q_OBJECT
		public:
			GLThread(GLViewer *glViewer);
			void resizeViewport(const QSize &size);
			void run();
			void stop();

		signals:
			void swapBuffer( int test );
        
		private:
			bool doRendering;
			bool doResize;
			int w;
			int h;
			int rotAngle;
			GLViewer *m_glViewer;
			base::StopWatch m_stopWatch;
		};
	}
}
