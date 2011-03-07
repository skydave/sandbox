#include "Shader.h"
#include "Attribute.h"

#include "../glsl/shaders.h"



Shader::Shader() : m_isOk(false)
{
	text =(char*) mmalloc( 1000 );
}

// requires gfx and extensions to be initialized already
void Shader::init(char *vsSrc, int &vsSrcSize, char *psSrc, int &psSrcSize)
{
	GLsizei siz = 0;
	m_glProgram = oglCreateProgram();

	printf( "Shader ===================================\n" );


	//// vertex shader ------
	attach(GL_VERTEX_SHADER_ARB, vsSrc, vsSrcSize);

	//// pixel shader ------
	attach(GL_FRAGMENT_SHADER_ARB, psSrc, psSrcSize);



}


void Shader::attach(int shaderType, char *src, int &srcSize)
{
	GLhandleARB s = oglCreateShader(shaderType);
	char* vsSrcList[1];
	vsSrcList[0] = src;
	oglShaderSource(s, 1, (const GLchar **)&vsSrcList, &srcSize);
	oglCompileShader(s);

	oglGetInfoLog(s, 1000, 0, text);
	printf( "ShaderInfoLog: %s\n", text );

	oglAttachShader(m_glProgram, s);
}

void Shader::finalize()
{
	//
	// bind attribute locations
	//
	oglBindAttribLocation(m_glProgram, 0, "P");
	oglBindAttribLocation(m_glProgram, 1, "N");
	oglBindAttribLocation(m_glProgram, 2, "Cd");
	oglBindAttribLocation(m_glProgram, 3, "W");
	oglBindAttribLocation(m_glProgram, 4, "CMT");
	oglBindAttribLocation(m_glProgram, 5, "BW");
	oglBindAttribLocation(m_glProgram, 6, "BI");
	oglBindAttribLocation(m_glProgram, 7, "UV");


	//
	// program linking --------
	//
	oglLinkProgram(m_glProgram);
	oglGetInfoLog(m_glProgram, 1000, 0, text);
	printf( "program link:%s\n", text );



	// extract active attributes info
	int numActiveAttributes = 0;
	oglGetProgramiv(m_glProgram, GL_ACTIVE_ATTRIBUTES, &numActiveAttributes);
	printf( "\nnumber of active attributes: %i\n", numActiveAttributes );
	for( int i=0;i<numActiveAttributes; ++i )
	{
		char name[1000];
		int length;
		int size;
		unsigned int type;
		oglGetActiveAttrib( m_glProgram, i, 1000, &length, &size, &type, name );
		int index = oglGetAttribLocation(m_glProgram, name);
		printf( "active attributes: %s at location %i\n", name, index );
		m_activeAttributes.push_back(index);
	}

	// extract active uniforms info
	int numActiveUniforms = 0;
	oglGetProgramiv(m_glProgram, GL_ACTIVE_UNIFORMS, &numActiveUniforms);
	printf( "\nnumber of active uniforms: %i\n", numActiveUniforms );
	for( int i=0;i<numActiveUniforms; ++i )
	{
		char *name = (char *)mmalloc( 100*sizeof(char) );
		int length;
		int size;
		unsigned int type;
		oglGetActiveUniform( m_glProgram, i, 1000, &length, &size, &type, name );
		int index = oglGetUniformLocation(m_glProgram, name);
		printf( "active uniform: %s at location %i\n", name, index );

		// index==-1 means uniform is a built in uniform and we dont touch it
		if( index != -1 )
		{
			m_activeUniforms.push_back(index);

			// fucking ati laptop cards puts an [i] at the end of array uniforms. have
			// to remove that to remain compatible with the other shit
			int l = msys_strlen( name );
			if( (name[l-3] == '[') && (name[l-2] == '0') && (name[l-1] == ']') )
				name[l-3] = '\0';
			// store the name
			m_activeUniformNames.push_back( name );
		}
	}
	m_isOk = true;
}





void Shader::use()
{
	oglUseProgram(m_glProgram);

	// iterate all active uniforms
	for( int i=0; i<m_activeUniforms.size(); ++i )
	{
		// iterate over all global uniforms and bind it if we found one
		for( int j=0; j<g_globalUniforms.size(); ++j )
		{
			char *t1 = (char *)m_activeUniformNames.m_data[i];
			char *t2 = (char *)g_globalUniformNames.m_data[j];
			if( !strcmp( (char *)m_activeUniformNames.m_data[i], g_globalUniformNames.m_data[j] ) )
			{
				((Attribute*)(g_globalUniforms.m_data[j]))->bindAsUniform( m_activeUniforms.m_data[i] );
				break;
			}
		}
		// now iterate over all local uniforms and bind the ones we found
		for( int j=0; j<m_uniforms.size(); ++j )
		{
			char *t1 = (char *)m_activeUniformNames.m_data[i];
			const char *t2 = m_uniformNames.m_data[j];
			if( !strcmp( (char *)m_activeUniformNames.m_data[i], m_uniformNames.m_data[j] ) )
			{
				((Attribute*)(m_uniforms.m_data[j]))->bindAsUniform( m_activeUniforms.m_data[i] );
				break;
			}
		}
	}
}

bool Shader::isOk()
{
	return m_isOk;
}



int atexit ( void ( * function ) (void) )
{
	return 0;
}


vector<void *> Shader::g_globalUniforms; // list of uniforms
vector<const char *> Shader::g_globalUniformNames; // list of uniform names

void Shader::setGlobalUniform( const char *name, Attribute *uniform )
{
	g_globalUniformNames.push_back( name );
	g_globalUniforms.push_back( uniform );
}



Attribute *Shader::getUniform( int uniformIndex )
{
	return (Attribute *)m_uniforms.m_data[uniformIndex];
}

void Shader::setUniform( const char *name, Attribute *uniform )
{
	// for each registered uniform
	for( int i=0; i<m_uniformNames.size(); ++i )
	{
		// if uniform already has been registered under that name
		if( !strcmp( (char *)m_uniformNames.m_data[i], name ) )
		{
			// just change the reference
			m_uniforms.m_data[i] = uniform;
			// done
			return;
		}
	}

	// no uniform existed add it
	m_uniformNames.push_back( name );
	m_uniforms.push_back( uniform );
}
