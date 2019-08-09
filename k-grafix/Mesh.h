#ifndef MESH_H
#define MESH_H

#include "Material.h"
#include "Gfx.h"

class Mesh
{
public:
   Mesh() : mData( NULL ), mIndexData( NULL ), mNumTex( 0 )
   {
      mDatasize = 0;
      mIndexDatasize = 0;
      mLocalToWorld = vmMatrix4::identity();
      mVbo = Gfx::NullVertexBuffer();
      mIbo = Gfx::NullVertexBuffer();
   }
   ~Mesh()
   {
      delete [] mData; mData = NULL;
      delete [] mIndexData; mIndexData = NULL;
   }
   enum { MAX_FVF = 16, MAX_TEX = 8 };

   void SetShaderProgram( Gfx::ShaderProgram p ) { mShaderProgram = p; }

   void SetNumTextures( size_t n )
   {
      assert( n < MAX_TEX );
      mNumTex = n;
   }

   void SetTexture( Gfx::Texture t, size_t unit )
   {
      assert( unit < mNumTex && unit < MAX_TEX );
      mTexture[unit] = t;
   }


   void SetXform( const vmMatrix4& mat )
   {
      mLocalToWorld = mat;
   }

   void InitMesh( size_t datasize, size_t indexsize = 0 )
   {
      assert( datasize != 0 );
      delete [] mData;
      mData = new char[datasize];
      mDatasize = datasize;
      if (indexsize)
      {
         delete [] mIndexData;
         mIndexData = new char[indexsize];
         mIndexDatasize = indexsize;
      }
   }

   // size = num bytes in data
   // makes copy of data internal
   void SetMesh( const void* data, size_t size,
                  const FvF* fvf, const void* indexdata = NULL,
                  size_t indexsize = 0, uint16_t bytesPerIndex = 2 )
   {
      // set data
      delete [] mData;
      mData = new char[size];
      memcpy( mData, data, size );
      mDatasize = size;
      //printf( "size=%lu\n", mDatasize );
      if (indexdata)
      {
         delete [] mIndexData;
         mIndexData = new char[indexsize];
         memcpy( mIndexData, indexdata, indexsize );
         mIndexDatasize = indexsize;
      }
      mBytesPerIndex = bytesPerIndex;
      mUseIbo = indexdata != NULL;

      // make a copy of the fvf for later
      const FvF* fvfIt = fvf;
      int fvfCnt = 0;
      int vertex_size = 0;
      while (fvfIt->ele != FvF::END)
      {
         vertex_size += fvfIt->cnt * Gfx::numBytesInType( fvfIt->type );
         mFvf[fvfCnt++] = *(fvfIt++); // copy fvf formatting
      }
      mFvf[fvfCnt] = *fvfIt; // copy END
      assert( fvfCnt < MAX_FVF );

      mNumElements = indexdata != NULL ? indexsize/bytesPerIndex : size/vertex_size;
      //printf( "%sMesh with %ld verts\n", indexdata != NULL ? "Indexed" : "", mNumElements );
   }

   // needed for Draw (not DrawDyn)
   bool initGraphics()
   {
      if (!Gfx::IsNullVertexBuffer( mVbo ))
      {
         releaseGraphics();
      }

      // greate GPU objects
      mVbo = Gfx::CreateVertexArray( mData, mDatasize, mFvf );
      if (Gfx::IsNullVertexBuffer( mVbo )) return false;
      if (mIndexData)
      {
         mIbo = Gfx::CreateIndexArray( mIndexData, mIndexDatasize );
         if (Gfx::IsNullIndexBuffer( mIbo )) return false;
      }
      return true;
   }

   // needed for Draw (not DrawDyn)
   void releaseGraphics()
   {
      // greate GPU objects
      Gfx::DeleteVertexArray( mVbo );
      if (mIndexData)
      {
         Gfx::DeleteIndexArray( mIbo );
      }
      mVbo = Gfx::NullVertexBuffer();
      mIbo = Gfx::NullIndexBuffer();
   }

