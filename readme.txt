













DEMO
====

the demo branch is about building the very basics required to run the demo. So this is actually about the juicy bits! Camera, motionpath, timing, sound, fx generalisation


fbx:

aim: get basic scene player functionality in a way which allows us to extend and ammend scene aspects

1. renderFBX scene op
->problem: nothing is exposed
2. turn fbx into full operator graph
->problem we will spend a lot of work replicating fbx features

-> what we will do
importFBX will create a operator graph which represents what the code would look like if I were to render
the fbx content programatically:

setCamera (using transform from baked fbx transform hierarchy)
for each entity:
	transform (baked from fbx transform hierarchy)
		renderEntity (e.g. renderMesh/Geometry)


first stage (end december): basic demo functionality
----------------------------------------------------
-mesh import and rendering (uv, vertexcolor, normals with nice shader) using fbx http://usa.autodesk.com/adsk/servlet/pc/index?id=6837478&siteID=123112
DONE:-animated camera
DONE:-sound
-animated object (animated parameter in operator graph)
-zipfile support (3rd party library zlib - fs api)


second stage (March): effects
-----------------------------

-particle nebulae
--atmo particle shape varuiations
--floating dust (moves with the camera)
--starmap for environment
--3dsmax/houdini io for camera and bok globule placement 

-particle - clouds with fake translucency
--do cloudscape setup in 3dsmax
--combine with starmap and atmospheric scattering

-atmospheric scattering
--continue with building map

-animated clouds surface shader
--redo effect (do it properly)

-demoOp: sequencer
-postprocessing (ssao, dof, tonemapping, vignetting)


third stage (April): art direction
----------------------------------




TODO:
-get rid of base::ops::Context


base:
-figure out how to handle forward declaration. compiletime will increase if we dont do something here
-Attribute bind function uploads all data every time - should only be done when something has changed
-context render uploads the whole index buffer everytime geometry is rendered - should be done only when index buffer changes
-attribute bind as uniform, when being a sampler used uniformindex as texture unit to bind to - very bad - need textureunit management
-get rid of python dependency when doing the glsl compilation by having c++ code doing it with cmake


clouds:
http://www.opengl.org/registry/specs/EXT/framebuffer_sRGB.txt
-understand double and 3+ scattering equations
-understand which units have to be passed to the shader (re N0 etc.)
-clarify when to use PF Ps and P and what they return for different values of theta
--check if Ps really has to return 0 when theta<theta_f, its the phase function for scattering...

Martin Jondo Reggae




linux
----------------
configure -prefix /somedir/dev/libs/qt/4.7.3 -prefix-install -debug-and-release -opensource -shared -platform linux-g++-64 -no-webkit -no-phonon -no-phonon-backend -no-script -no-scripttools -no-qt3support -no-multimedia
                                     



building the project
====================

I will need to have installed the following tools and libraries:
-git client for checking out the source code (I used msysgit for windows)
-cmake 2.8 or higher
-fbxsdk 2012.2
-qt 4.7 or 4.8


-downloading and installing git, cmake and fbxsdk is straightforward
-we need a 64bit build of qt which isnt provided in the download section. so you need to download the source and build it yourself. here is how to do it:

building qt for 64bit on windows
--------------------------------
-download/extract qt-everywhere-opensource
-copy to final location
-setup PATH environment variable to point to directory containing the extracted files (configure.exe)
-Open vs2010 cross tools command prompt. cd into the Qt directory where configure.exe lives
NOTE:if you want to compile 64bit, use Win64 command prompt
-nmake confclean (if rebuilding)
-configure.exe -debug-and-release -opensource -shared -platform wi
n32-msvc2010 -no-webkit -no-phonon -no-phonon-backend -no-script -no-scripttools
 -no-qt3support -no-multimedia -no-ltcg
-nmake
-after build has finished, setup Path environment variable to point to the bin directory of your qt install


linux
-----
-setenv FBXSDKPATH /to/the/path/of/fbxsdk


ISSUES
=======

-bad rotatation when exporting from 3dsmax


-animation glitches
try:
KFbxAnimCurveNode *curveNode = boneNode->LclRotation.GetCurveNode(lAnimLayer);
 if (curveNode)
 {
 KFbxAnimCurveFilterUnroll lUnrollFilter;
 lUnrollFilter.SetForceAutoTangents(true);
 lUnrollFilter.Apply(*curveNode);
 } 
