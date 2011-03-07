#pragma once
#include "../sys/msys.h"
#include "../math/Math.h"

struct Attribute;

struct Camera
{
	Camera();










	virtual                             void update( void ); ///< will compute the cameras projection matrix

	math::Matrix44f                               transform; ///< the matrix which transforms the camera from local into world space - the inverse of this matrix is the view transform

	float                                           m_znear;
	float                                            m_zfar;
	float                                             m_fov;
	float                                     m_aspectRatio;

	math::Matrix44f                        projectionMatrix;
	math::Matrix44f                              viewMatrix;

	Attribute *projectionMatrixAttribute;
	Attribute *viewMatrixAttribute;
	Attribute *viewProjectionMatrixAttribute;
};
