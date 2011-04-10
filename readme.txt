TODO:



base:
-figure out how to handle forward declaration. compiletime will increase if we dont do something here
-Attribute bind function uploads all data every time - should only be done when something has changed
-context render uploads the whole index buffer everytime geometry is rendered - should be done only when index buffer changes
-attribute bind as uniform, when being a sampler used uniformindex as texture unit to bind to - very bad - need textureunit management
-get rid of python dependency when doing the glsl compilation by having c++ code doing it with cmake


clouds:
-understand double and 3+ scattering equations
-understand which units have to be passed to the shader (re N0 etc.)
-clarify when to use PF Ps and P and what they return for different values of theta
--check if Ps really has to return 0 when theta<theta_f, its the phase function for scattering...

Martin Jondo Reggae