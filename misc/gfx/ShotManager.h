#pragma once
#include "../math/Math.h"
#include "Camera.h"
#include "Clip.h"

struct Shot : Clip
{
	Shot( math::Vec3f pos0, math::Vec3f target0, math::Vec3f pos1, math::Vec3f target1, float _startTime = 0.0f, float _endTime = 1.0f );

	math::Vec3f pos0;
	math::Vec3f target0;
	math::Vec3f pos1;
	math::Vec3f target1;
};


struct ShotManager : ClipCompound
{
	ShotManager( Camera *_cam );
	void addShot( float durationInLocalTime, math::Vec3f pos0, math::Vec3f target0, math::Vec3f pos1, math::Vec3f target1 );

	virtual void update( float parentTime );

	Camera *cam;
};