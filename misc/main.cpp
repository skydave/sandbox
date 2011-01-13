/*---------------------------------------------------------------------

Finit-Element Method study

----------------------------------------------------------------------*/
#include "win32/GLWindow.h"
#include "dk/Camera.h"
#include "dk/DebugNavigator.h"
#include "Domain.h"
#include "DomainIO.h"
#include "Solver.h"

using namespace dk;

LRESULT CALLBACK  glWinProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );  // event handling routine
bool                                                               g_done = false;  // indicates finish of the message loop
int                                                             lastMousePosX = 0;  // position of the mouse within the last frame is needed for interaction
int                                                             lastMousePosY = 0;  // position of the mouse within the last frame is needed for interaction

Camera                                                                     camera;
DebugNavigator                                                     debugNavigator;

void                                               renderDomain( Domain *domain );  // displays a visual representation of the given FEM-domain

// FEM stuff
Domain                                                                    *domain;
Solver                                                                     solver;

math::Vec3f                                                               locator;
math::Vec3f                                                        locator_global;

//
// entry point
//
int main(void)
//int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR szCmdLine,int iCmdShow)
{
	GLWindow                  window;

	// load domain from neutral format -> tetrahedral volume mesh (tvm) (exported by NetGen)
	//domain = DomainIO::importFromTVM( "fem_sphere.tvm" );
	domain = DomainIO::importFromTVM( "fem_cube.tvm" );

	// add dofs to the system and set boundary conditions
	for( size_t i = 0; i<domain->getNodeCount(); ++i )
	{
		Node *node = domain->getNode( i );

		// add dofs
		DOF *dofx = node->addDOF(); // x displacement
		DOF *dofy = node->addDOF(); // y displacement
		DOF *dofz = node->addDOF(); // z displacement

		// do we constrain the node displacement?
		// fix all nodes on the bottom
		if( node->getPosition().y < 0.1f )
		{
			// yes, then assign dirichlet boundary conditions
			dofx->assignDirichletBoundaryCondition( node->getPosition().x ); // keep displacement in each direction fixed
			dofy->assignDirichletBoundaryCondition( node->getPosition().y ); //
			dofz->assignDirichletBoundaryCondition( node->getPosition().z ); //
		}
		// apply some fixed displacement on all upper right nodes
		if( (node->getPosition().x > 0.9f) && (node->getPosition().y > 0.9f) )
		{
			dofx->assignDirichletBoundaryCondition( node->getPosition().x + 1.0f ); // keep displacement in each direction fixed
			dofy->assignDirichletBoundaryCondition( node->getPosition().y ); //
			dofz->assignDirichletBoundaryCondition( node->getPosition().z ); //
		}
	}

	// compute node displacements
	solver.solve( domain );

	// perform static analysis -> compute equivalent stress
	//...


	//locator = math::Vec3f( 0.0f, 0.0f, 0.2f );
	locator = math::Vec3f( 0.0f, 1.0f, 0.0f );
	//locator = math::Vec3f( 0.2f, 0.2f, 0.2f );


	printf( "shapefunction: %f\n", domain->getElement(20)->computeShapeFunction( 2, domain->getElement(20)->getGlobalPosition( locator ) ) );

	debugNavigator.setCamera( &camera );

	window.createGLWindow( "tset", 800, 600, 100, 100, 32, false, glWinProc, NULL);
	window.show();


	glEnable( GL_DEPTH_TEST );
	glEnable( GL_NORMALIZE );
	glEnable( GL_LIGHTING );
	glEnable( GL_LIGHT0 );
	GLfloat LightPosition[]={ 0.0f, 0.0f, 2.0f, 1.0f };
	GLfloat LightDiffuse[]=	{ 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat LightSpecular[]={ 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat LightAmbient[]=	{ 0.5f, 0.5f, 0.5f, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, LightSpecular);
	glLightfv(GL_LIGHT0, GL_POSITION,LightPosition);


	// Main message loop:
	while( !g_done ) 
	{
		MSG msg;
		if( PeekMessage( &msg,NULL,0,0,PM_REMOVE) )
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}else
		{
			// draw the scene -----------------------
			glClearColor( .5f, .5f, .5f, 1.0f );
			glClear( GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);

			glViewport( (GLsizei)0.0f, (GLsizei)0.0f, (GLsizei)window.getWidth(), (GLsizei)window.getHeight() );

			glMatrixMode( GL_PROJECTION );
			glLoadMatrixf( camera.getProjectionMatrix().ma );

			glMatrixMode( GL_MODELVIEW );
			glLoadMatrixf( (GLfloat *)camera.getViewMatrix().ma );

			//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE );
			// display the domain
			renderDomain( domain );
			//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL );

			/*
			glDisable( GL_DEPTH_TEST );
			glDisable( GL_LIGHTING );
			locator_global = domain->getElement(20)->getGlobalPosition( locator );
			//math::Vec3f locator_global = locator;
			glBegin( GL_LINES );
			glColor3f(1.0f, 0.0f, 1.0f);
			// draw locator within global coordinate system for a specific element
			glVertex3f( locator_global.x-0.1f, locator_global.y, locator_global.z );glVertex3f( locator_global.x+0.1f, locator_global.y, locator_global.z );
			glVertex3f( locator_global.x, locator_global.y-0.1f, locator_global.z );glVertex3f( locator_global.x, locator_global.y+0.1f, locator_global.z );
			glVertex3f( locator_global.x, locator_global.y, locator_global.z-0.1f );glVertex3f( locator_global.x, locator_global.y, locator_global.z+0.1f );
			glEnd();
			glEnable( GL_LIGHTING );
			glEnable( GL_DEPTH_TEST );

			// draw reference element
			float scale = 0.4f;
			glEnable( GL_SCISSOR_TEST );
			glViewport( (GLsizei)0.0f, (GLsizei)0.0f, (GLsizei)(window.getWidth()*scale), (GLsizei)(window.getHeight()*scale) );
			glScissor( 0, 0, (GLsizei)(window.getWidth()*scale), (GLsizei)(window.getHeight()*scale) );
			glClearColor( .1f, .1f, .1f, 1.0f );
			glClear( GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
			glDisable( GL_SCISSOR_TEST );

			//renderDomain( domain );
			glDisable( GL_LIGHTING );
			glColor3f( 1.0f, 1.0f, 1.0f );
			glLineWidth( 2.0f );
			glBegin( GL_LINES );

			// draw reference element
			glVertex3f( 0.0f, 0.0f, 0.0f );glVertex3f( 1.0f, 0.0f, 0.0f );
			glVertex3f( 1.0f, 0.0f, 0.0f );glVertex3f( 0.0f, 0.0f, 1.0f );
			glVertex3f( 0.0f, 0.0f, 1.0f );glVertex3f( 0.0f, 0.0f, 0.0f );
glColor3f(1.0f, 0.0f, 0.0f);
			glVertex3f( 0.0f, 0.0f, 0.0f );glVertex3f( 0.0f, 1.0f, 0.0f );
			glVertex3f( 0.0f, 1.0f, 0.0f );glVertex3f( 1.0f, 0.0f, 0.0f );
			glVertex3f( 1.0f, 0.0f, 0.0f );glVertex3f( 0.0f, 0.0f, 0.0f );
glColor3f(0.0f, 1.0f, 1.0f);
			// draw locator within reference coordinate system
math::Vec3f locator2 = domain->getElement(20)->getReferencePosition( locator_global );
			glVertex3f( locator2.x-0.1f, locator2.y, locator2.z );glVertex3f( locator2.x+0.1f, locator2.y, locator2.z );
			glVertex3f( locator2.x, locator2.y-0.1f, locator2.z );glVertex3f( locator2.x, locator2.y+0.1f, locator2.z );
			glVertex3f( locator2.x, locator2.y, locator2.z-0.1f );glVertex3f( locator2.x, locator2.y, locator2.z+0.1f );

			glEnd();
			glEnable( GL_LIGHTING );
			*/

			

			// blit screen
			SwapBuffers( window.mhDC );
		}
	};


	return 0;
}



void renderDomain( Domain *domain )
{
	glBegin( GL_TRIANGLES );

	/*
	// just dump the surface triangles for now
	for( std::vector<Domain::Triangle>::iterator it = domain->surface.begin(); it != domain->surface.end(); ++it  )
	{
		Domain::Triangle *tri = &(*it);

		glNormal3fv( tri->normal.v );
		glVertex3fv( domain->getNode( tri->indicees[0] )->position.v );
		glVertex3fv( domain->getNode( tri->indicees[1] )->position.v );
		glVertex3fv( domain->getNode( tri->indicees[2] )->position.v );
	}
	*/

	// render the volume elements (tetrahedrals)
	for( size_t i = 0; i<domain->getElementCount(); ++i )
	//size_t i = 20;
	{
		Element *e = domain->getElement( i );
		math::Vec3f              positions[4];
		math::Vec3f                    center;

		for( int i2 = 0; i2<4; ++i2 )
		{
			positions[i2] = e->getNode( i2 )->getPosition();
			center += positions[i2];
		}
		center /= 4.0f;

		for( int i2 = 0; i2<4; ++i2 )
		{
			positions[i2] -= center;
			positions[i2] *= 0.9f;
			positions[i2] += center;
		}

		glNormal3fv( math::invert( math::normalize( math::crossProduct( positions[1] - positions[2], positions[0] - positions[2] ) ) ).v );
		glVertex3fv( positions[2].v );
		glVertex3fv( positions[1].v );
		glVertex3fv( positions[0].v );

		glNormal3fv( math::invert( math::normalize( math::crossProduct( positions[1] - positions[0], positions[3] - positions[0] ) ) ).v );
		glVertex3fv( positions[0].v );
		glVertex3fv( positions[1].v );
		glVertex3fv( positions[3].v );
		glNormal3fv( math::invert( math::normalize( math::crossProduct( positions[2] - positions[1], positions[3] - positions[1] ) ) ).v );
		glVertex3fv( positions[1].v );
		glVertex3fv( positions[2].v );
		glVertex3fv( positions[3].v );

		glNormal3fv( math::invert( math::normalize( math::crossProduct( positions[3] - positions[0], positions[2] - positions[0] ) ) ).v );
		glVertex3fv( positions[0].v );
		glVertex3fv( positions[3].v );
		glVertex3fv( positions[2].v );
	}

	glEnd();


	glDisable( GL_LIGHTING );
	glPointSize( 10.0f );
	glBegin( GL_POINTS );
	// render all nodes which have boundary conditions
	for( size_t i = 0; i<domain->getNodeCount(); ++i )
	{
		Node *node = domain->getNode(i);

		// get displacements
		DOF *dx = node->getDOF(0);
		DOF *dy = node->getDOF(1);
		DOF *dz = node->getDOF(2);

		if( dx->hasBoundaryCondition() || dy->hasBoundaryCondition() || dz->hasBoundaryCondition()  )
		{
			// draw it
			glColor3f( 1.0f, 0.0f, 0.0f );
			glVertex3fv( (node->getPosition() + math::Vec3f( dx->getSolution(), dy->getSolution(), dz->getSolution() )).v );
		}
	}
	glEnd();
	glEnable( GL_LIGHTING );

}





//
// WinAPI message handler
//
LRESULT CALLBACK  glWinProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
	case WM_CLOSE:								
		{
			PostQuitMessage(0);
			g_done = true;
			return 0;
		}break;
	case WM_KEYDOWN: // a key has been pressed
		switch( wParam )
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			g_done = true;
			break;
		case 0 :
			break;
		case VK_RETURN:
			{
			}break;
		case VK_SPACE:
			{
			}break;
		case VK_UP :
			break;
		case VK_DOWN :
			break;
		case VK_LEFT :
			{
			}break;
		case VK_RIGHT :
			{
			}break;
		case VK_NUMPAD4 :
			break;
		case VK_NUMPAD6 :
			break;
		case VK_NUMPAD5 :
			break;
		case VK_NUMPAD8 :
			break;
		case VK_NUMPAD9 :
			break;
		case VK_NUMPAD3 :
			break;
		case VK_NUMPAD1 :
			break;
		case VK_ADD :
			break;
		case VK_SUBTRACT :
			break;
		case 70 :// KEY : 'f'
			break;
		case 82 :// KEY : 'r'
			break;
		case 68 :// KEY : 'd'
			break;
		case 86 :// KEY : 'v'
			break;
		case 71 :// KEY : 'g'
			break;
		case 49 : // 1 key
			break;
		case 50 : // 2 key
			break;
		case 51 : // 3 key
			break;
		case 52 : // 4 key
			break;
		default:
			return DefWindowProc( hWnd, uMsg, wParam, lParam );
			break;
		}
		break;

	case WM_MOUSEMOVE :
		{
			int xPos = ((int)(short)LOWORD(lParam)); 
			int yPos = ((int)(short)HIWORD(lParam)); 


			// if a mousebutton had been pressed
			if(((wParam == MK_LBUTTON)||(wParam == MK_MBUTTON)||(wParam == MK_RBUTTON))&&(GetKeyState( VK_LMENU )))
			{
				if( wParam == MK_LBUTTON )
				{
					// Alt + LMB => rotation

					//if( Deg2Rad(elevation) > RT_PI )
					//	twist   -=(xPos - lastMousePosX);
					//else
					//	twist   +=(xPos - lastMousePosX);

					debugNavigator.orbitView( (float)(xPos - lastMousePosX),(float) (yPos - lastMousePosY) );
				}else
					if( wParam == MK_RBUTTON )
					{
						// Alt + RMB => move camera along lookat vector
						float x = (float)(xPos - lastMousePosX);
						debugNavigator.zoomView( -x*debugNavigator.getDistance()*0.005f );
					}else
					{// MMBUTTON
						float x = float(xPos - lastMousePosX);
						float y = float(yPos - lastMousePosY);
						debugNavigator.panView( (float)x, (float)-y );
					}
			}

			lastMousePosX = xPos;
			lastMousePosY = yPos;
		}break;

	default:
        return DefWindowProc( hWnd, uMsg, wParam, lParam );   
	}


	return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

