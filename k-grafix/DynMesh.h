#ifndef DYN_MESH_INCLUDED_H
#define DYN_MESH_INCLUDED_H

#include <assert.h>
#include <vector>
#include <string>

#include <EASTL/vector.h>
#include <EASTL/map.h>

#include "Mesh.h"

   struct MeshWithDefaults : public Mesh
   {
      Gfx::ShaderProgram mShader;
      char* mVshader;
      char* mFshader;
      Gfx::Scene* mScene;

      MeshWithDefaults()
      {
         mShader = Gfx::NullShaderProgram();
         setToDefaultShaders();
         setToDefaultGeometry( 0,0, 200, 200 );
         setToDefaultScene( 0,0, 200, 200 );
         setToDefaultMaterial();
         setToDefaultTextures();
         Mesh::SetXform( vmMatrix4::identity() );
      }

      static char* defaultVShader()
      {
         static char Vshader[] = "\
attribute vec3 InVertex;\n\
attribute vec2 InTexCoord0;\n\
attribute vec4 InColor;\n\
attribute vec3 InNormal;\n\
uniform mat4 local2proj;\n\
\n\
void main()\n\
{\n\
      gl_Position = local2proj * vec4(InVertex,1.0);\n\
      gl_TexCoord[0]  = InTexCoord0.stst;\n\
      gl_FrontColor = InColor;\n\
      /*gl_BackColor = InColor * 0.2;*/\n\
}\n";
         return Vshader;
      }
      static char* defaultFShader()
      {
         static char Fshader[] = "\
uniform sampler2D tex;\n\
void main()\n\
{\n\
   /*gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);*/\n\
   gl_FragColor = gl_Color * texture2D(tex, gl_TexCoord[0].st );\n\
   /*gl_FragColor = gl_Color*/;\n\
   /*gl_FragColor = texture2D(tex, gl_TexCoord[0].st );*/ \n\
}\n";
         return Fshader;
      }

      static void defaultGeometry( float* &ret_data, int& ret_size,
                                   FvF* &ret_fvf,
                            float t, float l, float b, float r,
                            float* tc, float* bc )
      {
         const float d[12*6] = {
                                 r,t,0.0, tc[0],tc[1],tc[2],tc[3], 1,0, 0,0,1,
                                 r,b,0.0, bc[0],bc[1],bc[2],bc[3], 1,1, 0,0,1,
                                 l,b,0.0, bc[0],bc[1],bc[2],bc[3], 0,1, 0,0,1,
         /* l/t */               l,t,0.0, tc[0],tc[1],tc[2],tc[3], 0,0, 0,0,1,
         /* r/t */               r,t,0.0, tc[0],tc[1],tc[2],tc[3], 1,0, 0,0,1,
         /* l/b */               l,b,0.0, bc[0],bc[1],bc[2],bc[3], 0,1, 0,0,1,
                              };
         static float data[sizeof(d)/sizeof(float)];
         memcpy( data, d, sizeof( d ) );

         static FvF datafvf[] = {
            {FvF::VERTEX,   3, GL_FLOAT,               0, 12 * sizeof( float )},
            {FvF::COLOR,    4, GL_FLOAT, 3*sizeof(float), 12 * sizeof( float )},
            {FvF::TEXCOORD, 2, GL_FLOAT, 7*sizeof(float), 12 * sizeof( float )},
            {FvF::NORMAL,   3, GL_FLOAT, 9*sizeof(float), 12 * sizeof( float )},
            {FvF::END, 0, 0, 0, 0} };

         ret_data = data;
         ret_size = sizeof( data );
         ret_fvf = datafvf;
      }

      static Gfx::Scene& defaultScene( float t, float l, float b, float r )
      {
         static Gfx::Scene scene;
         vmMatrix4 world2cam;
         world2cam = vmMatrix4::identity();
                     //vmMatrix4::translation( vmVec3( 0,0,-25.0f ) ) *
                     //vmMatrix4::rotation( syn::Math::PI / 180.0f,
                     //                        vmVec3( 0,1,0 ) );

         // 0,0 at top left of the screen, just like photoshop
         vmMatrix4 uiLocalToProj;
         uiLocalToProj = vmMatrix4::orthographic( l, r, b, t, -100.0f, 100.0f );
         //vmMatrix4 cam2proj;
         //cam2proj = vmMatrix4::perspective( 70.0f * syn::Math::PI / 180.0f,
         //                                    aspect, 0.1f, 10000.0f );

         scene.SetCameraToProj( uiLocalToProj );//cam2proj );
         scene.SetWorldToCamera( world2cam );
         scene.SetLocalToWorld( vmMatrix4::identity() );
         //vmMatrix4 mGameProjToWorld = mScene.proj2world; // for mouse interaction screenspace to ui space
         return scene;
      }

      static Material& defaultMaterial()
      {
         static Material material;
         material.mOpts[0] = Material::OPT_SRCDESTBLEND_RGBA;
         material.mOpts[1] = Material::OPT_DEPTH;
         material.mOpts[1] = Material::OPT_END;
         material.mSrcBlendOp = Material::SRCBLEND_SRC_ALPHA;
         material.mDstBlendOp = Material::DSTBLEND_ONE_MINUS_SRC_ALPHA;
         return material;
      }

      static void defaultTexture( int& ret_x, int& ret_y,
                                 Gfx::PixFormat& ret_pf, int& ret_chan,
                                 unsigned char* &ret_data )
      {
         static unsigned char data[4*4*3] = {0xff,0xff,0xff,0x0,0x0,0x0,0xff,0xff,0xff,0x0,0x0,0x0,0x0,0x0,0x0,0xff,0xff,0xff,0x0,0x0,0x0,0xff,0xff,0xff,0xff,0xff,0xff,0x0,0x0,0x0,0xff,0xff,0xff,0x0,0x0,0x0,0x0,0x0,0x0,0xff,0xff,0xff,0x0,0x0,0x0,0xff,0xff,0xff,};
         /*
         for (int x = 0; x < 4; ++x)
            for (int y = 0; y < 4; ++y)
            {
               data[x*3 + y*4*3 + 0] = ((x+y)%2 == 0) ? 255 : 0;
               data[x*3 + y*4*3 + 1] = ((x+y)%2 == 0) ? 255 : 0;
               data[x*3 + y*4*3 + 2] = ((x+y)%2 == 0) ? 255 : 0;
               printf( "0x%x,", (unsigned)data[x*3 + y*4*3 + 0] );
               printf( "0x%x,", (unsigned)data[x*3 + y*4*3 + 1] );
               printf( "0x%x,", (unsigned)data[x*3 + y*4*3 + 2] );
            }
         */
         ret_data = data;
         ret_x = 4;
         ret_y = 4;
         ret_pf = Gfx::RGB;
         ret_chan = 3;
      }


      void setToDefaultGeometry( float t, float l, float b, float r )
      {
         float tc[]={1.0,0.0,0.0,1.0};
         float bc[]={0.2,0.2,1.0,1.0};
         float *ret_data;
         int ret_size;
         FvF *ret_fvf;
         defaultGeometry( ret_data, ret_size, ret_fvf, t, l, b, r, tc, bc );
         Mesh::SetMesh( ret_data, ret_size, ret_fvf );
      }

      void setToDefaultScene( float t, float l, float b, float r )
      {
         mScene = &defaultScene( t, l, b, r );
      }

      void setToDefaultShaders()
      {
         mVshader = defaultVShader();
         mFshader = defaultFShader();
      }
      void setToDefaultMaterial()
      {
         Mesh::mMaterial = defaultMaterial();
      }

      void setToDefaultTextures()
      {
         int x, y, ch;
         unsigned char* d;
         Gfx::PixFormat pf;
         defaultTexture( x, y, pf, ch, d );
         static Gfx::Texture tex = Gfx::CreateTexture( x, y, pf, ch, d );
         Mesh::SetNumTextures( 1 );
         Mesh::SetTexture( tex, 0 );
      }

      void initGraphics()
      {
         if (Gfx::IsNullShaderProgram( mShader ))
            Gfx::DeleteShaderProgram( mShader );
         mShader = Gfx::CompileAndLinkShaderProgram( mVshader, mFshader );
         Mesh::SetShaderProgram( mShader );

         if (!Mesh::initGraphics())
         {
            printf( "ouch, StaticDrawTest()::initGraphics()'s mesh.initGraphics() failed\n" );
         }
      }

      // call initGraphics to upload this to graphics memory
      void setMesh( char* data, size_t size, const FvF* fvf,
                     const void* indexdata = NULL,
                     size_t indexsize = 0, uint16_t bytesPerIndex = 2 )
      {
         Mesh::SetMesh( data, size, fvf, indexdata, indexsize, bytesPerIndex );
      }

      void draw()
      {
         Mesh::Draw( *mScene );
      }

      static void test()
      {
         static MeshWithDefaults sd;
         static bool once = true;
         if (once)
         {
            sd.initGraphics();
            once = false;
         }
         sd.draw();
      }
   };

