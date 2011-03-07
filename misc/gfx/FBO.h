#pragma once

struct Texture;

struct FBO
{
	FBO( int _width, int _height );

	void setOutputs( Texture *out0, Texture *out1 = 0, Texture *out2 = 0, Texture *out3 = 0 );
	void setSize( int newXres, int newYres );
	void finalize();


	void begin( bool clear = true );
	void end();

	unsigned int fboId;
	int width, height;
	char numAttachments;
	unsigned int m_depthbufferId;
};
