#pragma once
#include "../math/Math.h"

struct Attribute;


struct TransformManager
{

	TransformManager();


	void pushCamera( const math::Matrix44f &view, const math::Matrix44f &viewinv, const math::Matrix44f &proj );
	void popCamera();


	//void pushModelMatrix( math::Matrix44f modelMatrix = math::Matrix44f::Identity() );
	//void popModelMatrix();

	void updateModelViewProjection();
	void updateModelViewMatrixInverseTranspose();



	math::Matrix44f m_modelMatrix;
	math::Matrix44f m_viewMatrix;
	math::Matrix44f m_viewInverseMatrix;
	math::Matrix44f m_projectionMatrix;


	Attribute *m_mvpmAttr; // model view projection matrix
	Attribute *m_vminvAttr; // view matrix inverse
	Attribute *m_mvminvtAttr; // model view matrix inverse transpose (model view matrix without scaling/shearing) used to transform vectors



	//Attribute *projectionMatrixAttribute;
	//Attribute *viewMatrixAttribute;
	//Attribute *viewProjectionMatrixAttribute;
	//Attribute *transformMatrixAttribute;

};
