#include "Skeleton.h"

// begin test
unsigned char testCurve[] = { 0x03, 0x00, 0x00, 0x00, 0x00, 0x55, 0x55, 0x05, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x6A, 0x20, 0x40, 0x42, 0xCE, 0x88, 0x08, 0x43, 0x6A, 0x20, 0x40, 0x42 };
// end test data


// begin testSkel1
unsigned char testSkel11[] = {
0x02, 0x00, 0x00, 0x44, 0x02, 0x5C, 0x40, 0x58, 0xBA, 0x6B,
0x40, 0x03, 0x00, 0x00, 0x00, 0x00, 0x55, 0x55, 0x05, 0x3F,
0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
0x00, 0x55, 0x55, 0x05, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x55, 0x55, 0x05, 0x3F,
0x00, 0x00, 0x80, 0x3F, 0x96, 0xDF, 0x27, 0x42, 0x38, 0x23,
0x3A, 0xC2, 0x96, 0xDF, 0x27, 0x42, 0x05, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0xC0, 0x3D, 0x00, 0x00, 0x00, 0x3F, 0xAB,
0xAA, 0x1A, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0xC0, 0x3D, 0x00, 0x00, 0x00, 0x3F,
0xAB, 0xAA, 0x1A, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x3D, 0x00, 0x00, 0x00,
0x3F, 0xAB, 0xAA, 0x1A, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00,
0x00, 0x00, 0x80, 0x76, 0x53, 0x23, 0x41, 0x92, 0x3C, 0xFF,
0x40, 0x4B, 0xCA, 0xA7, 0xC1, 0x00, 0x00, 0x00, 0x80 };
// end testSkel1 data

// begin testSkel1
unsigned char testSkel2[] = {
0x03, 0x00, 0x00, 0x00, 0x00, 0x55, 0x55, 0x05, 0x3F, 0x00,
0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,
0x55, 0x55, 0x05, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x03, 0x00, 0x00, 0x00, 0x00, 0x55, 0x55, 0x05, 0x3F, 0x00,
0x00, 0x80, 0x3F, 0x6A, 0x20, 0x40, 0x42, 0xCE, 0x88, 0x08,
0x43, 0x6A, 0x20, 0x40, 0x42 };
// end testSkel1 data

Skeleton::Bone::Bone()
{
	parent = 0;
}


Skeleton::Skeleton()
{
	m_root = new Bone();
}

Skeleton::Skeleton( unsigned char *skel, unsigned char *_pose, unsigned char *anims )
{
	//unsigned char *data = testSkel11;



	int numBones = skel[0];
	unsigned char *parentIndices = &skel[1];
	float *lengths = (float *)&skel[1+numBones*sizeof(char)];

	float *pose = (float *)_pose;
	unsigned char *rotAnims = anims;

	for(int i=0; i<numBones; ++i )
	{
		Bone *b = new Bone();
		m_bones.push_back( b );
		if( i>0 )
		{
			((Bone*)m_bones.m_data[parentIndices[i]])->childBones.push_back( b );
			b->parent = (Bone*)m_bones.m_data[parentIndices[i]];
		}
		b->length = lengths[i];

		// apply pose
		b->localEulerRotation.x = math::degToRad(pose[i]);
		b->localEulerRotation.y = math::degToRad(pose[numBones+i]);
		b->localEulerRotation.z = math::degToRad(pose[numBones*2+i]);

		//printf( "bone %i %f   %f   %f\n", i, b->localEulerRotation.x, b->localEulerRotation.y, b->localEulerRotation.z );
		if( rotAnims )
		{
			// read animations of rotations
			// the tricky bit: we dont know how much space each anim uses so we
			// have to find out after loading it
			FCurve *rotAnimX = new FCurve( rotAnims, pose[i] );
			rotAnims += 1 + rotAnimX->m_numKeys*2*sizeof(float);
			FCurve *rotAnimY = new FCurve( rotAnims, pose[numBones+i] );
			rotAnims += 1 + rotAnimY->m_numKeys*2*sizeof(float);
			FCurve *rotAnimZ = new FCurve( rotAnims, pose[numBones*2+i] );
			rotAnims += 1 + rotAnimZ->m_numKeys*2*sizeof(float);

			m_localEulerRotationAnims.push_back( rotAnimX );
			m_localEulerRotationAnims.push_back( rotAnimY );
			m_localEulerRotationAnims.push_back( rotAnimZ );
		}
	}

	m_root = (Bone*)m_bones.m_data[0];


	// create the uniform which will be attached to geometry which has to be driven by skin deformation
	m_boneMatricesUniform = new Attribute( 16 );
	// just fill up the list with dummy entries (will be overwritten in update)
	for( int j=0; j<m_bones.size(); ++j )
		m_boneMatricesUniform->appendElement( &math::Matrix44f::Identity() );

	// update skeleton in refpose (updates global transform)
	// TODO: replace with apply pose
	update( 0.001f, false );

	// store refpose transforms
	for(int i=0; i<numBones; ++i )
	{
		math::Matrix44f bindPoseInv = getBone(i)->globalTransform;
		bindPoseInv.invert();
		getBone(i)->bindPoseInv = bindPoseInv;
	}

}

