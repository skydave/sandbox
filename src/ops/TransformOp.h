
#pragma once

#include <ops/Op.h>
#include <ops/Manager.h>


//
// updates the viewport and projection matrices from wrapped camera
//
BASE_DECL_SMARTPTR(TransformOp);
class TransformOp : public base::ops::Op
{
public:
	TransformOp() : base::ops::Op(), m_transform(math::Matrix44f::Identity())
	{
	}

	virtual void execute()
	{
		// get inputs
		get( "transformMatrix", m_transform );

		for( int i=0;i<4;++i )
		{
			for( int j=0;j<4;++j )
			{
				//std::cout << m_transform.m[i][j] << " ";
			}
			//std::cout << std::endl;
		}

		// get snapshot of curret transformstate
		math::Matrix44f tmp1, tmp2, tmp3, tmp4, tmp5;
		math::Matrix33f tmp6;
		base::ops::Manager::context()->getTransformState( tmp1, tmp2, tmp3, tmp4, tmp5, tmp6 );

		// set modelmatrix
		base::ops::Manager::context()->setModelMatrix( m_transform );

		// execute inputs
		for( OpList::iterator it = m_opList.begin(); it != m_opList.end(); ++it)
			(*it)->execute();

		// restore original transform state
		base::ops::Manager::context()->setTransformState( tmp1, tmp2, tmp3, tmp4, tmp5, tmp6 );
	}

	static TransformOpPtr create()
	{
		return TransformOpPtr( new TransformOp() );
	}

//private:
	math::Matrix44f                  m_transform; 
};
