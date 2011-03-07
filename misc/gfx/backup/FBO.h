#pragma once

struct Texture;

struct FBO
{
	FBO( int _width, int _height );

	void attach( Texture *texture );
	void finalize();


	void begin();
	void end();

	unsigned int fboId;
	int width, height;
	char numAttachments;
};
