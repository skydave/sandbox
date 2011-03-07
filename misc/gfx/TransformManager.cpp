#include "TransformManager.h"

#include "Attribute.h"
#include "Shader.h"



TransformManager::TransformManager()
{
	m_modelMatrix = math::Matrix44f::Identity();
	m_viewMatrix = math::Matrix44f::Identity();
	m_projectionMatrix = math::Matrix44f::Identity();
	m_viewInverseMatrix = math::Matrix44f::Identity();

	// view projection matrix
	m_mvpmAttr = new Attribute( 16 );
	m_mvpmAttr->appendElement( math::Matrix44f::Identity().ma );
	Shader::setGlobalUniform( "mvpm", m_mvpmAttr );

	// model view matrix inverse transpose
	m_mvminvtAttr = new Attribute( 9 );
	m_mvminvtAttr->appendElement( math::Matrix44f::Identity().ma );
	Shader::setGlobalUniform( "mvminvt", m_mvminvtAttr );


	//Shader::setGlobalUniform( "vm", cam->viewMatrixAttribute ); // view matrix

	// view matrix inverse (camera transform)
	m_vminvAttr = new Attribute( 16 );
	m_vminvAttr->appendElement( math::Matrix44f::Identity().ma );
	Shader::setGlobalUniform( "vminv", m_vminvAttr );

	//Shader::setGlobalUniform( "vminv", cam->transformMatrixAttribute ); // inverse view matrix (camera world transform)
	//Shader::setGlobalUniform( "mm", mmAttr ); // model matrix (object to world transform)





	/*
	projectionMatrixAttribute = new Attribute(16);
	projectionMatrixAttribute->appendElement( &((math::Matrix44f::Identity()).m[0][0]) );

	viewMatrixAttribute = new Attribute(16);
	viewMatrixAttribute->appendElement( &((math::Matrix44f::Identity()).m[0][0]) );


	viewProjectionMatrixAttribute = new Attribute(16);
	viewProjectionMatrixAttribute->appendElement( &((math::Matrix44f::Identity()).m[0][0]) );

	transformMatrixAttribute = new Attribute(16);
	transformMatrixAttribute->appendElement( &((math::Matrix44f::Identity()).m[0][0]) );



	camera::update

	projectionMatrixAttribute->setElement( 0, &projectionMatrix.ma[0] );
	viewMatrixAttribute->setElement( 0, &viewMatrix.ma[0] );
	viewProjectionMatrixAttribute->setElement( 0, &(viewMatrix*projectionMatrix).ma[0] );
	transformMatrixAttribute->setElement( 0, transform.ma );
	*/
}



void TransformManager::pushCamera( const math::Matrix44f &view, const math::Matrix44f &viewInv, const math::Matrix44f &proj )
{
	m_viewMatrix = view;
	m_viewInverseMatrix = viewInv;
	//m_viewInverseMatrix.transpose();
	m_projectionMatrix = proj;

	// update camera transform
	m_vminvAttr->setElement( 0, m_viewInverseMatrix.ma );

	// update model view projection
	updateModelViewProjection();
	updateModelViewMatrixInverseTranspose();
}


void TransformManager::popCamera()
{
	// todo
}






void TransformManager::updateModelViewProjection()
{
	math::Matrix44f m = m_modelMatrix * m_viewMatrix * m_projectionMatrix;
	m_mvpmAttr->setElement( 0, m.ma );
}

void TransformManager::updateModelViewMatrixInverseTranspose()
{
	float m33f[9]; // result which will be uploaded

	math::Matrix44f m = m_modelMatrix * m_viewMatrix;
	m.invert();
	m.transpose();

	// we do the transpose when we extract the orientation from 44f matrix
	m33f[0] = m.m[0][0];
	m33f[1] = m.m[0][1];
	m33f[2] = m.m[0][2];
	m33f[3] = m.m[1][0];
	m33f[4] = m.m[1][1];
	m33f[5] = m.m[1][2];
	m33f[6] = m.m[2][0];
	m33f[7] = m.m[2][1];
	m33f[8] = m.m[2][2];

	m_mvminvtAttr->setElement( 0, m33f );
}
