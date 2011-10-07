//============================================================================
//
//
//
//============================================================================



#include <QtGui>
#include <QApplication>

#include <stdio.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

#include <ui/GLViewer.h>
#include <gltools/gl.h>
#include <gltools/misc.h>
#include <util/StringManip.h>
#include <util/Path.h>
#include <gfx/Geometry.h>
#include <gfx/ObjIO.h>
#include <gfx/Shader.h>
#include <gfx/Texture.h>
#include <gfx/Image.h>
#include <gfx/Context.h>
#include <gfx/FCurve.h>
#include <gfx/glsl/common.h>
#include <gfx/FBO.h>

#include <ops/ops.h>

#include "composer/widgets/CurveEditor/CurveEditor.h"
#include "composer/widgets/Trackball/Trackball.h"
#include "composer/widgets/GLViewer/GLViewer.h"

#include "vec3.h"
#include "mat4.h"
//#include "tiffio.h"

#include "Main.h"

composer::widgets::GLViewer *glviewer;


base::ContextPtr context;

base::GeometryPtr geo;
/*
base::Texture2dPtr noisePermutationTableTex;
base::Texture2dPtr colorBuffer;
base::ShaderPtr shader;
base::ShaderPtr shader_screen;
base::FBOPtr fbo;

*/
base::ShaderPtr baseShader;
base::Texture2dPtr baseTexture;
base::GeometryPtr baseGeo;

base::ShaderPtr skyShader;





using namespace std;

// sky parameters
float innerRadius = 6360.0; // inner sphere radius (earth surface distance from origin) in km
float outerRadius = 6420.0; // outer sphere radius (end of atmosphere) in km
int transmittanceIntegralSamples = 50;

// Rayleigh
float rayleighHeightScale = 8.0;
math::Vec3f betaR(5.8e-3f, 1.35e-2f, 3.31e-2f);

// Mie
float mieHeightScale = 1.2f;
math::Vec3f betaMSca(4e-3f);
math::Vec3f betaMEx = betaMSca / 0.9f;

// sky shader parameters
base::Texture2dPtr sky_transmittanceTexture;

// ----------------------------------------------------------------------------
// TOOLS
// ----------------------------------------------------------------------------

void loadTIFF(char *name, unsigned char *tex)
{
	/*
    tstrip_t strip = 0;
    tsize_t off = 0;
    tsize_t n = 0;
    TIFF* tf = TIFFOpen(name, "r");
    while ((n = TIFFReadEncodedStrip(tf, strip, tex + off, (tsize_t) -1)) > 0) {
    	strip += 1;
        off += n;
    };
    TIFFClose(tf);
	*/
}

string* loadFile(const string &fileName)
{
    string* result = new string();
	//base::Path( SRC_PATH ) + "/src/base/gfx/glsl/geometry_vs.glsl", base::Path( SRC_PATH )
    ifstream file( (base::Path( SRC_PATH ) + "/src/" + fileName).c_str());
    if (!file) {
        std::cerr << "Cannot open file " << fileName << endl;
        throw exception();
    }
    string line;
    while (getline(file, line)) {
        *result += line;
        *result += '\n';
    }
    file.close();
    return result;
}

void printShaderLog(int shaderId)
{
    int logLength;
    glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        char *log = new char[logLength];
        glGetShaderInfoLog(shaderId, logLength, &logLength, log);
        cout << string(log);
        delete[] log;
    }
}