void Skeleton::update( const float &time, bool updateAnim)
{
	if( updateAnim )
	{
		// update all bones local values (rotation fcurves)
		int c = 0;
		for(int i=0; i<m_bones.size(); ++i )
		{
			//float rx = getBone(i)->localEulerRotation.x;
			//float ry = getBone(i)->localEulerRotation.y;
			//float rz = getBone(i)->localEulerRotation.z;

			//if( ((FCurve*)m_localEulerRotationAnims.m_data[c])->m_numKeys > 0 )
			//	rx = math::degToRad(((FCurve*)m_localEulerRotationAnims.m_data[c])->eval(time));
			//if( ((FCurve*)m_localEulerRotationAnims.m_data[c+1])->m_numKeys > 0 )
			//	ry = math::degToRad(((FCurve*)m_localEulerRotationAnims.m_data[c+1])->eval(time));
			//if( ((FCurve*)m_localEulerRotationAnims.m_data[c+2])->m_numKeys > 0 )
			//	rz = math::degToRad(((FCurve*)m_localEulerRotationAnims.m_data[c+2])->eval(time));

			getBone(i)->localEulerRotation = math::Vec3f( math::degToRad(((FCurve*)m_localEulerRotationAnims.m_data[c])->eval(time)), math::degToRad(((FCurve *)m_localEulerRotationAnims.m_data[c+1])->eval(time)), math::degToRad(((FCurve*)m_localEulerRotationAnims.m_data[c+2])->eval(time)) );
			//getBone(i)->localEulerRotation = math::Vec3f( rx, ry, rz );
			c+=3;
		}
	}
	

	// update global transforms (depends on hierarchy)
	//updateBone(time, m_root, math::Matrix44f::Identity(), 0.0f);
	updateBone(time, m_root, math::Matrix44f::ScaleMatrix(0.1f,0.1f,0.1f), 0.0f);

	// update uniform which goes via geometry into the shader
	for( int j=0; j<m_bones.size(); ++j )
		m_boneMatricesUniform->setElement( j, &getBone(j)->vertexTransform );
}

//
//
//
void Skeleton::updateBone( const float &time, Bone *bone, const math::Matrix44f &parentTransform, float parentLength )
{
	// do global transform by concatenating local with parent transform
	// TODO: decide order of rotation! X/Y/Z or Z/Y/X
	bone->globalTransform = math::Matrix44f::RotationMatrixX(bone->localEulerRotation.x)*math::Matrix44f::RotationMatrixY(bone->localEulerRotation.y)*math::Matrix44f::RotationMatrixZ(bone->localEulerRotation.z)*math::Matrix44f::TranslationMatrix(0.0f, parentLength, 0.0f)*parentTransform; // works

	// the matrix which will be used to transform vertices
	bone->vertexTransform = bone->bindPoseInv*math::Matrix44f::RotationMatrixX(bone->localEulerRotation.x)*math::Matrix44f::RotationMatrixY(bone->localEulerRotation.y)*math::Matrix44f::RotationMatrixZ(bone->localEulerRotation.z)*math::Matrix44f::TranslationMatrix(0.0f, parentLength, 0.0f)*parentTransform;
	//bone->vertexTransform = math::Matrix44f::RotationMatrixZ(bone->localEulerRotation.z)*math::Matrix44f::RotationMatrixY(bone->localEulerRotation.y)*bone->bindPoseInv*math::Matrix44f::RotationMatrixX(bone->localEulerRotation.x)*math::Matrix44f::TranslationMatrix(0.0f, parentLength, 0.0f)*parentTransform;

	// now update all childbones
	for( int i=0; i<bone->childBones.size();++i )
		updateBone( time, (Bone *)bone->childBones.m_data[i], bone->globalTransform, bone->length);
}


void Skeleton::draw( Bone *bone )
{
	// iterate over all bones and draw it
	glPushMatrix();
	glMultMatrixf( &bone->globalTransform.m[0][0] );

	glBegin( GL_LINE_STRIP );
	{
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f( 0.0f, 0.0f, 0.0f);
		glVertex3f(-0.2f, 0.2f,-0.2f);
		glVertex3f( 0.2f, 0.2f,-0.2f);
		glVertex3f( 0.0f, bone->length, 0.0f); // Bone length = 3.0f
		glVertex3f(-0.2f, 0.2f,-0.2f);
		glVertex3f(-0.2f, 0.2f, 0.2f);
		glVertex3f( 0.0f, 0.0f, 0.0f);
		glVertex3f( 0.2f, 0.2f,-0.2f);
		glVertex3f( 0.2f, 0.2f, 0.2f);
		glVertex3f( 0.0f, 0.0f, 0.0f);
		glVertex3f(-0.2f, 0.2f, 0.2f);
		glVertex3f( 0.0f, bone->length, 0.0f); // Bone length = 3.0f
		glVertex3f( 0.2f, 0.2f, 0.2f);
		glVertex3f(-0.2f, 0.2f, 0.2f);
	}
	glEnd();
	glPopMatrix();

	for( int i=0; i<bone->childBones.size();++i )
		draw( (Bone *)bone->childBones.m_data[i]);
}

void Skeleton::draw()
{
	draw( m_root );
}

Skeleton::Bone *Skeleton::getBone( int index )
{
	return (Bone*)m_bones.m_data[index];
}
