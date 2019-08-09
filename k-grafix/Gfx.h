
#ifndef GFX_ABSTRACTION_H
#define GFX_ABSTRACTION_H

#include "SynMath.h"

//#define GFX_MASSIVE_ASSERTS

// Flexible Vertex Format descriptor
// for arrays of FvF, end is reached when eletype == END
// it is generally best if all of the components of a vertex format
// are aligned to 4 bytes.
struct FvF
{
   // to add a new variable:
   // - add to ElementType enum, AND getShaderVariable function
   // - keep the sequence of integers.
   enum ElementType
   {START=0,VERTEX=0,COLOR=1,TEXCOORD=2,NORMAL=3,INDEX=4,END=5 };
   static const char* getShaderVariable( size_t t )
   {
      const char* names[] =
      {"InVertex","InColor","InTexCoord0","InNormal","InIndex"};
      return names[t];
   }

   ElementType ele;
   size_t cnt;        // number of elements (s,t = 2, x,y,z == 3, r,g,b,a == 4)
   GLint type;       // GL_FLOAT
   size_t start;      // starting place in the buffer for the data type (bytes)
   GLint stride;     // distance between in bytes
};
// ret -1 if not found
inline int FindIndex( FvF::ElementType et, const FvF* fvf )
{
   for (int index = 0; fvf->ele != FvF::END; ++index)
   {
      if (fvf[index].ele == et)
         return index;
   }
   return -1;
}

// Cross Platform Rendering Abstraction
// the idea with Gfx is a thin (functional) platform agnostic rendering layer
// at least, we're going that direction...
struct Gfx
{
   static int numBytesInType( GLenum type )
   {
      switch (type)
      {
         case GL_FLOAT: return sizeof( float );
         default: assert( false ); // todo: fill in other types here.
      }
   }

   static GLenum gl_tex( unsigned int i )
   {
      static const GLenum a[] = {GL_TEXTURE0, GL_TEXTURE1, GL_TEXTURE2,
                                 GL_TEXTURE3, GL_TEXTURE4, GL_TEXTURE5,
                                 GL_TEXTURE6, GL_TEXTURE7, GL_TEXTURE8,
      GL_TEXTURE9, GL_TEXTURE10, GL_TEXTURE11, GL_TEXTURE12,
      GL_TEXTURE13, GL_TEXTURE14, GL_TEXTURE15, GL_TEXTURE16,
      GL_TEXTURE17, GL_TEXTURE18, GL_TEXTURE19, GL_TEXTURE20,
      GL_TEXTURE21, GL_TEXTURE22, GL_TEXTURE23, GL_TEXTURE24};
      assert( i < 24 );
      return a[i];
   }
   static void ActiveTexture( unsigned int i )
   { glActiveTexture( Gfx::gl_tex( i ) ); }

