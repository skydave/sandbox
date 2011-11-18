#pragma once

#include <ops/Op.h>


#include <ops/Manager.h>




//
// updates the context time
//
BASE_DECL_SMARTPTR(TimeOp);
class TimeOp : public base::ops::Op
{
public:
	TimeOp() : base::ops::Op(), m_inTimeStart(0.0f), m_inTimeEnd(1.0f), m_outTimeStart(0.0f), m_outTimeEnd(1.0f)
	{
	}

	virtual void execute()
	{
		float parentTime = base::ops::Manager::context()->time();
		// normalized CurrentTime
		float currentTime = (parentTime - m_inTimeStart)/(m_inTimeEnd - m_inTimeStart);
		// map to outputtimerange
		currentTime = currentTime*(m_outTimeEnd - m_outTimeStart ) + m_outTimeStart;
		// push new time on context
		base::ops::Manager::context()->setTime( currentTime );
		// execute inputs
		for( OpList::iterator it = m_opList.begin(); it != m_opList.end(); ++it)
			(*it)->execute();
		// pop time
		base::ops::Manager::context()->setTime( parentTime );
	}

	static TimeOpPtr create()
	{
		return TimeOpPtr( new TimeOp() );
	}

//private:
	float        m_inTimeStart, m_inTimeEnd;
	float      m_outTimeStart, m_outTimeEnd;
};
