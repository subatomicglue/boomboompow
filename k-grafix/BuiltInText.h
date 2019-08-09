
#ifndef BUILTINTEXT
#define BUILTINTEXT

#ifdef WIN32
   #include <windows.h>  // make the app win32 friendly. :)
   #include "resource.h"
#endif
#ifndef EXCLUDE_GLEW
#  include <GL/glew.h>
#endif

#ifdef USE_SDL
#include <SDL.h>
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
#endif

#include <assert.h>
#if _MSC_VER >= 1300 // vis C++ 7.0 or higher
   #include <hash_map>
   #define HASHMAP_NAMESPACE stdext
#elif defined __clang__
   #include <unordered_map>
   #define HASHMAP_NAMESPACE std
   #define hash_map unordered_map
#elif defined __GNUC__ || defined __APPLE__
   #include <ext/hash_map>
   #define HASHMAP_NAMESPACE __gnu_cxx
#else
   #include <hash_map>
   #define HASHMAP_NAMESPACE std
#endif

#include "PixmapFont.h"

extern PixmapFont gBuiltInText;

namespace BuiltInText
{
   inline int maximum( int a, int b )
   {
      return a > b ? a : b;
   }
   enum Filtering
   {
      NEAREST, LINEAR, MIPMAP_NEAREST, MIPMAP_LINEAR
   };

   //: Number of pixels in one scanline, a pixel is typically 1,3, or 4 bytes
   extern const unsigned int PixmapFontWidth;

   //: Number of scanlines in image data
   extern const unsigned int PixmapFontHeight;

   //: Number of color channels in the image
   //  Returns a number: '1' (Greyscale/Luminance), '3' (RGB), '4' (RGBA)
   extern const unsigned int PixmapFontChannels;

   //: Scanline alignment, each row's size in bytes is a multiple of this
   //  Each row is padded with (width % alignment) non-pixel bytes
   extern const unsigned int PixmapFontAlignment;

   //: Scanline padding
   //  This is the number of bytes (non-pixel data) attached to the end of each scanline.
   extern const unsigned int PixmapFontPadding;

   //: Number of bits per pixel
   //  Typical values include 8, 15, 16, 24, 32
   extern const unsigned int PixmapFontBpp;

   //: Image data
   //  Orientation is by scanline. RGBARGBARGBAX etc... where RGBA is pixel, X is padding
   extern const unsigned char PixmapFontData[];

#ifndef EXCLUDE_GLEW
   static const int GL_TEXTUREX_ARB_Lookup[] = {
      GL_TEXTURE0_ARB,
      GL_TEXTURE1_ARB,
      GL_TEXTURE2_ARB,
      GL_TEXTURE3_ARB,
      GL_TEXTURE4_ARB,
      GL_TEXTURE5_ARB,
      GL_TEXTURE6_ARB,
      GL_TEXTURE7_ARB,
   };
#endif

   extern HASHMAP_NAMESPACE::hash_map<size_t, unsigned int> PixmapFontTextureID;
   extern size_t PixmapFontCurrentCtx; // current ctx.  no need to set if only using one window
	extern unsigned int* PixmapFontTextureIDPtr; // internal ctx


