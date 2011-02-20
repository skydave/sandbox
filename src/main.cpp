//============================================================================
//
//
//
//============================================================================
#include <stdio.h>
#include <cstdlib>
#include <string>
#include <iostream>

#include <ui/GLViewer.h>
#include <gltools/gl.h>
#include <gltools/misc.h>


void render( base::Camera *cam )
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadMatrixf( cam->m_projectionMatrix.ma );

	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadMatrixf( (GLfloat *)cam->m_viewMatrix.ma );

	// draw scene
	base::drawGrid();

	glMatrixMode( GL_PROJECTION );
	glPopMatrix();

	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();
}




int main(int argc, char ** argv)
{
	base::GLViewer window( 800, 600, "test", render ); 
	window.show();
	base::Application app;
	return app.exec();
}