struct DynMesh : public MeshWithDefaults
{
      int mDynIndex;
      DynMesh() : MeshWithDefaults()
      {
         InitMesh( 1024*1024 );
         mData[mDatasize-1] = 0x69;
         mDynIndex = 0;
      }

      void begin()
      {
         assert( mData[mDatasize-1] == 0x69 );
      }

      inline void operator()( float x, float y, float z, float w )
      {
         ((float*)&mData[mDynIndex])[0] = x;
         ((float*)&mData[mDynIndex])[1] = y;
         ((float*)&mData[mDynIndex])[2] = z;
         ((float*)&mData[mDynIndex])[3] = w;
         mDynIndex += 4 * sizeof(float);
      }
      inline void operator()( float x, float y, float z )
      {
         ((float*)&mData[mDynIndex])[0] = x;
         ((float*)&mData[mDynIndex])[1] = y;
         ((float*)&mData[mDynIndex])[2] = z;
         mDynIndex += 3 * sizeof(float);
      }
      inline void operator()( float x, float y )
      {
         ((float*)&mData[mDynIndex])[0] = x;
         ((float*)&mData[mDynIndex])[1] = y;
         mDynIndex += 2 * sizeof(float);
      }

      void end()
      {
         assert( mData[mDatasize-1] == 0x69 );
      }

