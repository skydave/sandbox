//
//
//
#pragma once

#include "../TOP.h"




/*
// abstract base class
struct RenderFunc
{
	//virtual void operator()( const float &time)=0;  // call using operator
	virtual void call( const float &time)=0;  // call using operator
};


// derived template class
template <typename TClass> struct TRenderFunc : RenderFunc
{
	void (TClass::*func)(const float&);   // pointer to member function
	TClass* obj;                  // pointer to object

	// constructor - takes pointer to an object and pointer to a member and stores
	// them in two private variables
	TRenderFunc( TClass* _obj, void(TClass::*_func)(const float &)) : obj(_obj), func(_func)
	{
	};

	// override operator "()"
	//virtual void operator()(const float &time)
	virtual void call(const float &time)
	{
		(*obj.*func)(time);
	};
};
*/











//
//
//
struct RenderTOP : TOP
{
	RenderTOP();

	//void setInputs( RenderFunc *_renderCallback, int xres = 512, int yres = 512 );
	void setInputs( int xres = 512, int yres = 512 );

	void begin();
	//void render( float time = 0.0f );
	void end();

	//RenderFunc *renderCallback;
};
