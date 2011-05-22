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


particles:
http://www.youtube.com/user/InfinitySupport#p/u/13/LOxFvRy3KdQ
http://www.infinity-universe.com/Infinity/index.php?option=com_content&task=blogcategory&id=22&Itemid=49&limit=6&limitstart=30
http://www.infinity-universe.com/Infinity/index.php?option=com_content&task=view&id=104&Itemid=49
http://www.infinity-universe.com/Infinity/index.php?option=com_content&task=view&id=64&Itemid=49
http://www.infinity-universe.com/Infinity/index.php?option=com_content&task=view&id=67&Itemid=49
http://www.infinity-universe.com/Infinity/index.php?option=com_content&task=view&id=77&Itemid=49
http://www.infinity-universe.com/Infinity/index.php?option=com_zoom&Itemid=81&catid=4&PageNo=3&nxShowImage=nebulae44.jpg
http://gallery.artofgregmartin.com/tuts_arts/making_a_star_field.html
http://gallery.artofgregmartin.com/tuts_arts/making_a_planet.html
http://www.gamedev.net/topic/477224-spherical-environment-map-how-big/page__p__4128448__hl__starfield__fromsearch__1#entry4128448
http://en.wikipedia.org/wiki/Infinity_%28MMOG%29
http://archive.gamedev.net/community/forums/mod/journal/journal.asp?jn=263350&cmonth=1&cyear=2005
http://www.gamedev.net/topic/406936-some-crits-and-comments-needed-for-starfieldnebula-skybox-test-56k-beware/page__p__3708632__hl__nebulae__fromsearch__1#entry3708632
http://www.gamedev.net/topic/308727-making-space-clouds-or-nebula/page__p__2971031__hl__nebulae__fromsearch__1#entry2971031

http://www.infinity-universe.com/Infinity/index.php?option=com_smf&Itemid=75
http://www.infinity-universe.com/Infinity/index.php?option=com_smf&Itemid=75&board=6.75
http://www.infinity-universe.com/Infinity/index.php?option=com_smf&Itemid=75&topic=2193.0
http://www.infinity-universe.com/Infinity/index.php?option=com_smf&Itemid=75&topic=1780.0
http://www.infinity-universe.com/Infinity/index.php?option=com_smf&Itemid=75&topic=1769.0
http://www.infinity-universe.com/Infinity/index.php?option=com_smf&Itemid=75&topic=1728.0
http://www.infinity-universe.com/Infinity/index.php?option=com_smf&Itemid=75&topic=1670.0
http://www.stanford.edu/~svlevine/papers/nebula.pdf
http://www.shmup-dev.com/forum/index.php?topic=1870.0
http://www.fedoraforum.org/forum/showthread.php?t=223383
http://www.ploksoftware.org/ExNihilo/pages/Tutorialpointsprite.htm
http://www.gpgpu.org/forums/viewtopic.php?p=21834
http://www.opengl.org/discussion_boards/ubbthreads.php?ubb=showflat&Number=223395
http://www.codesampler.com/oglsrc/oglsrc_6.htm#Particle%20System%20%28Using%20NV_point_sprites%29
http://www.opengl.org/discussion_boards/ubbthreads.php?ubb=showflat&Number=220459
http://www.opengl.org/discussion_boards/ubbthreads.php?ubb=showflat&Number=168123
http://www.khronos.org/message_boards/viewtopic.php?f=43&t=2184

Martin Jondo Reggae

-download/extract qt-everywhere-opensource
-copy to final location
-setup path environment variable to point to directory containing qmake
-Open vs2010 cross tools command prompt. cd into the Qt directory where configure.exe lives
-nmake confclean (if rebuilding)
-configure.exe -debug-and-release -opensource -shared -platform wi
n32-msvc2010 -no-webkit -no-phonon -no-phonon-backend -no-script -no-scripttools
 -no-qt3support -no-multimedia -no-ltcg
-nmake
