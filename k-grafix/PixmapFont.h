#ifndef PIXMAP_FONT
#define PIXMAP_FONT

#include <string>
#include <vector>
#include <assert.h>

#ifdef WIN32
   #include <windows.h>  // make the app win32 friendly. :)
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


class PixmapFont
{
	// don't use gmtl
	// make this class standalone as possible so it can be used in other projects easily..
	struct Vec2f
	{
		Vec2f() {mData[0] = 0; mData[1] = 0;}
		Vec2f( float x, float y ) {mData[0] = x; mData[1] = y;}
		Vec2f( const Vec2f& v ) {mData[0] = v.mData[0]; mData[1] = v.mData[1];}
		float operator[]( int x ) { return mData[x]; }
		const float* getData() const { return mData; }
		float mData[2];
	};
public:
   PixmapFont() : mInitialized( false )
   {
      mMapping.resize( 256 );
   }

   class GlyphData
   {
   public:
      void setWidth( float w ) { mWidth = w; }
      //void setHeight( float h ) { mHeight = w; }
      //const float& height() const { return mHeight; }
      const float& width() const { return mWidth; }
      float& width() { return mWidth; }
      Vec2f t[4];
   private:
      float mWidth;
      //float mHeight;
   };

   void setHeight( float height )
   {
      mHeight = height;
   }

   void setGlyphData( int x, GlyphData& mapping )
   {
      printf( "this:%lu mMapping.size%lu\n", (size_t)this, mMapping.size() );
      assert( 0 <= x && x < mMapping.size() && "oops" );
      mMapping[x] = mapping;
      mInitialized = true;
   }

   const float& height() const { return mHeight; }
   const GlyphData& glyphData( int whichGlyph ) const { return mMapping[whichGlyph]; }

   void initBasic()
   {
		mInitialized = true;
      for (int x = 0; x < 16; ++x)
      {
         for (int y = 0; y < 16; ++y)
         {
            GlyphData d;
            d.setWidth( 1 );
            // 3 2
            // 0 1  <-- quad data layout
            d.t[3] = Vec2f( ((float)x) / 16.0f, 1.0f - ((float)y) / 16.0f );
            d.t[0] = Vec2f( ((float)x) / 16.0f, 1.0f - ((float)y+1.0f) / 16.0f );
            d.t[2] = Vec2f( ((float)x+1.0f) / 16.0f, 1.0f - ((float)y) / 16.0f );
            d.t[1] = Vec2f( ((float)x+1.0f) / 16.0f, 1.0f - ((float)y+1.0f) / 16.0f );

            // flip
            d.t[3] = Vec2f( ((float)x) / 16.0f, 1.0f - ((float)y+1.0f) / 16.0f );
            d.t[0] = Vec2f( ((float)x) / 16.0f, 1.0f - ((float)y) / 16.0f );
            d.t[2] = Vec2f( ((float)x+1.0f) / 16.0f, 1.0f - ((float)y+1.0f) / 16.0f );
            d.t[1] = Vec2f( ((float)x+1.0f) / 16.0f, 1.0f - ((float)y) / 16.0f );

            this->setGlyphData( x + y*16, d );

            // flip vertically...
            //this->setGlyphData( (x + (15-y)*16), d );
         }
      }
   }

