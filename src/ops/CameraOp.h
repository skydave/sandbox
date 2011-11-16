
#pragma once

#include <ops/Op.h>
#include <ops/Context.h>
#include <gfx/Camera.h>


//
// updates the viewport and projection matrices from wrapped camera
//
BASE_DECL_SMARTPTR(CameraOp);
class CameraOp : public base::ops::Op
{
public:
	CameraOp() : base::ops::Op(), m_camera(base::CameraPtr(new base::Camera()))
	{
	}

	virtual void execute()
	{
		// get inputs and set it on the camera
		get( "projectionMatrix", m_camera->m_projectionMatrix );
		get( "viewMatrix", m_camera->m_viewMatrix );
		get( "transformMatrix", m_camera->m_transform );

		// update camera
		m_camera->update();

		// push camera.view
		base::CameraPtr parentCamera = base::ops::Context::camera();
		base::ops::Context::setCamera( m_camera );

		// execute inputs
		for( OpList::iterator it = m_opList.begin(); it != m_opList.end(); ++it)
			(*it)->execute();

		// pop camera view
		base::ops::Context::setCamera(parentCamera);
	}

	static CameraOpPtr create()
	{
		return CameraOpPtr( new CameraOp() );
	}

private:
	base::CameraPtr             m_camera;
};
