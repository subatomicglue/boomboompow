

#ifndef COMMON_H
#define COMMON_H

#if _DEBUG_NEW_REDEFINE_NEW
//#include <nvwa/debug_new.h>
#endif

# ifdef USE_SDL
#  include <SDL.h>
#endif

#if defined(USE_OLD_OPENGL)

# ifdef __APPLE__
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>
# else
#  include <GL/gl.h>
#  include <GL/glu.h>
# endif
# if !defined(EXCLUDE_GLEW)
#include <GL/glew.h>
#endif

#else

#ifdef USE_SDL
#if TARGET_OS_IPHONE
#include <SDL_opengles2.h>
#else
#  ifndef EXCLUDE_GLEW
#     include <SDL_opengl.h>
#  else
#     include <GLES2/gl2.h>
#     include <SDL_opengles2.h>
#  endif
#endif
# else
fuck no opengl
# endif

#endif


#endif


