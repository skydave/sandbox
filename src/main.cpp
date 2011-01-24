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



#define XRES 640
#define YRES 480











//#include "../../intro.h"
//#include "../../sys/msys.h"
//#include "../mys_malloc.h"
//#include "../mys_font.h"

//----------------------------------------------------------------------------
/*
typedef struct
{
	//---------------
	Display     *hDisplay;
	GLXContext  hRC;
	Window      hWnd;
	//---------------
	int         full;
	//---------------
	char        wndclass[11];	// window class and title :)
	//---------------
	MSYS_EVENTINFO   events;
}WININFO;

static int doubleBufferVisual[]  =
{
		GLX_RGBA,           // Needs to support OpenGL
		GLX_DEPTH_SIZE, 24, // Needs to support a 16 bit depth buffer
		GLX_DOUBLEBUFFER,   // Needs to support double-buffering
		None                // end of list
};


static WININFO wininfo = {  0,0,0,
							1,	// full
							{'t','e','s','t','_','t','e','s','t','t',0}
							};







MSYS_EVENTINFO *getEvents( void )
{
	return &wininfo.events;
}

int getKeyPress( int key )
{
	int res;

	res = wininfo.events.keyb.state[key];
	wininfo.events.keyb.state[key] = 0;
	return res;
}

static void queryState( void )
{
	Window root_return;
	Window child_return;
	int root_x_return;
	int root_y_return;
	int win_x_return;
	int win_y_return;
	unsigned int mask_return;

	bool result = XQueryPointer(wininfo.hDisplay, wininfo.hWnd, &root_return, &child_return,  &root_x_return, &root_y_return, &win_x_return, &win_y_return, &mask_return);

	if (result)
	{
		wininfo.events.mouse.ox = wininfo.events.mouse.x;
		wininfo.events.mouse.oy = wininfo.events.mouse.y;
		wininfo.events.mouse.x = win_x_return;
		wininfo.events.mouse.y = win_y_return;
		wininfo.events.mouse.dx =  wininfo.events.mouse.x - wininfo.events.mouse.ox;
		wininfo.events.mouse.dy =  wininfo.events.mouse.y - wininfo.events.mouse.oy;

		wininfo.events.mouse.obuttons[0] = wininfo.events.mouse.buttons[0];
		wininfo.events.mouse.obuttons[1] = wininfo.events.mouse.buttons[1];
		wininfo.events.mouse.buttons[0] = (mask_return & 256)?1:0;
		wininfo.events.mouse.buttons[2] = (mask_return & 512)?1:0;
		wininfo.events.mouse.buttons[1] = (mask_return & 1024)?1:0;

		wininfo.events.mouse.dbuttons[0] = wininfo.events.mouse.buttons[0] - wininfo.events.mouse.obuttons[0];
		wininfo.events.mouse.dbuttons[1] = wininfo.events.mouse.buttons[1] - wininfo.events.mouse.obuttons[1];
	}
}

//----------------------------------------------------------------------------

static void window_end( WININFO *info )
{
	XDestroyWindow( info->hDisplay, info->hWnd );
	XCloseDisplay( info->hDisplay );
}

static int window_init( WININFO *info )
{
	XVisualInfo *visualInfo;
	int         errorBase;
	int         eventBase;

	info->hDisplay = XOpenDisplay( NULL );
	if( !info->hDisplay )
		return( 0 );

	// Make sure OpenGL's GLX extension supported
	if( !glXQueryExtension( info->hDisplay, &errorBase, &eventBase ) )
		return( 0 );

	// Try for the double-bufferd visual first
	visualInfo = glXChooseVisual( info->hDisplay, DefaultScreen(info->hDisplay), doubleBufferVisual );
	if( visualInfo == NULL )
		return( 0 );

	// Create an OpenGL rendering context
	info->hRC = glXCreateContext( info->hDisplay, visualInfo, NULL, GL_TRUE );
	if( info->hRC == NULL )
		return( 0 );

	// Create an X colormap since we're probably not using the default visual
	Colormap colorMap;
	colorMap = XCreateColormap( info->hDisplay, RootWindow(info->hDisplay, visualInfo->screen),
								visualInfo->visual, AllocNone );

	XSetWindowAttributes winAttr;
	winAttr.colormap     = colorMap;
	winAttr.border_pixel = 0;
	winAttr.event_mask   = ExposureMask           |
						   VisibilityChangeMask   |
						   KeyPressMask           |
						   KeyReleaseMask         |
						   ButtonPressMask        |
						   ButtonReleaseMask      |
						   PointerMotionMask      |
						   StructureNotifyMask    |
						   SubstructureNotifyMask |
						   FocusChangeMask;

	// Create an X window with the selected visual
	info->hWnd = XCreateWindow( info->hDisplay, RootWindow(info->hDisplay, visualInfo->screen),
							  0, 0, XRES, YRES, 0, visualInfo->depth, InputOutput,
				visualInfo->visual, CWBorderPixel | CWColormap | CWEventMask,
							  &winAttr );

	if( !info->hWnd )
		return( 0 );

	//XSetStandardProperties( info->hDisplay, info->hWnd, info->wndclass, info->wndclass, None, argv, argc, NULL );
	char *argv[] = { "hola", 0 };
	XSetStandardProperties( info->hDisplay, info->hWnd, info->wndclass,
	info->wndclass, None, argv, 1, NULL );

	glXMakeCurrent( info->hDisplay, info->hWnd, info->hRC );


	XMapWindow( info->hDisplay, info->hWnd );


	return( 1 );
}

*/
//----------------------------------------------------------------------------

int main( void )
{
/*
	XEvent      event;
	int         done=0;
	WININFO     *info = &wininfo;


	if( !msys_init(0) )
	{
		#ifdef ERRORMSG
		printf( "msys_init()!" );
		#endif
		return( 1 );
	}


	if( !window_init(info) )
	{
		window_end( info );
		#ifdef ERRORMSG
			printf( "window_init()!" );
		#endif
		return( 2 );
	}


	g_client.init();

	while( !done )
	{
		queryState();
		done = g_client.update();
		g_client.m_game.render();

		wininfo.events.keyb.state[KEY_LEFT]     = 0;
		wininfo.events.keyb.state[KEY_RIGHT]    = 0;
		wininfo.events.keyb.state[KEY_UP]       = 0;
		wininfo.events.keyb.state[KEY_DOWN]     = 0;
		wininfo.events.keyb.state[KEY_PGUP]     = 0;

		while( XPending(info->hDisplay) )
		{
			XNextEvent( info->hDisplay, &event );
			switch( event.type )
			{
			case KeyPress:
				switch( XKeycodeToKeysym( info->hDisplay, event.xkey.keycode, 0 ) )
				{
				case XK_Up:
					wininfo.events.keyb.state[KEY_UP] = 1;break;
				case XK_Down:
					wininfo.events.keyb.state[KEY_DOWN] = 1;break;
				case XK_Left:
					wininfo.events.keyb.state[KEY_LEFT] = 1;break;
				case XK_Right:
					wininfo.events.keyb.state[KEY_RIGHT] = 1;break;
				case XK_Escape:
					done = 1;break;
				}break;
			case DestroyNotify:
				done = 1;break;
			}
		}
		glXSwapBuffers( info->hDisplay, info->hWnd );
	}

	g_client.shutdown();

	window_end( info );

	msys_end();
*/
	return( 0 );
}








