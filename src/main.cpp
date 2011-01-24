//============================================================================
//
//
//
//============================================================================
#include <stdio.h>
#include <cstdlib>
#include <string.h>

#include <X11/X.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glut.h>



#define XRES 640
#define YRES 480









void initGL()
{
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glShadeModel(GL_SMOOTH);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
}

void reshape(int width, int height)
{
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	//gluPerspective(45, (float)width/height, camera->m_near, camera->m_far);
	gluPerspective(45, (float)width/height, 0.01f, 10000.0f);
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glutPostRedisplay();
	glutSwapBuffers();
}


void keydown(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27: //ESC
		exit(0);
		break;
		break;
	default:
		break;
	}

	glutPostRedisplay();
}

void specialKeydown(int key, int, int )
{
}


void idle()
{
	glutPostRedisplay();
}

void processMouse(int button, int state, int x, int y)
{

	glutPostRedisplay();
}

void processMouseEntry(int state)
{
	glutPostRedisplay();
}

void processMousePassiveMotion(int x, int y)
{
	glutPostRedisplay();
}

void processMouseActiveMotion(int x, int y)
{
	// update camera
	glutPostRedisplay();
}



int main(int argc, char ** argv)
{

	// Initialize GLUT
	glutInit(&argc, argv);

	//glutInitDisplayString("rgba alpha double samples>=4");
	glutInitWindowSize(XRES, YRES);
	glutCreateWindow("test");


	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keydown);
	glutMouseFunc(processMouse);
	glutSpecialFunc(specialKeydown);
	glutMotionFunc(processMouseActiveMotion);
	glutPassiveMotionFunc(processMousePassiveMotion);
	glutEntryFunc(processMouseEntry);
	glutIdleFunc(idle);



	initGL();




	glutMainLoop();

	return 0;
}