   typedef GLuint Texture;
   enum PixFormat { RGB = GL_RGB, RGBA = GL_RGBA, UNKNOWN };
   static Texture CreateTexture( size_t width, size_t height, PixFormat pf, int chan, const void* data, bool repeat=true )
   {
      GLuint tex;
      glGenTextures(1, &tex);
      glBindTexture(GL_TEXTURE_2D, tex);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeat?GL_REPEAT:GL_CLAMP_TO_EDGE);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeat?GL_REPEAT:GL_CLAMP_TO_EDGE);
      glTexImage2D(GL_TEXTURE_2D,0,pf,
                   width,height,0,pf,
                   GL_UNSIGNED_BYTE,(GLvoid*)data);
      return tex;
   }

   static void SetTexture( Texture tex, unsigned int unit = 0 )
   {
#ifdef GFX_MASSIVE_ASSERTS
      assert( GL_TRUE == ::glIsTexture(tex) );
#endif
      //glEnable( GL_TEXTURE_2D );
      Gfx::ActiveTexture( unit );
      glBindTexture(GL_TEXTURE_2D, tex);
   }
   static void ClearTexture( unsigned int unit = 0 )
   {
      Gfx::ActiveTexture( unit );
      glBindTexture(GL_TEXTURE_2D, 0);
   }

   static void DeleteTexture( Texture& tex )
   {
#ifdef GFX_MASSIVE_ASSERTS
      assert( GL_TRUE == ::glIsTexture(tex) );
#endif
      ::glDeleteTextures( 1, (GLuint*)&tex );
      tex = 0;
   }

   static Texture NullTexture() { return 0; }


   typedef GLuint VertexShader;
   typedef GLuint PixelShader;

   // Create a shader object, load the shader source, and compile the shader.
   static GLuint CompileShader(GLenum type, const char *shaderSrc)
   {
      GLuint shader;
      GLint compiled;
      // Create the shader object
      shader = glCreateShader( type );
#ifdef GFX_MASSIVE_ASSERTS
      assert( GL_TRUE == ::glIsShader(shader) );
#endif
      if(shader == 0)
         return 0;
      // Load the shader source
      glShaderSource( shader, 1, &shaderSrc, NULL );
      //printf( "Shader Source Code: %s\n", shaderSrc );

      // Compile the shader
      glCompileShader( shader );
      // Check the compile status
      glGetShaderiv( shader, GL_COMPILE_STATUS, &compiled );
      if(!compiled)
      {
         GLint infoLen = 0;
         glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &infoLen );
         if (infoLen > 1)
         {
            eastl::vector<char> infoLog;
            infoLog.resize( infoLen );
            glGetShaderInfoLog( shader, infoLog.size(), NULL, &infoLog[0] );
            printf( "Error compiling shader:\n\n\ttype:%s\n\treason:\n\t\t\"%s\"\n\n", GL_VERTEX_SHADER == type ? "vertex" : GL_FRAGMENT_SHADER == type ? "fragment" : "unknown", &infoLog[0] );
         }
         glDeleteShader( shader );
         return 0;
      }
#ifdef GFX_MASSIVE_ASSERTS
      assert( GL_TRUE == ::glIsShader(shader) );
#endif
      return shader;
   }
   static VertexShader CompileVertexShader(const char *shaderSrc)
   {
      return CompileShader(GL_VERTEX_SHADER, shaderSrc);
   }
   static PixelShader CompilePixelShader(const char *shaderSrc)
   {
      return CompileShader(GL_FRAGMENT_SHADER, shaderSrc);
   }

   typedef GLuint ShaderProgram; // both vertex and fragment shaders in one
   static ShaderProgram CompileAndLinkShaderProgram( const char* vShaderStr,
                                                     const char* fShaderStr )
   {
      GLuint vertexShader;
      GLuint fragmentShader;
      GLuint programObject;
      GLint linked;

      // Load the vertex/fragment shaders
      vertexShader   = Gfx::CompileVertexShader(vShaderStr);
      fragmentShader = Gfx::CompilePixelShader(fShaderStr);
#ifdef GFX_MASSIVE_ASSERTS
      assert( GL_TRUE == ::glIsShader(vertexShader) );
      assert( GL_TRUE == ::glIsShader(fragmentShader) );
#endif
      // Create the program object
      programObject = glCreateProgram();
#ifdef GFX_MASSIVE_ASSERTS
      assert( GL_TRUE == ::glIsProgram(programObject) );
#endif
      if(programObject == 0)
         return 0;
      glAttachShader(programObject, vertexShader);
      glAttachShader(programObject, fragmentShader);

      for (size_t x = FvF::START; x < FvF::END; ++x)
      {
         glBindAttribLocation( programObject, x, FvF::getShaderVariable( x ) );
      }

      //vertexLoc = glGetAttribLocation(MyShader, "InVertex");
      //texCoord0Loc = glGetAttribLocation(MyShader, "InTexCoord0");
      //normalLoc = glGetAttribLocation(MyShader, "InNormal");
      // others...
      ///// loc = glGetUniformLocationARB(p,"time"); // init()
      ///// glUniform1fARB(loc, time); // draw()

      // Link the program
      glLinkProgram(programObject);
      // Check the link status
      glGetProgramiv(programObject, GL_LINK_STATUS, &linked);
      if(!linked)
      {
         GLint infoLen = 0;
         glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLen);
         if(infoLen > 1)
         {
            std::vector<char> infoLog;
            infoLog.resize( infoLen );
            glGetProgramInfoLog( programObject, infoLog.size(), NULL, &infoLog[0] );
            printf( "Error linking shader program:\n%s\n", &infoLog[0] );
         }
         glDeleteProgram(programObject);
         return 0;
      }

