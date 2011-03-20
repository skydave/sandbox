TODO:



base:
-figure out how to handle forward declaration. compiletime will increase if we dont do something here
-Attribute bind function uploads all data every time - should only be done when something has changed
-context render uploads the whole index buffer everytime geometry is rendered - should be done only when index buffer changes
-attribute bind as uniform, when being a sampler used uniformindex as texture unit to bind to - very bad - need textureunit management
-for some reason qtcreator complains that its not in debug mode when debugging
-handle glsl compilation
-get rid of python dependency when doing the glsl compilation by having c++ code doing it with cmake


clouds:
-get perlin noise in glsl running
-setup cloud scene