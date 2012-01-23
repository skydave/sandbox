
#pragma once
#include <fbxsdk.h>
#include <ops/Op.h>
#include <ops/Manager.h>
#include <ops/Variant.h>


//
// returns worldtransform matrix of a fbxnode
//
BASE_DECL_SMARTPTR(FBXTransformOp);
class FBXTransformOp : public base::ops::Op
{
public:
	FBXTransformOp( KFbxNode *fbxNode ) : base::ops::Op()
	{
		m_fbxNode = fbxNode;
		m_fbxScene = m_fbxNode->GetScene();
		m_fbxEvaluator = m_fbxScene->GetEvaluator();

		m_outputs.push_back( base::Variant::create() );
	}

	virtual void execute()
	{
		KTime fbxTime;
		fbxTime.SetSecondDouble( base::ops::Manager::context()->time() );
		KFbxXMatrix& worldTransform = m_fbxEvaluator->GetNodeGlobalTransform(m_fbxNode, fbxTime);
		math::Matrix44f m;
		for( int i=0;i<4;++i )
			for( int j=0;j<4;++j )
				m.m[i][j] = worldTransform.Get( i, j );
		/*
		for( int i=0;i<4;++i )
		{
			for( int j=0;j<4;++j )
			{
				std::cout << m.m[i][j] << " ";
			}
			std::cout << std::endl;
		}std::cout << std::endl;
		*/
		base::dynamic_pointer_cast<base::Variant>(m_outputs[0])->m_variant = m;
	}

	static FBXTransformOpPtr create( KFbxNode *fbxNode )
	{
		return FBXTransformOpPtr( new FBXTransformOp( fbxNode  ) );
	}

//private:
	KFbxNode                               *m_fbxNode;
	KFbxScene                             *m_fbxScene;
	KFbxAnimEvaluator                 *m_fbxEvaluator;


};
