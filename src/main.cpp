//============================================================================
//
//
//
//============================================================================
#include <stdio.h>
#include <cstdlib>
#include <string>
#include <iostream>

#include <math/Vec3f.h>
#include <ui/GLWindow.h>

//#ifdef WINDOWS
//#include <windows.h>
//#endif
//#include <GL/gl.h>
/*
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glut.h>
*/




#define XRES 640
#define YRES 480












int main(int argc, char ** argv)
{
	base::GLWindow window( 800, 600, "test" ); 
	window.show();
	base::Application app;
	return app.exec();
}