unsigned int loadProgram(const vector<string> &files)
{
    unsigned int programId = glCreateProgram();
    unsigned int vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    unsigned int fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    int n = (int)files.size();
    string **strs = new string*[n];
    const char** lines = new const char*[n + 1];
    cout << "loading program " << files[n - 1] << "..." << endl;
    bool geo = false;
    for (int i = 0; i < n; ++i) {
        string* s = loadFile(files[i]);
        strs[i] = s;
        lines[i + 1] = s->c_str();
        if (strstr(lines[i + 1], "_GEOMETRY_") != NULL) {
            geo = true;
        }
    }

    lines[0] = "#define _VERTEX_\n";
    glShaderSource(vertexShaderId, n + 1, lines, NULL);
    glCompileShader(vertexShaderId);
    printShaderLog(vertexShaderId);

    if (geo) {
        unsigned geometryShaderId = glCreateShader(GL_GEOMETRY_SHADER_EXT);
        glAttachShader(programId, geometryShaderId);
        lines[0] = "#define _GEOMETRY_\n";
        glShaderSource(geometryShaderId, n + 1, lines, NULL);
        glCompileShader(geometryShaderId);
        printShaderLog(geometryShaderId);
        glProgramParameteriEXT(programId, GL_GEOMETRY_INPUT_TYPE_EXT, GL_TRIANGLES);
        glProgramParameteriEXT(programId, GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);
        glProgramParameteriEXT(programId, GL_GEOMETRY_VERTICES_OUT_EXT, 3);
    }

    lines[0] = "#define _FRAGMENT_\n";
    glShaderSource(fragmentShaderId, n + 1, lines, NULL);
    glCompileShader(fragmentShaderId);
    printShaderLog(fragmentShaderId);

    glLinkProgram(programId);

    for (int i = 0; i < n; ++i) {
        delete strs[i];
    }
    delete[] strs;
    delete[] lines;

    return programId;
}

void drawQuad()
{
    glBegin(GL_TRIANGLE_STRIP);
    glVertex2f(-1.0, -1.0);
    glVertex2f(+1.0, -1.0);
    glVertex2f(-1.0, +1.0);
    glVertex2f(+1.0, +1.0);
    glEnd();
}


// ----------------------------------------------------------------------------
// PRECOMPUTATIONS
// ----------------------------------------------------------------------------

const int reflectanceUnit = 0;
const int transmittanceUnit = 1;
const int irradianceUnit = 2;
const int inscatterUnit = 3;
const int deltaEUnit = 4;
const int deltaSRUnit = 5;
const int deltaSMUnit = 6;
const int deltaJUnit = 7;

unsigned int reflectanceTexture;//unit 0, ground reflectance texture
unsigned int transmittanceTexture;//unit 1, T table
unsigned int irradianceTexture;//unit 2, E table
unsigned int inscatterTexture;//unit 3, S table
unsigned int deltaETexture;//unit 4, deltaE table
unsigned int deltaSRTexture;//unit 5, deltaS table (Rayleigh part)
unsigned int deltaSMTexture;//unit 6, deltaS table (Mie part)
unsigned int deltaJTexture;//unit 7, deltaJ table

unsigned int transmittanceProg;
unsigned int irradiance1Prog;
unsigned int inscatter1Prog;
unsigned int copyIrradianceProg;
unsigned int copyInscatter1Prog;
unsigned int jProg;
unsigned int irradianceNProg;
unsigned int inscatterNProg;
unsigned int copyInscatterNProg;

unsigned int fbo;

unsigned int drawProg;

void setLayer(unsigned int prog, int layer)
{
    double r = layer / (RES_R - 1.0);
    r = r * r;
    r = sqrt(Rg * Rg + r * (Rt * Rt - Rg * Rg)) + (layer == 0 ? 0.01 : (layer == RES_R - 1 ? -0.001 : 0.0));
    double dmin = Rt - r;
    double dmax = sqrt(r * r - Rg * Rg) + sqrt(Rt * Rt - Rg * Rg);
    double dminp = r - Rg;
    double dmaxp = sqrt(r * r - Rg * Rg);

    glUniform1f(glGetUniformLocation(prog, "r"), float(r));
    glUniform4f(glGetUniformLocation(prog, "dhdH"), float(dmin), float(dmax), float(dminp), float(dmaxp));
    glUniform1i(glGetUniformLocation(prog, "layer"), layer);
}

