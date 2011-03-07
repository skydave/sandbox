#include "Clip.h"
#include "../math/Math.h"

Clip::Clip( float _startTime, float _endTime ) : startTime(_startTime), endTime(_endTime), localTime(0.0f)
{
}


void Clip::update( float parentTime )
{
	localTime = math::mapValueTo0_1( startTime, endTime, parentTime );
	// todo: handle out of range behaviour
	if( localTime < 0.0f )
		localTime = 0.0f;
	else
	if( localTime > 1.0f )
		localTime = 1.0f;
}

bool Clip::isActive()
{
	return (localTime >= 0.0f) && (localTime < 1.0f);
}



ClipCompound::ClipCompound( float _startTime, float _endTime ) : Clip( _startTime, _endTime )
{
}


void ClipCompound::update( float parentTime )
{
	// first update our local time
	Clip::update(parentTime);
	// now let all child clips update themself
	for(int i=0; i<childClips.size(); ++i)
		((Clip *)childClips.m_data[i])->update( localTime );
}