   inline int loadTex( const unsigned char* data, 
               int width, int height, Filtering f = NEAREST, 
               int mipmapLevelOfDetail = 0,
               int format = GL_RGBA, 
               int type = GL_UNSIGNED_BYTE, 
               int bpc = 32,
               int channels = 4, 
               int bordersize = 0 )
   {
#if !TARGET_OS_IPHONE && !defined(EXCLUDE_GLEW)
      // fatal error. Nothing to render
      if ( data == NULL )
         return false; 

      if (channels == 4)
      {
         glAlphaFunc( GL_GEQUAL, 0.25f );
         glEnable( GL_ALPHA_TEST );

         glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
         glEnable( GL_BLEND );
      }

#ifndef EXCLUDE_GLEW
      for (int x = 1; x < 8; ++x)
      {
         ::glActiveTextureARB( GL_TEXTUREX_ARB_Lookup[x] );
         ::glDisable( GL_TEXTURE_2D );
      }
      ::glActiveTextureARB( GL_TEXTUREX_ARB_Lookup[0] );
#endif
	  ::glEnable( GL_TEXTURE_2D );

      //make sure the alignment matches the pixel size in bytes
      ::glPixelStorei( GL_UNPACK_ALIGNMENT, PixmapFontAlignment );

      // texture bind!!
	  if (*PixmapFontTextureIDPtr != 0)
      {
         // already loaded texture, just queue up the texobj
         ::glBindTexture( GL_TEXTURE_2D, *PixmapFontTextureIDPtr );
         return true; //< already have a tex obj
      }
      else
      {
		 ::glGenTextures( 1, PixmapFontTextureIDPtr );
         ::glBindTexture( GL_TEXTURE_2D, *PixmapFontTextureIDPtr );
      }

      // load texture (it wasn't yet loaded)...

      // figure out what filtering to use.
      // set the filtering for the texture...
      ::glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
      ::glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
      int texDimension;
      if( height == 1 || width == 1 )
      {
         texDimension = GL_TEXTURE_1D;	
      } 
      else 
      {
         texDimension = GL_TEXTURE_2D;
      }
      switch (f)
      {
      case NEAREST:
         ::glTexParameteri( texDimension, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
         ::glTexParameteri( texDimension, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
      break;

      case LINEAR:
         ::glTexParameteri( texDimension, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
         ::glTexParameteri( texDimension, GL_TEXTURE_MIN_FILTER, GL_LINEAR );	
      break;

      case MIPMAP_NEAREST:
         ::glTexParameteri( texDimension, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
         ::glTexParameteri( texDimension, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST );
      break;

      case MIPMAP_LINEAR:
         ::glTexParameteri( texDimension, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
         ::glTexParameteri( texDimension, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
      break;
      default:
         assert(false);
      }

      // find out what dimension texture we're using
      if( height == 1 || width == 1 )
      {
         // handle one dimensional images...
         int length = maximum( width, height );
         if (f != NEAREST ||
               f != LINEAR)
         {
#ifdef USE_OLD_OPENGL
            ::gluBuild1DMipmaps( GL_TEXTURE_1D, 
                                 channels, length, 
                                 format, type, data );
#else
            ::glTexImage1D( GL_TEXTURE_1D, 0,
                           channels, length,
                           bordersize, format, type, data );
            ::glGenerateMipmap( GL_TEXTURE_2D );
#endif
         }
         else
         {
            ::glTexImage1D( GL_TEXTURE_1D, mipmapLevelOfDetail,
                           channels, length,
                           bordersize, format, type, data );
         }
      } 

      else 
      {
         // other wise, handle 2D images.
         if (f == MIPMAP_NEAREST ||
               f == MIPMAP_LINEAR)
         {
#ifdef USE_OLD_OPENGL
            ::gluBuild2DMipmaps( GL_TEXTURE_2D,
                                 channels, width,
                                 height, format, type,
                                 data );
#else
            ::glTexImage2D( GL_TEXTURE_2D, 0,
                           channels, width,
                           height, bordersize, format,
                           type, data );
            ::glGenerateMipmap( GL_TEXTURE_2D );
#endif
         }
         else
         {
            ::glTexImage2D( GL_TEXTURE_2D, mipmapLevelOfDetail,
                           channels, width,
                           height, bordersize, format,
                           type, data );
         }
      }

      ::glEnable( GL_TEXTURE_2D );
#endif
      return true;
   }

   inline void init()
   {
      gBuiltInText.initBasic();
   }

   inline void pushTex()
   {
	  BuiltInText::loadTex( BuiltInText::PixmapFontData, BuiltInText::PixmapFontWidth, BuiltInText::PixmapFontHeight, BuiltInText::MIPMAP_LINEAR );
   }

///////////////////////////
// interface:
///////////////////////////



   /// call this function once per frame, for every window you're using built in text
   /// sets the context used for texture object lookup
   inline void setCtx( size_t ctx )
   {
	   PixmapFontCurrentCtx = ctx;
	   PixmapFontTextureIDPtr = &PixmapFontTextureID[PixmapFontCurrentCtx];
   }

   /// call to inform the text that the ctx has been trashed and needs refreshing
   inline void resetCtx( size_t ctx )
   {
	   PixmapFontTextureID[ctx] = 0;
   }

   /// call this function each frame to render text at a certain position
   /// use glColor to set color of text.
   inline void render( const char* const buf, float x, float y, float z, float size, float scalex = 1.0f )
   {
		if (!gBuiltInText.isInitialized())
		{
			BuiltInText::init();
		}
		pushTex();
		gBuiltInText.render( buf, x, y, z, size, scalex );
   }

   /// call this function each frame to render text at a certain position
   /// use glColor to set color of text.
   /// note: this vesion is upside down, for viewports that are upsidedown
   inline void renderUpsideDown( const char* const buf, float x, float y, float z, float size, float scalex = 1.0f )
   {
	   if (!gBuiltInText.isInitialized())
	   {
		   BuiltInText::init();
	   }
	   pushTex();
	   gBuiltInText.renderUpsideDown( buf, x, y, z, size, scalex );
   }

   inline void renderTextWithShadow( float r, float g, float b, const char* text, float x, float y, float z, float size, float offset, float scalex = 1.0f )
   {
#ifdef USE_OLD_OPENGL
      glColor3f( 0.0f,0.0f,0.0f );
#endif
      BuiltInText::renderUpsideDown( text, offset+x,offset+y,z,size, scalex );
#ifdef USE_OLD_OPENGL
      glColor3f( r, g, b );
#endif
      BuiltInText::renderUpsideDown( text, x,y,z, size, scalex );
   }

}



#endif
