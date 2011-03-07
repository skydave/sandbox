#pragma once
#include "../sys/msys.h"
#include "../std/vector.h"
#include "../math/Math.h"
#include "Attribute.h"
#include "FCurve.h"

struct Skeleton
{
	struct Bone
	{
		Bone();


		float length;
		math::Vec3f  localEulerRotation;
		math::Matrix44f globalTransform; // not neccessarily needed - >will be stored in attribute
		math::Matrix44f vertexTransform; // not neccessarily needed - >will be stored in attribute
		float *finalData;

		vector<void *> childBones;
		Bone *parent;

		math::Matrix44f   bindPoseInv;
		int level;
	};

	Skeleton();
	Skeleton( unsigned char *skel, unsigned char *_pose, unsigned char *anims = 0 );

	void update( const float &time, bool updateAnim = true );
	void updateBone( const float &time, Bone *bone, const math::Matrix44f &parentTransform, float parentLength );
	void draw();
	void draw( Bone *bone );
	Bone *getBone( int index );


	// bone matrices -> uniform attribute which will be added to a mesh and updated here by the skeleton
	// 
	Bone                             *m_root; // root bone
	vector<void *>                   m_bones; // we track a flatted list of all bones for easier access
	vector<void *> m_localEulerRotationAnims; // contains fcurves for all bones local rotation values x,y,z
	Attribute         *m_boneMatricesUniform; // will be attach to geometry as uniform
};


