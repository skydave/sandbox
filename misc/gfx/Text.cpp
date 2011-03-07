#include "Text.h"
#include "../sys/msys.h"

static unsigned int g_fontLists;
static unsigned int g_fontInitialised = false;

Text::Text( const char *text )
{
	if(!g_fontInitialised)
	{
		// initialise fontlists
		g_fontLists = msys_buildFont();
		g_fontInitialised = true;
	}
}


void Text::render()
{
	char *t = "hicknhack";
	glPushAttrib( GL_LIST_BIT );
	glColor3f(1.0f, 0.0f, 0.0f);
	glListBase( g_fontLists );
	glCallLists( 9, GL_UNSIGNED_BYTE, t );
	glPopAttrib();
}