void loadData()
{
    cout << "loading Earth texture..." << endl;
    glActiveTexture(GL_TEXTURE0 + reflectanceUnit);
    glGenTextures(1, &reflectanceTexture);
    glBindTexture(GL_TEXTURE_2D, reflectanceTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    unsigned char *tex = new unsigned char[2500 * 1250 * 4];
    loadTIFF("earth.tiff", tex);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2500, 1250, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
    glGenerateMipmapEXT(GL_TEXTURE_2D);
    delete[] tex;
}

void precompute()
{
	/*
    glActiveTexture(GL_TEXTURE0 + transmittanceUnit);
    glGenTextures(1, &transmittanceTexture);
    glBindTexture(GL_TEXTURE_2D, transmittanceTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F_ARB, TRANSMITTANCE_W, TRANSMITTANCE_H, 0, GL_RGB, GL_FLOAT, NULL);

    glActiveTexture(GL_TEXTURE0 + irradianceUnit);
    glGenTextures(1, &irradianceTexture);
    glBindTexture(GL_TEXTURE_2D, irradianceTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F_ARB, SKY_W, SKY_H, 0, GL_RGB, GL_FLOAT, NULL);

    glActiveTexture(GL_TEXTURE0 + inscatterUnit);
    glGenTextures(1, &inscatterTexture);
    glBindTexture(GL_TEXTURE_3D, inscatterTexture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F_ARB, RES_MU_S * RES_NU, RES_MU, RES_R, 0, GL_RGB, GL_FLOAT, NULL);

    glActiveTexture(GL_TEXTURE0 + deltaEUnit);
    glGenTextures(1, &deltaETexture);
    glBindTexture(GL_TEXTURE_2D, deltaETexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F_ARB, SKY_W, SKY_H, 0, GL_RGB, GL_FLOAT, NULL);

    glActiveTexture(GL_TEXTURE0 + deltaSRUnit);
    glGenTextures(1, &deltaSRTexture);
    glBindTexture(GL_TEXTURE_3D, deltaSRTexture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB16F_ARB, RES_MU_S * RES_NU, RES_MU, RES_R, 0, GL_RGB, GL_FLOAT, NULL);

    glActiveTexture(GL_TEXTURE0 + deltaSMUnit);
    glGenTextures(1, &deltaSMTexture);
    glBindTexture(GL_TEXTURE_3D, deltaSMTexture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB16F_ARB, RES_MU_S * RES_NU, RES_MU, RES_R, 0, GL_RGB, GL_FLOAT, NULL);

    glActiveTexture(GL_TEXTURE0 + deltaJUnit);
    glGenTextures(1, &deltaJTexture);
    glBindTexture(GL_TEXTURE_3D, deltaJTexture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB16F_ARB, RES_MU_S * RES_NU, RES_MU, RES_R, 0, GL_RGB, GL_FLOAT, NULL);

    vector<string> files;
    files.push_back("Main.h");
    files.push_back("sky_common.glsl");
    files.push_back("transmittance.glsl");
    transmittanceProg = loadProgram(files);

    files.clear();
    files.push_back("Main.h");
    files.push_back("sky_common.glsl");
    files.push_back("irradiance1.glsl");
    irradiance1Prog = loadProgram(files);

    files.clear();
    files.push_back("Main.h");
    files.push_back("sky_common.glsl");
    files.push_back("inscatter1.glsl");
    inscatter1Prog = loadProgram(files);

    files.clear();
    files.push_back("Main.h");
    files.push_back("sky_common.glsl");
    files.push_back("copyIrradiance.glsl");
    copyIrradianceProg = loadProgram(files);

    files.clear();
    files.push_back("Main.h");
    files.push_back("sky_common.glsl");
    files.push_back("copyInscatter1.glsl");
    copyInscatter1Prog = loadProgram(files);

    files.clear();
    files.push_back("Main.h");
    files.push_back("sky_common.glsl");
    files.push_back("inscatterS.glsl");
    jProg = loadProgram(files);

    files.clear();
    files.push_back("Main.h");
    files.push_back("sky_common.glsl");
    files.push_back("irradianceN.glsl");
    irradianceNProg = loadProgram(files);

    files.clear();
    files.push_back("Main.h");
    files.push_back("sky_common.glsl");
    files.push_back("inscatterN.glsl");
    inscatterNProg = loadProgram(files);

    files.clear();
    files.push_back("Main.h");
    files.push_back("sky_common.glsl");
    files.push_back("copyInscatterN.glsl");
    copyInscatterNProg = loadProgram(files);

    files.clear();
    files.push_back("Main.h");
    files.push_back("sky_common.glsl");
    files.push_back("earth.glsl");
    drawProg = loadProgram(files);
    glUseProgram(drawProg);
    glUniform1i(glGetUniformLocation(drawProg, "reflectanceSampler"), reflectanceUnit);
    glUniform1i(glGetUniformLocation(drawProg, "transmittanceSampler"), transmittanceUnit);
    glUniform1i(glGetUniformLocation(drawProg, "irradianceSampler"), irradianceUnit);
    glUniform1i(glGetUniformLocation(drawProg, "inscatterSampler"), inscatterUnit);

    cout << "precomputations..." << endl;

    glGenFramebuffersEXT(1, &fbo);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
    glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);

    // computes transmittance texture T (line 1 in algorithm 4.1)
    glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, transmittanceTexture, 0);
    glViewport(0, 0, TRANSMITTANCE_W, TRANSMITTANCE_H);
    glUseProgram(transmittanceProg);
    drawQuad();

    // computes irradiance texture deltaE (line 2 in algorithm 4.1)
    glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, deltaETexture, 0);
    glViewport(0, 0, SKY_W, SKY_H);
    glUseProgram(irradiance1Prog);
    glUniform1i(glGetUniformLocation(irradiance1Prog, "transmittanceSampler"), transmittanceUnit);
    drawQuad();

    // computes single scattering texture deltaS (line 3 in algorithm 4.1)
    // Rayleigh and Mie separated in deltaSR + deltaSM
    glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, deltaSRTexture, 0);
    glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, deltaSMTexture, 0);
    unsigned int bufs[2] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT };
    glDrawBuffers(2, bufs);
    glViewport(0, 0, RES_MU_S * RES_NU, RES_MU);
    glUseProgram(inscatter1Prog);
    glUniform1i(glGetUniformLocation(inscatter1Prog, "transmittanceSampler"), transmittanceUnit);
    for (int layer = 0; layer < RES_R; ++layer) {
        setLayer(inscatter1Prog, layer);
        drawQuad();
    }
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, 0, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);

    // copies deltaE into irradiance texture E (line 4 in algorithm 4.1)
    glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, irradianceTexture, 0);
    glViewport(0, 0, SKY_W, SKY_H);
    glUseProgram(copyIrradianceProg);
    glUniform1f(glGetUniformLocation(copyIrradianceProg, "k"), 0.0);
    glUniform1i(glGetUniformLocation(copyIrradianceProg, "deltaESampler"), deltaEUnit);
    drawQuad();

    // copies deltaS into inscatter texture S (line 5 in algorithm 4.1)
    glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, inscatterTexture, 0);
    glViewport(0, 0, RES_MU_S * RES_NU, RES_MU);
    glUseProgram(copyInscatter1Prog);
    glUniform1i(glGetUniformLocation(copyInscatter1Prog, "deltaSRSampler"), deltaSRUnit);
    glUniform1i(glGetUniformLocation(copyInscatter1Prog, "deltaSMSampler"), deltaSMUnit);
    for (int layer = 0; layer < RES_R; ++layer) {
        setLayer(copyInscatter1Prog, layer);
        drawQuad();
    }

    // loop for each scattering order (line 6 in algorithm 4.1)
    for (int order = 2; order <= 4; ++order)
	{

        // computes deltaJ (line 7 in algorithm 4.1)
        glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, deltaJTexture, 0);
        glViewport(0, 0, RES_MU_S * RES_NU, RES_MU);
        glUseProgram(jProg);
        glUniform1f(glGetUniformLocation(jProg, "first"), order == 2 ? 1.0 : 0.0);
        glUniform1i(glGetUniformLocation(jProg, "transmittanceSampler"), transmittanceUnit);
        glUniform1i(glGetUniformLocation(jProg, "deltaESampler"), deltaEUnit);
        glUniform1i(glGetUniformLocation(jProg, "deltaSRSampler"), deltaSRUnit);
        glUniform1i(glGetUniformLocation(jProg, "deltaSMSampler"), deltaSMUnit);
        for (int layer = 0; layer < RES_R; ++layer)
		{
			//cout << "check " << layer << "/" << RES_R << " order:" << order << endl << flush;
            setLayer(jProg, layer);
            drawQuad();
        }

        // computes deltaE (line 8 in algorithm 4.1)
        glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, deltaETexture, 0);
        glViewport(0, 0, SKY_W, SKY_H);
        glUseProgram(irradianceNProg);
        glUniform1f(glGetUniformLocation(irradianceNProg, "first"), order == 2 ? 1.0 : 0.0);
        glUniform1i(glGetUniformLocation(irradianceNProg, "transmittanceSampler"), transmittanceUnit);
        glUniform1i(glGetUniformLocation(irradianceNProg, "deltaSRSampler"), deltaSRUnit);
        glUniform1i(glGetUniformLocation(irradianceNProg, "deltaSMSampler"), deltaSMUnit);
        drawQuad();

        // computes deltaS (line 9 in algorithm 4.1)
        glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, deltaSRTexture, 0);
        glViewport(0, 0, RES_MU_S * RES_NU, RES_MU);
        glUseProgram(inscatterNProg);
        glUniform1f(glGetUniformLocation(inscatterNProg, "first"), order == 2 ? 1.0 : 0.0);
        glUniform1i(glGetUniformLocation(inscatterNProg, "transmittanceSampler"), transmittanceUnit);
        glUniform1i(glGetUniformLocation(inscatterNProg, "deltaJSampler"), deltaJUnit);
        for (int layer = 0; layer < RES_R; ++layer) {
            setLayer(inscatterNProg, layer);
            drawQuad();
        }

        glEnable(GL_BLEND);
        glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
        glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE);

        // adds deltaE into irradiance texture E (line 10 in algorithm 4.1)
        glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, irradianceTexture, 0);
        glViewport(0, 0, SKY_W, SKY_H);
        glUseProgram(copyIrradianceProg);
        glUniform1f(glGetUniformLocation(copyIrradianceProg, "k"), 1.0);
        glUniform1i(glGetUniformLocation(copyIrradianceProg, "deltaESampler"), deltaEUnit);
        drawQuad();

        // adds deltaS into inscatter texture S (line 11 in algorithm 4.1)
        glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, inscatterTexture, 0);
        glViewport(0, 0, RES_MU_S * RES_NU, RES_MU);
        glUseProgram(copyInscatterNProg);
        glUniform1i(glGetUniformLocation(copyInscatterNProg, "deltaSSampler"), deltaSRUnit);
        for (int layer = 0; layer < RES_R; ++layer) {
            setLayer(copyInscatterNProg, layer);
            drawQuad();
        }
        glDisable(GL_BLEND);
    }

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    glFinish();
    cout << "ready." << endl << flush;
    glUseProgram(drawProg);
	*/

}





// ----------------------------------------------------------------------------
// RENDERING
// ----------------------------------------------------------------------------


int width, height;
int oldx, oldy;
int move;

vec3f s(0.0, -1.0, 0.0);

double lon = 0.0;
double lat = 0.0;
double theta = 0.0;
double phi = 0.0;
double d = Rg;
vec3d position;
mat4d view;

double exposure = 0.4;

void updateView()
{
	/*
	double co = cos(lon);
	double so = sin(lon);
	double ca = cos(lat);
	double sa = sin(lat);
	vec3d po = vec3d(co*ca, so*ca, sa) * Rg;
	vec3d px = vec3d(-so, co, 0);
    vec3d py = vec3d(-co*sa, -so*sa, ca);
    vec3d pz = vec3d(co*ca, so*ca, sa);

    double ct = cos(theta);
    double st = sin(theta);
    double cp = cos(phi);
    double sp = sin(phi);
    vec3d cx = px * cp + py * sp;
    vec3d cy = -px * sp*ct + py * cp*ct + pz * st;
    vec3d cz = px * sp*st - py * cp*st + pz * ct;
    position = po + cz * d;

    if (position.length() < Rg + 0.01) {
    	position.normalize(Rg + 0.01);
    }

    view = mat4d(cx.x, cx.y, cx.z, 0,
            cy.x, cy.y, cy.z, 0,
            cz.x, cz.y, cz.z, 0,
            0, 0, 0, 1);
    view = view * mat4d::translate(-position);
	*/
}



void render( base::CameraPtr cam )
{

	//glClampColorARB(GL_CLAMP_VERTEX_COLOR_ARB, GL_FALSE);glClampColorARB(GL_CLAMP_READ_COLOR_ARB, GL_FALSE);glClampColorARB(GL_CLAMP_FRAGMENT_COLOR_ARB, GL_FALSE);
	glDisable( GL_CULL_FACE );
	//glEnable( GL_DEPTH_TEST );


	context->setView( cam->m_viewMatrix, cam->m_transform, cam->m_projectionMatrix );


	// render to screen
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	context->render( geo, baseShader );
	context->renderScreen( skyShader );

	/*


    float h = position.length() - Rg;
    float vfov = 2.0 * atan(float(height) / float(width) * tan(80.0 / 180 * M_PI / 2.0)) / M_PI * 180;
    mat4f proj = mat4f::perspectiveProjection(vfov, float(width) / float(height), 0.1 * h, 1e5 * h);

    mat4f iproj = proj.inverse();
    mat4d iview = view.inverse();
    vec3d c = iview * vec3d(0.0, 0.0, 0.0);

    mat4f iviewf = mat4f(iview[0][0], iview[0][1], iview[0][2], iview[0][3],
        iview[1][0], iview[1][1], iview[1][2], iview[1][3],
        iview[2][0], iview[2][1], iview[2][2], iview[2][3],
        iview[3][0], iview[3][1], iview[3][2], iview[3][3]);

    glUniform3f(glGetUniformLocation(drawProg, "c"), c.x, c.y, c.z);
    glUniform3f(glGetUniformLocation(drawProg, "s"), s.x, s.y, s.z);
    glUniformMatrix4fv(glGetUniformLocation(drawProg, "projInverse"), 1, true, iproj.coefficients());
    glUniformMatrix4fv(glGetUniformLocation(drawProg, "viewInverse"), 1, true, iviewf.coefficients());
    glUniform1f(glGetUniformLocation(drawProg, "exposure"), exposure);
    drawQuad();
	*/
    //glutSwapBuffers();

	//loadData();
	//precompute();
	//updateView();
}



void updateSunDir( const math::Vec3f &vec  )
{
	//std::cout << vec.x << " " << vec.y << " " << vec.z << " " << vec.getLength() << std::endl;
	//cloudShader->setUniform( "sunDir", vec );
	glviewer->update();
}

float closestIntersection(float height, float cosViewAngle)
{
	//calculate intersection with outer atmosphere
	float outer_t1 = -height*cosViewAngle + sqrt( height*height*(cosViewAngle*cosViewAngle - 1.0f) + outerRadius*outerRadius );
	//computer disciminant of inner sphere intersection
	float inner_discriminant =  height*height*(cosViewAngle*cosViewAngle - 1.0f) + innerRadius*innerRadius;
	// is there an intersection?
	if(inner_discriminant)
	{
		// computer inner sphere intersection
		float inner_t1 = -height*cosViewAngle + sqrt(inner_discriminant);
		// is it in front of ray origin ?
		if(inner_t1 >= 0.0)
		{
			return std::min( inner_t1, outer_t1 );
		}
	}

	return outer_t1;
}

float getOpticalDepth( float height, float cosViewAngle, float heightScale )
{
	/*
   computeOpticalDepth
	   ->intersect ray from camera to outerSphere and innerSphere
	   ->construct ray from closest intersection and divide ray length by number of sampling points
	   ->do raymarching
		   ->compute density at current height
		   ->add density to accumulated height
		   ->update position with raystep
	   ->return accumulated density
	   */

	// get length of one raysegment
	float dx = closestIntersection( height, cosViewAngle )/(float)(transmittanceIntegralSamples);
	float opticalDepth = 0.0f;
	float lastDensity = exp( -(height - innerRadius)/heightScale );

	for( int i=1;i<transmittanceIntegralSamples;++i )
	{
		float distanceTravelled = i*dx;
		float currentHeight = sqrt( height*height + distanceTravelled*distanceTravelled + 2.0f*height*distanceTravelled*cosViewAngle );
		float currentDensity = exp( -(currentHeight - innerRadius)/heightScale );

		// we integrate density by using trapezoidal rule
		opticalDepth += 0.5f*dx*lastDensity + 0.5f*dx*currentDensity;

		lastDensity = currentDensity;
	}



	return opticalDepth;
}

base::Texture2dPtr setupTransmittanceTexture()
{
	/*
	create 2d transmittance texture (256x64, rgba_float_16, clamp, linear)
	  for each pixel
		  -> get r and view angle
			  x:r goes from Rg to Rt
			  y:muS goes from -0.15 to 1.0
		  ->compute optical depth
			  which is a sum of opticalDepth for Rayleigh and Miescattering (multiplied by respective beta values)
		  ->compute transmittance by exp(-opticalDepth)
	*/


	base::Texture2dPtr tex = base::Texture2d::createRGBAFloat32();

	float *data = (float*)malloc( tex->m_xres*tex->m_yres*sizeof(float)*4 );

	for( int j = 0; j<tex->m_yres; ++j )
		for( int i = 0; i<tex->m_xres; ++i )
		{
			float u = (float)i/(float)(tex->m_xres-1);
			float v = (float)j/(float)(tex->m_yres-1);

			// we need to flip v vertically because opengl textures have 0,0 at lower bottom
			v = 1.0f - v;

			float cosViewAngle = 2.0f * u - 1.0f;
			float height = innerRadius + v*(outerRadius-innerRadius);

			math::Vec3f rayleigh_opticalDepth = betaR*getOpticalDepth( height, cosViewAngle, rayleighHeightScale );
			math::Vec3f mie_opticalDepth = betaMEx*getOpticalDepth( height, cosViewAngle, mieHeightScale );
			math::Vec3f opticalDepth = rayleigh_opticalDepth + mie_opticalDepth;
			math::Vec3f transmittance = math::Vec3f( exp( -opticalDepth.x ), exp( -opticalDepth.y ), exp( -opticalDepth.z ) );

			data[j*tex->m_xres*4 + i*4] = transmittance.x;
			data[j*tex->m_xres*4 + i*4 + 1] = transmittance.y;
			data[j*tex->m_xres*4 + i*4 + 2] = transmittance.z;
			data[j*tex->m_xres*4 + i*4 + 3] = 0.0f;
			//std::cout << "height: " << height << "     cosViewAngle: " << cosViewAngle << "       transmittance: " << transmittance.x << ", " << transmittance.y << ", " << transmittance.z << std::endl;
		}

	tex->uploadRGBAFloat32( tex->m_xres, tex->m_yres, data );
	return tex;
}


void init()
{
	std::cout << "init!\n";


	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cout << "glew init failed\n";
	}

	context = base::ContextPtr( new base::Context() );


	// op testing
	base::ops::SphereOpPtr s = base::ops::SphereOp::create(innerRadius);
	base::MeshPtr m = s->getMesh(0);
	geo = m->getGeometry();


	// rendering
	/*

	RenderOpPtr rop = base::ops::RenderOp::create();
	rop->append( base::ops::ClearOp::create() );
	rop->append( base::ops::SkyOp::create() );
	rop->append( base::ops::RenderMeshOp::create() );

	rop->execute();

	*/

	/*

	  create 2d transmittance texture (256x64, rgba_float_16, clamp, linear)
		for each pixel
			-> get r and view angle
				x:r goes from Rg to Rt
				y:muS goes from -0.15 to 1.0
			->compute optical depth
				which is a sum of opticalDepth for Rayleigh and Miescattering (multiplied by respective beta values)
			->compute transmittance by exp(-opticalDepth)
		computeOpticalDepth
			->intersect ray from camera to outerSphere and innerSphere
			->construct ray from closest intersection and divide ray length by number of sampling points
			->do raymarching
				->compute density at current height
				->add density to accumulated height
				->update position with raystep
			->return accumulated density

	  */



	sky_transmittanceTexture = setupTransmittanceTexture();









	skyShader = base::Shader::load( base::Path( SRC_PATH ) + "/src/sky.vs.glsl", base::Path( SRC_PATH ) + "/src/sky.ps.glsl" );
	skyShader->setUniform( "transmittanceSampler", sky_transmittanceTexture->getUniform() );
	skyShader->setUniform( "innerRadius", innerRadius );
	skyShader->setUniform( "outerRadius", outerRadius );











	// tmp for obj io:

	baseShader = base::Shader::load( base::Path( SRC_PATH ) + "/src/base/gfx/glsl/geometry_vs.glsl", base::Path( SRC_PATH ) + "/src/base/gfx/glsl/geometry_ps.glsl" );
	baseGeo = base::importObj( base::Path( SRC_PATH ) + "/data/test.1.obj" );
	//base::apply_transform( baseGeo, math::Matrix44f::ScaleMatrix( 30000.0f ) );
	base::apply_normals( baseGeo );

	baseTexture = base::Texture2d::load( base::Path( SRC_PATH ) + "/src/base/data/uvref2.png" );
	baseShader->setUniform( "input", baseTexture->getUniform() );






	//loadData();
	//precompute();
	//updateView();
}

struct event_visitor
{
   void operator()( int x )
   {
	  std::cout<<"int event: "<<x<<"\n";
   }
   void operator()( double x )
   {
	  std::cout<<"double event: "<<x<<"\n";
   }
};

int main(int argc, char ** argv)
{
	QApplication app(argc, argv);
	app.setOrganizationName("test");
	app.setApplicationName("test");

	QMainWindow mainWin;
	mainWin.resize(800, 600);
	width = 800;
	height = 600;

	glviewer = new composer::widgets::GLViewer(init, render);
	glviewer->getCamera()->m_znear = .1f;
	glviewer->getCamera()->m_zfar = 100000.0f;
	glviewer->setView( math::Vec3f(0.0f, innerRadius + 0.95 * (outerRadius-innerRadius), 0.0f), 10.0f, 0.0f, 0.0f );

	mainWin.setCentralWidget( glviewer );
	mainWin.show();

	return app.exec();
}
