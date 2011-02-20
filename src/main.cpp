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





int main(int argc, char ** argv)
{
	base::GLViewer window( 800, 600, "test" ); 
	window.show();
	base::Application app;
	return app.exec();
}