      static void test()
      {
         static DynMesh dd;
         dd.initGraphics();
         float t=0,l=0,b=200,r=200;
         float tc[]={1.0,0.0,1.0,1.0};
         float bc[]={0.2,0.2,1.0,1.0};
         dd.begin();
            // pos, color, texcoord, normal
            dd( r,t,0 ); dd( tc[0],tc[1],tc[2],tc[3] ); dd( 1,0 ); dd( 0,0,1 );
            dd( r,b,0 ); dd( bc[0],bc[1],bc[2],bc[3] ); dd( 1,1 ); dd( 0,0,1 );
            dd( l,b,0 ); dd( bc[0],bc[1],bc[2],bc[3] ); dd( 0,1 ); dd( 0,0,1 );
            dd( l,t,0 ); dd( tc[0],tc[1],tc[2],tc[3] ); dd( 0,0 ); dd( 0,0,1 );
            dd( r,t,0 ); dd( tc[0],tc[1],tc[2],tc[3] ); dd( 1,0 ); dd( 0,0,1 );
            dd( l,b,0 ); dd( bc[0],bc[1],bc[2],bc[3] ); dd( 0,1 ); dd( 0,0,1 );
         dd.end();
         dd.draw();
      }

      void draw()
      {
         assert( mData[mDatasize-1] == 0x69 ); // overwrote end

         // upload the new mesh
         size_t old = mDatasize;
         mDatasize = mDynIndex;
         initGraphics();

         // draw the new mesh
         Mesh::DrawDyn( *mScene );
         mDatasize = old;

         // reset
         mDynIndex = 0;
      }
};


#endif