   void render( const std::string& str, float xx = 0, float yy = 0, float zz = 0, float size = 1.0f, float scalex = 1.0f ) const
   {
#if !TARGET_OS_IPHONE && !defined(EXCLUDE_GLEW)
      glPushMatrix();
      glTranslatef( xx, yy, zz );
      float xpos_iter = 0, ypos_iter = 0;
      for (unsigned int x = 0; x < str.size(); ++x)
      {
         if ('\n' == str[x])
         {
            xpos_iter = 0;
            ++ypos_iter;
            // skip the character, not renderable.
         }
         else if ('\r' == str[x])
         {
            xpos_iter = 0;
            // skip the character, not renderable.
         }
         else
         {
            glBegin( GL_TRIANGLE_STRIP );
               glNormal3f( 0.0f, 0.0f, 1.0f );
               glTexCoord2fv( mMapping[str[x]].t[3].getData() );
               glVertex3f( (float)xpos_iter * size * scalex, (ypos_iter+1.0f) * size, 0.0f );
               glTexCoord2fv( mMapping[str[x]].t[0].getData() );
               glVertex3f( (float)xpos_iter * size * scalex, ypos_iter * size, 0.0f );
               glTexCoord2fv( mMapping[str[x]].t[2].getData() );
               glVertex3f( (float)(xpos_iter+1.0f) * size * scalex, (ypos_iter+1.0f) * size, 0.0f );
               glTexCoord2fv( mMapping[str[x]].t[1].getData() );
               glVertex3f( (float)(xpos_iter+1.0f) * size * scalex, ypos_iter * size, 0.0f );
            glEnd();
            ++xpos_iter;
         }
      }
      glPopMatrix();
#endif
   }

   void renderUpsideDown( const std::string& str, float xx = 0, float yy = 0, float zz = 0, float size = 1.0f, float scalex = 1.0f ) const
   {
#if !TARGET_OS_IPHONE && !defined(EXCLUDE_GLEW)
      glPushMatrix();
      glTranslatef( xx, yy, zz );
      /*
      for (unsigned int x = 0; x < str.size(); ++x)
      {
         glBegin( GL_TRIANGLE_STRIP );
            glNormal3f( 0.0f, 0.0f, 1.0f );
            glTexCoord2fv( mMapping[str[x]].t[2].getData() );
            glVertex3f( (float)(x+1) * size, 0.0f, 0.0f );
            glTexCoord2fv( mMapping[str[x]].t[1].getData() );
            glVertex3f( (float)(x+1) * size, 1.0f * size, 0.0f );
            glTexCoord2fv( mMapping[str[x]].t[3].getData() );
            glVertex3f( (float)x * size, 0.0f, 0.0f );
            glTexCoord2fv( mMapping[str[x]].t[0].getData() );
            glVertex3f( (float)x * size, 1.0f * size, 0.0f );
         glEnd();
      }
      */
      float xpos_iter = 0, ypos_iter = 0;
      for (unsigned int x = 0; x < str.size(); ++x)
      {
         if ('\n' == str[x])
         {
            xpos_iter = 0;
            --ypos_iter;
            // skip the character, not renderable.
         }
         else if ('\r' == str[x])
         {
            xpos_iter = 0;
            // skip the character, not renderable.
         }
         else
         {
            glBegin( GL_TRIANGLE_STRIP );
               glNormal3f( 0.0f, 0.0f, 1.0f );
               glTexCoord2fv( mMapping[str[x]].t[2].getData() );
               glVertex3f( (float)(xpos_iter+1.0f) * size * scalex, ypos_iter * size, 0.0f );
               glTexCoord2fv( mMapping[str[x]].t[1].getData() );
               glVertex3f( (float)(xpos_iter+1.0f) * size * scalex, (ypos_iter+1.0f) * size, 0.0f );
               glTexCoord2fv( mMapping[str[x]].t[3].getData() );
               glVertex3f( (float)xpos_iter * size * scalex, ypos_iter * size, 0.0f );
               glTexCoord2fv( mMapping[str[x]].t[0].getData() );
               glVertex3f( (float)xpos_iter * size * scalex, (ypos_iter+1.0f) * size, 0.0f );
            glEnd();
            ++xpos_iter;
         }
      }
      glPopMatrix();
#endif
   }

	/// has init been called?
	bool isInitialized() const { return mInitialized; }
	//int numGlyphs() const { return mMapping.size(); }

private:
   std::vector<GlyphData> mMapping;
   float                mHeight;
	bool                 mInitialized;
};



#endif
