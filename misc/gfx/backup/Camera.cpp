#include "Camera.h"
#include "Attribute.h"


Camera::Camera()
{
	m_znear = 0.1f;
	m_zfar  = 1000.0f;
	m_fov   = 45.0f;
	m_aspectRatio = 1.333f;


	projectionMatrixAttribute = new Attribute(16);
	projectionMatrixAttribute->appendElement( &((math::Matrix44f::Identity()).m[0][0]) );

	viewMatrixAttribute = new Attribute(16);
	viewMatrixAttribute->appendElement( &((math::Matrix44f::Identity()).m[0][0]) );

	viewProjectionMatrixAttribute = new Attribute(16);
	viewProjectionMatrixAttribute->appendElement( &((math::Matrix44f::Identity()).m[0][0]) );
}

void Camera::update( void )
{
	//calculate width and height at 1 unit in front of the camera
	float height = 2 * tanf( float(m_fov/2) );
	float width = height * m_aspectRatio;


	//create projection matrix
	float sc = ( m_zfar + m_znear ) / ( m_zfar - m_znear );
	float of = 2 * m_zfar * m_znear / ( m_zfar - m_znear );
	projectionMatrix.ma[ 0] = 2/width;
	projectionMatrix.ma[ 1] = 0;
	projectionMatrix.ma[ 2] = 0;
	projectionMatrix.ma[ 3] = 0;
	projectionMatrix.ma[ 4] = 0;
	projectionMatrix.ma[ 5] = 2/height;
	projectionMatrix.ma[ 6] = 0;
	projectionMatrix.ma[ 7] = 0;
	projectionMatrix.ma[ 8] = 0;
	projectionMatrix.ma[ 9] = 0;
	projectionMatrix.ma[10] = -sc;
	projectionMatrix.ma[11] = -1;
	projectionMatrix.ma[12] = 0;
	projectionMatrix.ma[13] = 0;
	projectionMatrix.ma[14] = -of;
	projectionMatrix.ma[15] = 0;

	// create view matrix
	viewMatrix = transform;
	// speedup inverse computation by transposing the rotational part of the matrix
	//viewMatrix.invert();
	math::Vec3f translation = viewMatrix.getTranslation();
	math::Matrix44f rotation = viewMatrix.getOrientation();
	rotation.transpose();
	viewMatrix = math::Matrix44f::TranslationMatrix( -translation ) * rotation;


	projectionMatrixAttribute->setElement( 0, &projectionMatrix.ma[0] );
	viewMatrixAttribute->setElement( 0, &viewMatrix.ma[0] );
	viewProjectionMatrixAttribute->setElement( 0, &(viewMatrix*projectionMatrix).ma[0] );
}
