

#include "ParticleCloud.h"









void ParticleCloud::buildGeometry()
{
	// rebuild sprite geo
	m_spriteGeo->clear();
	m_sortHelpers.clear();

	base::AttributePtr spriteGeo_positions = m_spriteGeo->getAttr( "P" );
	base::AttributePtr spriteGeo_color = m_spriteGeo->getAttr( "Cd" );
	base::AttributePtr spriteGeo_uvs = m_spriteGeo->getAttr( "UV" );


	// for each point
	int numPoints = (int)m_positions.size();
	numPoints = (int)(m_numParticlesScale*numPoints);

	for( int i=0;i<numPoints;++i )
	{
		// build transform matrix from position and normal
		math::Vec3f p, n, tangentu;
		int textureId = m_textureIds[i];

		//p = math::Vec3f(0.0f, 0.0f, 0.0f);
		//n = math::Vec3f(0.0f, 1.0f, 0.0f);
		p = m_positions[i];
		n = m_normals[i];
		tangentu = m_tangentu[i];

		// Positions ==========================
		math::Vec3f right, up, forward;
		forward = n.normalized();
		up = math::Vec3f( 0.0f, 1.0f, 0.0f );

		/*
		right = tangentu.normalized();
		
		up = math::crossProduct( forward, right ).normalized();
		*/

		/*
		if( fabs(math::dotProduct(n, up)) > 0.99  )
		{
			right = math::Vec3f( 1.0f, 0.0f, 0.0f );
			up = math::crossProduct( forward, right );
			right = math::crossProduct( up, forward );
		}else
		{
			right = math::crossProduct( n, up );
			up = math::crossProduct( forward, right );
		}
		*/
		right = math::crossProduct( up, n ).normalized();
		up = math::crossProduct( right, forward ).normalized();

		//n, up
		//forward, right

		//up, n
		//forward, right

		//n, up
		//right, forward

		//up, n
		//right, forward



		math::Matrix44f rotation( right, up, forward );
		math::Matrix44f scale = math::Matrix44f::ScaleMatrix(m_globalScale);

		//float uniformScale = 0.3f;
		float uniformScale = m_billboardTypes[textureId].scale;

		math::Matrix44f xform = scale*rotation*math::Matrix44f::TranslationMatrix(p);
		//math::Matrix44f xform = math::Matrix44f::TranslationMatrix(p);
		//math::Matrix44f xform = math::Matrix44f::Identity();

		/*
		int i0 = spriteGeo_positions->appendElement( p );
		int i1 = spriteGeo_positions->appendElement( p + math::Vec3f(0.0f, 0.01f, 0.0f) );
		int i2 = spriteGeo_positions->appendElement( p + right*0.03f + math::Vec3f(0.0f, 0.01f, 0.0f) );
		int i3 = spriteGeo_positions->appendElement( p + right*0.03f );
		*/
		///*
		math::Vec3f p0, p1, p2, p3;
		p0 = math::transform(math::Vec3f(-uniformScale,-uniformScale,0.0f), xform);
		p1 = math::transform(math::Vec3f(-uniformScale,uniformScale,0.0f), xform);
		p2 = math::transform(math::Vec3f(uniformScale,uniformScale,0.0f), xform);
		p3 = math::transform(math::Vec3f(uniformScale,-uniformScale,0.0f), xform);
		int i0 = spriteGeo_positions->appendElement( p0 );
		int i1 = spriteGeo_positions->appendElement( p1 );
		int i2 = spriteGeo_positions->appendElement( p2 );
		int i3 = spriteGeo_positions->appendElement( p3 );
		//*
		/*
		int i0 = spriteGeo_positions->appendElement( math::transform(math::Vec3f(0.0f,0.0f,0.0f), xform) );
		int i1 = spriteGeo_positions->appendElement( math::transform(math::Vec3f(uniformScale,0.0f,0.0f), xform) );
		int i2 = spriteGeo_positions->appendElement( math::transform(math::Vec3f(uniformScale,0.03f,0.0f), xform) );
		int i3 = spriteGeo_positions->appendElement( math::transform(math::Vec3f(0.0f,0.03f,0.0f), xform) );
		*/


		// UV's =============================
		float uStart, uEnd, vStart, vEnd;
		int numRows, numCols;
		numRows = 4;
		numCols = 4;
		float uWidth = 1.0f/numCols;
		float vWidth = 1.0f/numRows;

		div_t divresult;
		divresult = div (textureId,numCols);


		uStart = 0.0f + divresult.rem*uWidth;
		vStart = 0.0f + divresult.quot*vWidth;

		uEnd = uStart + uWidth;
		vEnd = vStart + vWidth;

		spriteGeo_uvs->appendElement( uStart, vStart );
		spriteGeo_uvs->appendElement( uStart, vEnd );
		spriteGeo_uvs->appendElement( uEnd, vEnd );
		spriteGeo_uvs->appendElement( uEnd, vStart );

		// Colors =============================
		math::Vec3f c0,c1,c2,c3;

		c0 = math::Vec3f(1.0f,1.0f,1.0f);
		c1 = math::Vec3f(1.0f,1.0f,1.0f);
		c2 = math::Vec3f(1.0f,1.0f,1.0f);
		c3 = math::Vec3f(1.0f,1.0f,1.0f);

		float maxY = 0.75;
		float minY = -0.75;
		float rangeY = maxY - minY;

		c0 = math::Vec3f(1.0f,1.0f,1.0f)*((p0.y-minY)/rangeY);
		c1 = math::Vec3f(1.0f,1.0f,1.0f)*((p1.y-minY)/rangeY);
		c2 = math::Vec3f(1.0f,1.0f,1.0f)*((p2.y-minY)/rangeY);
		c3 = math::Vec3f(1.0f,1.0f,1.0f)*((p3.y-minY)/rangeY);


		/*
		spriteGeo_color->appendElement( math::Vec3f(1.0f,1.0f,1.0f) );
		spriteGeo_color->appendElement( math::Vec3f(1.0f,1.0f,1.0f) );
		spriteGeo_color->appendElement( math::Vec3f(1.0f,1.0f,1.0f) );
		spriteGeo_color->appendElement( math::Vec3f(1.0f,1.0f,1.0f) );
		*/

		spriteGeo_color->appendElement( c0 );
		spriteGeo_color->appendElement( c1 );
		spriteGeo_color->appendElement( c2 );
		spriteGeo_color->appendElement( c3 );

		m_spriteGeo->addQuad( i3, i2, i1, i0 );

		// sorthelper
		SortHelper sh;
		sh.indices[0] = i0;
		sh.indices[1] = i1;
		sh.indices[2] = i2;
		sh.indices[3] = i3;
		sh.center = (p0 + p1 + p2 + p3)/4.0f;
		m_sortHelpers.push_back( sh );
	}

	base::apply_normals( m_spriteGeo );
}


void ParticleCloud::sort( base::CameraPtr cam )
{
	// compute distance to camera
	math::Vec3f camPos = cam->m_transform.getTranslation();
	math::Vec3f dir = cam->m_transform.getDir();
	for( std::vector<SortHelper>::iterator it = m_sortHelpers.begin(); it != m_sortHelpers.end(); ++it )
	{
		SortHelper &sh = *it;
		sh.dist2 = -math::dotProduct( sh.center - camPos, dir );
	}

	// sort
	std::sort(m_sortHelpers.begin(), m_sortHelpers.end());
	// update indexbuffer
	int c = 0;
	for( std::vector<SortHelper>::iterator it = m_sortHelpers.begin(); it != m_sortHelpers.end(); ++it )
	{
		SortHelper &sh = *it;
		m_spriteGeo->m_indexBuffer[c++] = sh.indices[0];
		m_spriteGeo->m_indexBuffer[c++] = sh.indices[1];
		m_spriteGeo->m_indexBuffer[c++] = sh.indices[2];
		m_spriteGeo->m_indexBuffer[c++] = sh.indices[3];
	}
	m_spriteGeo->m_indexBufferIsDirty = true;

}