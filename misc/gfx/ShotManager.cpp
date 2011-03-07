#include "ShotManager.h"




Shot::Shot( math::Vec3f _pos0, math::Vec3f _target0, math::Vec3f _pos1, math::Vec3f _target1, float _startTime, float _endTime ) : Clip( _startTime, _endTime ), pos0(_pos0), pos1(_pos1), target0(_target0), target1(_target1)
{
}



ShotManager::ShotManager( Camera *_cam ) : cam(_cam), ClipCompound( 0.0f, 1.0f )
{
}

void ShotManager::addShot( float durationInLocalTime, math::Vec3f pos0, math::Vec3f target0, math::Vec3f pos1, math::Vec3f target1 )
{
	float endTime = 0.0f;
	if( childClips.size() > 0 )
		endTime = ((Shot *)childClips.m_data[childClips.size() - 1])->endTime;
	childClips.push_back( new Shot( pos0, target0, pos1, target1, endTime, endTime + durationInLocalTime ) );
}


void ShotManager::update( float parentTime )
{
	// update local time
	ClipCompound::update(parentTime);

	Shot *s = 0;
	// find current shot
	int i = 0;
	do
	{
		if( i < childClips.size() )
			s = ((Shot*)childClips.m_data[i]);
		else
			s = 0;
		++i;
	}while( s && (s->endTime < localTime)  );



	if(s && s->endTime >= localTime)
	{
		// interpolate to result (remember localTime goes from 0-1)
		float interpcompl = math::smoothstep(s->localTime);
		float interp = 1.0f - interpcompl;
		cam->transform = math::createLookAtMatrix( interp*s->pos0 + interpcompl*s->pos1, interp*s->target0 + interpcompl*s->target1, math::Vec3f(0.0f, 1.0f, 0.0f) );
		// update camera
		cam->update();
	}
}