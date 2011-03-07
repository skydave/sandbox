#pragma once
#include "../sys/msys.h"
#include "../std/vector.h"

struct Attribute;

struct Shader
{
	Shader();
	void init(char *vsSrc, int &vsSrcSize, char *psSrc, int &psSrcSize);
	void attach(int shaderType, char *src, int &srcSize);
	void finalize();
	void use();
	bool isOk();

	// set uniforms
	// set vertex attributes

	// list of active attributes (which are required by this shader)
	vector<int> m_activeAttributes;
	// list of active uniforms (which are required by this shader)
	vector<int> m_activeUniforms;
	vector<void *> m_activeUniformNames; // list of uniform names (unfortunately thats the only way to connect unforms of geometry with the shader) again: hashmap would be nice



	//
	// local uniform management
	//
	void setUniform( const char *name, Attribute *uniform );
	Attribute               *getUniform( int uniformIndex );
	vector<void *>                               m_uniforms; // list of uniforms
	vector<const char *>                     m_uniformNames; // list of uniform names

	//
	// global uniform management
	// global uniforms are attributes which will be used by all shaders
	// examples are: mvp, permutationTable, etc.
	//
	static void setGlobalUniform( const char *name, Attribute *uniform );
	static vector<void *> g_globalUniforms; // list of uniforms
	static vector<const char *> g_globalUniformNames; // list of uniform names

	GLhandleARB m_glProgram;
	bool             m_isOk;
	char              *text; // used for debug log
};