#ifdef GFX_MASSIVE_ASSERTS
      assert( GL_TRUE == ::glIsProgram(programObject) );
#endif
      // todo: leak in vertexShader / fragmentShader??...
      return programObject;
   }

   static void SetShaderProgram( ShaderProgram p )
   {
#ifdef GFX_MASSIVE_ASSERTS
      assert( GL_TRUE == ::glIsProgram(p) );
#endif
      ::glUseProgram( p );
   }

   static ShaderProgram NullShaderProgram() { return 0; }

   inline static bool IsNullShaderProgram( ShaderProgram p ) { return p == 0; }

   static void DeleteShaderProgram( ShaderProgram& p )
   {
#ifdef GFX_MASSIVE_ASSERTS
      assert( GL_TRUE == ::glIsProgram(p) );
#endif
      ::glDeleteShader( p );
      p = 0;
   }

   typedef GLuint VertexBuffer;
   static VertexBuffer CreateVertexArray( const void* data, size_t size,
                                          const FvF* fvf )
   {
      VertexBuffer vbo = 0;
      // create VBO, activate VBO, then upload data to VBO
      glGenBuffers( 1, &vbo );
#ifdef GFX_MASSIVE_ASSERTS
      //assert( GL_TRUE == ::glIsBuffer(vbo) );
#endif
      glBindBuffer( GL_ARRAY_BUFFER, vbo );
      glBufferData( GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW );
      //printf( "Creating Vertex Buffer Object: %u\n", vbo );
/*
      glBindBuffer( GL_ARRAY_BUFFER, vbo );
      // specify the vertex format in a flexible fashion
      // (this is way more flexible than glInterleavedArrays, and just as fast)
      const FvF* fvfIt = fvf;
      while (fvfIt->ele != FvF::END)
      {
         glEnableVertexAttribArray( fvfIt->ele );
         glVertexAttribPointer( fvfIt->ele,  fvfIt->cnt, fvfIt->type, GL_FALSE,
                  fvfIt->stride,
                  (GLvoid*)fvfIt->start );
         ++fvfIt;
      }
*/
      // don't leave anything active...  deactivate the VBO
      glBindBuffer(GL_ARRAY_BUFFER, 0);
#ifdef GFX_MASSIVE_ASSERTS
      assert( GL_TRUE == ::glIsBuffer(vbo) );
#endif
      return vbo;
   }
   static void DeleteVertexArray( VertexBuffer vbo )
   {
#ifdef GFX_MASSIVE_ASSERTS
      assert( GL_TRUE == ::glIsBuffer(vbo) );
#endif
      glDeleteBuffers( 1, &vbo );
   }

   // immediately draw the data, reupload the data every time
   // index data is optional
   static void DrawVertexArrayI( const char* data, size_t size,
                                 const FvF* fvf,
                                 const char* idata = 0, size_t isize = 0,
                                 uint16_t bytesPerIndex = 2 )
   {
      // pull from RAM, not gfx memory
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

      // specify the vertex format in a flexible fashion
      // (this is way more flexible than glInterleavedArrays, and just as fast)
      const FvF* fvfIt = fvf;
      FvF::ElementType ele = fvf->ele;
      int numtexcoordchannels = 0;
      int vertex_size = 0;
      while (ele != FvF::END)
      {
         glEnableVertexAttribArray( ele );
         glVertexAttribPointer( fvfIt->ele,  fvfIt->cnt, fvfIt->type, GL_FALSE,
                  fvfIt->stride,
                  data + fvfIt->start );
         vertex_size += fvfIt->cnt * Gfx::numBytesInType( fvfIt->type );
         ++fvfIt;
         ele = fvfIt->ele;
      }

      size_t num_elements = idata != NULL ? isize/bytesPerIndex : size/vertex_size;
      const int type[] = { 0, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT,
                           0, GL_UNSIGNED_INT };
      if (idata)
         glDrawElements( GL_TRIANGLES, num_elements, type[bytesPerIndex], idata);
      else
         glDrawArrays( GL_TRIANGLES, 0, num_elements );


      fvfIt = fvf;
      ele = fvfIt->ele;
      while (ele != FvF::END)
      {
         glDisableVertexAttribArray(ele);
         ++fvfIt;
         ele = fvfIt->ele;
      }
      /*
      GLint MaxVertexAttribs = 0;
      glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &MaxVertexAttribs);
      for (int x = 0; x <MaxVertexAttribs; ++x)
      {
         glDisableVertexAttribArray(x);
      }*/
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   }

   typedef GLuint IndexBuffer;
   static IndexBuffer CreateIndexArray( const void* data, size_t size )
   {
      VertexBuffer ibo = 0;
      // create IBO, activate IBO, then upload data to IBO
      glGenBuffers( 1, &ibo );
#ifdef GFX_MASSIVE_ASSERTS
      assert( GL_TRUE == ::glIsBuffer(ibo) );
#endif
      glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );
      glBufferData( GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW );
      //printf( "Creating New Index Buffer Object: %u\n", ibo );

      // don't leave anything active...  deactivate the IBO
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#ifdef GFX_MASSIVE_ASSERTS
      assert( GL_TRUE == ::glIsBuffer(ibo) );
