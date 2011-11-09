#pragma once

#include <ops/Op.h>


#include <ops/Context.h>




//
// updates the context time
//
BASE_DECL_SMARTPTR(TimeOp);
class TimeOp : public base::ops::Op
{
public:
	TimeOp() : base::ops::Op(), m_startTime(0.0f), m_endTime(0.0f)
	{
	}

	virtual void execute()
	{
		float parentTime = base::ops::Context::time();
		// find current time
		float currentTime = (parentTime - m_startTime)/(m_endTime - m_startTime);
		// push new time on context
		base::ops::Context::setTime( currentTime );
		// execute inputs
		for( OpList::iterator it = m_opList.begin(); it != m_opList.end(); ++it)
			(*it)->execute();
		// pop time
		base::ops::Context::setTime( parentTime );
	}

	static TimeOpPtr create()
	{
		return TimeOpPtr( new TimeOp() );
	}

private:
	float                   m_startTime;
	float                     m_endTime;
};