   // submit draw comands to GPU
   void Draw( Gfx::Scene& scene )
   {
      mMaterial.draw();                   // state/blend/mode changes

      for (size_t unit = 0; unit < mNumTex; ++unit)
      {
         Gfx::SetTexture( mTexture[unit], unit );
      }
      Gfx::SetShaderProgram( mShaderProgram );

      scene.SetLocalToWorld( mLocalToWorld );
      scene.Draw( mShaderProgram );       // global shader vars
      mShaderVars.draw( mShaderProgram ); // local shadervars

      Gfx::DrawVertexArray( mVbo, mNumElements, mFvf, mIbo, mUseIbo,
                            mBytesPerIndex );

      /*
      int fvfV = FindIndex( FvF::VERTEX, mFvf ),
             fvfC = FindIndex( FvF::COLOR, mFvf ),
             fvfT = FindIndex( FvF::TEXCOORD, mFvf ),
             fvfN = FindIndex( FvF::NORMAL, mFvf );
      char* d = mData;
      for (int x = 0; x < mNumElements; ++x)
      {
         float* vert     = (float*)&d[x * mFvf[fvfV].stride + mFvf[fvfV].start];
         float* color    = (float*)&d[x * mFvf[fvfC].stride + mFvf[fvfC].start];
         float* texcoord = (float*)&d[x * mFvf[fvfT].stride + mFvf[fvfT].start];
         float* normal   = (float*)&d[x * mFvf[fvfN].stride + mFvf[fvfN].start];

         glVertex3f( vert[x], vert[x+1], vert[x+2] );
         glColor4f( color[x], color[x+1], color[x+2], color[x+3] );
         glTexCoord2f( texcoord[x], texcoord[x+1] );
         glNormal3f( normal[x], normal[x+1], normal[x+2] );
      }
      */

      //for (size_t unit = 0; unit < mNumTex; ++unit)
      //{
      //   Gfx::ClearTexture( unit );
      //}

      mMaterial.undraw();
   }

   // draw dynamic changing data - for when we expect data to change every call.
   // do not call initGraphics/releaseGraphics/Draw when using this
   void DrawDyn( Gfx::Scene& scene )
   {
      mMaterial.draw();                   // state/blend/mode changes
      for (size_t unit = 0; unit < mNumTex; ++unit)
      {
         Gfx::SetTexture( mTexture[unit], unit );
      }
      Gfx::SetShaderProgram( mShaderProgram );
      scene.SetLocalToWorld( mLocalToWorld );
      scene.Draw( mShaderProgram );       // global shader vars
      mShaderVars.draw( mShaderProgram ); // local shadervars
      Gfx::DrawVertexArrayI( mData, mDatasize, mFvf, mIndexData, mIndexDatasize );
      mMaterial.undraw();
   }

   void DebugMesh()
   {
      for (size_t unit = 0; unit < mNumTex; ++unit)
      {
         Gfx::SetTexture( mTexture[unit], unit );
      }
      Gfx::SetShaderProgram( mShaderProgram );

#if !defined(EXCLUDE_GLEW) || defined(USE_OLD_OPENGL)
      glBegin( GL_LINE_LOOP );
         glVertex3f( 0.0f, 0.0f, 0.0f );
         glColor3f( 1,1,1 );
         glVertex3f( 1.0f, 0.0f, 0.0f );
         glColor3f( 1,1,1 );
         glVertex3f( 1.0f, 1.0f, 0.0f );
         glColor3f( 1,1,1 );
         glVertex3f( 0.0f, 1.0f, 0.0f );
         glColor3f( 1,1,1 );
      glEnd();
#endif
      /*
      glFrontFace(GL_CW);
         glColor3f( 0.5,0.5,1.0 );
         glutSolidTeapot( 0.1f );
      glFrontFace(GL_CCW);
      */
   }

   void setShaderVar( const char* varname, float* data )
   {
      mShaderVars.scalars[varname] = data;
   }
   void setShaderVar( const char* varname, vmMatrix4* data )
   {
      mShaderVars.mats[varname] = data;
   }
   void setShaderVar( const char* varname, vmVec4* data )
   {
      mShaderVars.vecs[varname] = data;
   }

   vmMatrix4 mLocalToWorld;
   char* mIndexData;
   size_t mIndexDatasize;
   char* mData;
   size_t mDatasize;
   size_t mNumElements;
   Gfx::VertexBuffer mVbo;
   Gfx::IndexBuffer mIbo;
   bool mUseIbo;
   FvF mFvf[MAX_FVF];
   uint16_t mBytesPerIndex;

   Gfx::ShaderProgram mShaderProgram;
   Gfx::Texture mTexture[MAX_TEX];
   Material mMaterial;
   ShaderVars mShaderVars;
   size_t mNumTex;
};

#endif