#endif
      return ibo;
   }
   static void DeleteIndexArray( IndexBuffer ibo )
   {
#ifdef GFX_MASSIVE_ASSERTS
      assert( GL_TRUE == ::glIsBuffer(ibo) );
#endif
      glDeleteBuffers( 1, &ibo );
   }

   // http://www.opengl.org/wiki/VBO_-_just_examples
   static void DrawVertexArray( VertexBuffer vb, size_t num_elements,
            const FvF* fvf, IndexBuffer ib, bool useibo = false,
            uint16_t bytesPerIndex = 2 )
   {
#ifdef GFX_MASSIVE_ASSERTS
      assert( GL_TRUE == ::glIsBuffer(vb) );
#endif
#ifdef DEBUG
      if ( GL_FALSE == glIsBuffer( vb ) )
      {
         printf( "error: vertex buffer object %u is invalid\n", vb );
         return;
      }
      if ( useibo && GL_FALSE == glIsBuffer( ib ) )
      {
         printf( "error: index buffer object %u is invalid\n", ib );
         return;
      }
#endif

      GLint MaxVertexAttribs = 0;
      glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &MaxVertexAttribs);
      for (int x = 0; x <MaxVertexAttribs; ++x)
      {
         glDisableVertexAttribArray(x);
      }
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

      glBindBuffer( GL_ARRAY_BUFFER, vb );
      register const FvF* fvfIt = fvf;
      register FvF vf = *fvf;
      while (vf.ele != FvF::END)
      {
         glEnableVertexAttribArray( vf.ele );
         glVertexAttribPointer( vf.ele, vf.cnt, vf.type, GL_FALSE,
                                vf.stride, (GLvoid*)vf.start );
         ++fvfIt;
         vf = *fvfIt; // precache in local register
      }

      if (useibo)
      {
         //printf( "d vbo: %u, ", vb );
         //printf( "d ibo: %u\n", ib );
         glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ib );
         const int type[] = { 0, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT,
                              0, GL_UNSIGNED_INT };
         glDrawElements( GL_TRIANGLES, num_elements, type[bytesPerIndex], 0 );
         glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
      }
      else
      {
         assert( 0 < num_elements );
         //printf( "num_elements %d\n", num_elements );
         glDrawArrays( GL_TRIANGLES, 0, num_elements );
         //printf( "num_elements %d\n", num_elements );
      }
      glBindBuffer( GL_ARRAY_BUFFER, 0 );

      fvfIt = fvf;
      int ele = fvf->ele;
      while (ele != FvF::END)
      {
         glDisableVertexAttribArray( ele );
         ++fvfIt;
         ele = fvfIt->ele; // precache
      }
   }

   static bool IsNullVertexBuffer( VertexBuffer vbo ) { return vbo == 0; }
   static bool IsNullIndexBuffer( IndexBuffer ibo ) { return ibo == 0; }
   static VertexBuffer NullVertexBuffer() { return 0; }
   static IndexBuffer NullIndexBuffer() { return 0; }

   // call Draw to commit the Scene state
   class Scene
   {
   public:
      vmMatrix4 cam2proj;
      vmMatrix4 world2cam;
      vmMatrix4 local2world;
      vmMatrix4 local2cam; // generated
      vmMatrix4 world2proj; // generated
      vmMatrix4 local2proj; // generated
      vmMatrix4 proj2world; // generated

      // typically set once per rendertarget
      void SetCameraToProj( const vmMatrix4& c2p )
      {
         //printf( "SetCameraToProj: " );
         //print( c2p );
         cam2proj = c2p;
         world2proj = c2p * world2cam;
         local2proj = world2proj * local2world;
         proj2world = inverse( world2proj );
      }

      // typically set once per frame per rendertarget
      void SetWorldToCamera( const vmMatrix4& w2c )
      {
         //printf( "SetWorldToCamera: " );
         //print( w2c );
         world2cam = w2c;
         local2cam = w2c * local2world;
         world2proj = cam2proj * w2c;
         local2proj = world2proj * local2world;
         proj2world = inverse( world2proj );
      }

      // typically set once per mesh draw
      void SetLocalToWorld( const vmMatrix4& l2w )
      {
         //printf( "SetLocalToWorld: " );
         //print( l2w );
         local2world = l2w;
         local2cam = world2cam * l2w;
         local2proj = world2proj * l2w;
      }

      // update shader program with data
      // shader should be part of current state with glUseProgram()
      // before calling Draw()
      void Draw( ShaderProgram p )
      {
         /*
         glMatrixMode( GL_PROJECTION );
         glLoadMatrixf( (float*)&cam2proj );
         glMatrixMode( GL_MODELVIEW );
         glLoadMatrixf( (float*)&local2cam );
         */

         // support fixed func
         //glMatrixMode( GL_PROJECTION );
         //glLoadMatrixf( (float*)&world2proj );
         //glMatrixMode( GL_MODELVIEW );
         //glLoadMatrixf( (float*)&local2world );

         // shader constants:
         GLint loc;
         loc = glGetUniformLocation(p,"cam2proj");
         if (loc != -1)
         {
            glUniformMatrix4fv( loc, 1, GL_FALSE, (float*)&cam2proj );
         }

         loc = glGetUniformLocation(p,"world2cam");
         if (loc != -1)
         {
            glUniformMatrix4fv( loc, 1, GL_FALSE, (float*)&world2cam );
         }

         loc = glGetUniformLocation(p,"local2world");
         if (loc != -1)
         {
            glUniformMatrix4fv( loc, 1, GL_FALSE, (float*)&local2world );
         }

         loc = glGetUniformLocation(p,"local2proj");
         if (loc != -1)
         {
            glUniformMatrix4fv( loc, 1, GL_FALSE, (float*)&local2proj );
         }

         loc = glGetUniformLocation(p,"proj2world");
         if (loc != -1)
         {
            glUniformMatrix4fv( loc, 1, GL_FALSE, (float*)&proj2world );
         }
      }
   };
};


#endif
