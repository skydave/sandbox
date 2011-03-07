#pragma once
#include "../std/vector.h"

struct Clip
{
	Clip( float _startTime = 0.0f, float _endTime = 1.0f );


	virtual void update( float parentTime );
	bool isActive();

	// time range information
	float startTime; // startTime relative to parentTime
	float endTime; // endTime relative to parentTime
	float localTime; // 0-1
};

struct ClipCompound : Clip
{
	ClipCompound( float _startTime, float _endTime );

	virtual void update( float parentTime );

	vector<void *> childClips;
};