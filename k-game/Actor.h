#ifndef ACTOR_H
#define ACTOR_H

#include <string>
#include <EASTL/fixed_string.h>
#include <list>
#include "Parser.h"
#include "ActorSchema.h"
#include "triangulate.h"
#include "Polygon2VertexBuffer.h"
#include "Ctx.h"
#include "FileCache.h"
#include "Mesh.h"

// editors:
//  http://code.google.com/p/box2d-editor
//  http://www.jacobschatz.com/bisonkick-beta/

class Actor
{
public:
   Actor()
   {
      init();
   }

   ~Actor()
   {
   }

   void init()
   {
      mBodyMat = vmMatrix4::identity();
   }
   void graphicsRelease()
   {
      for (std::vector<Graphic*>::iterator it = mGraphics.begin();
           it != mGraphics.end(); ++it)
      {
         (*it)->graphicsRelease();
      }
   }

   void release()
   {
      for (std::vector<Graphic*>::iterator it = mGraphics.begin();
           it != mGraphics.end(); ++it)
      {
         (*it)->release();
         delete *it;
         *it = NULL;
      }
      mGraphics.clear();
   }

   // write
/*
   void serialize( std::string& text )
   {
      // convert Actor to ActorDef
      ActorDef actorDef;
      actorDef.type = mType;

      // bodies
      for (std::list<b2Body*>::const_iterator bit = mBodies.begin();
           bit != mBodies.end(); ++bit)
      {
         const b2Body* const b = *bit;
         actorDef.bodies.resize( actorDef.bodies.size() + 1 );
         BodyDef& bodyDef = actorDef.bodies[actorDef.bodies.size()-1];

         bodyDef.x = b->GetPosition()(0);
         bodyDef.y = b->GetPosition()(1);
         bodyDef.rotation = b->GetAngle() * (180.0f / syn::Math::PI); // to degrees
         assert( b->GetType() == b2_staticBody ||
                 b->GetType() == b2_kinematicBody ||
                 b->GetType() == b2_dynamicBody );
         bodyDef.physicstype = b->GetType() == b2_staticBody ? STATIC :
                               b->GetType() == b2_kinematicBody ? KINEMATIC :
                               DYNAMIC;

         const b2Fixture* fl = b->GetFixtureList();
         while (fl)
         {
            const b2Shape* const sh = fl->GetShape();

            bodyDef.shapes.resize( bodyDef.shapes.size() + 1 );
            ShapeDef& shapeDef = *(bodyDef.shapes.rbegin());
            shapeDef.elasticity = fl->GetRestitution();
            shapeDef.friction = fl->GetFriction();
            shapeDef.density = fl->GetDensity();
            switch (sh->GetType())
            {
               case b2Shape::e_circle:
                  {
                     const b2CircleShape* const cs =
                                             (const b2CircleShape* const)sh;
                     shapeDef.circlex  = cs->m_p(0);
                     shapeDef.circley  = cs->m_p(1);
                     shapeDef.shape  = CIRCLE;
                     shapeDef.width  = sh->m_radius * 2;
                     shapeDef.height = sh->m_radius * 2;
                  }
                  break;
               case b2Shape::e_polygon:
                  {
                     shapeDef.shape  = POLYGON;
                     const b2PolygonShape* const ps =
                                          (const b2PolygonShape* const)sh;
                     shapeDef.verts.reserve( ps->GetVertexCount() );
                     for (int x = 0; x < ps->GetVertexCount(); ++x)
                     {
                        const b2Vec2& v = ps->GetVertex( x );
                        shapeDef.verts.push_back( v(0) );
                        shapeDef.verts.push_back( v(1) );
                     }
                     break;
                  }
                  break;
               default:
                  assert( false && "unknown shape type");
            }
            fl = fl->GetNext();
         }
      }
      // joints
      for (std::list<b2Joint*>::const_iterator jit = mJoints.begin();
           jit != mJoints.end(); ++jit)
      {
         const b2Joint* joint = *jit;
         actorDef.joints.resize( actorDef.joints.size() + 1 );
         JointDef& j = actorDef.joints[actorDef.joints.size()-1];

         switch (joint->GetType() )
         {
            case e_revoluteJoint:
            {
               b2RevoluteJoint* rj = (b2RevoluteJoint*)joint;
               b2Body* bodya = rj->GetBodyA();
               b2Body* bodyb = rj->GetBodyB();
               b2Vec2 anchora = bodya->GetWorldPoint( rj->GetLocalAnchorA() );
               b2Vec2 anchorb = bodyb->GetWorldPoint( rj->GetLocalAnchorB() );
               assert( syn::Math::isEqual( anchora(0), anchorb(0), 0.001f ) &&
                       syn::Math::isEqual( anchora(1), anchorb(1), 0.001f ) );

               j.l0 = j.l1 = -1;
               int count = 0;
               for (std::list<b2Body*>::iterator it = mBodies.begin();
                    it != mBodies.end(); ++it)
               {
                  if (*it == bodya)
                     j.l0 = count;
                  if (*it == bodyb)
                     j.l1 = count;
                  ++count;
               }
               j.type = REVOLUTE;
               j.isLimited = rj->IsLimitEnabled();
               j.anchorx = anchora(0);
               j.anchory = anchora(1);
               j.upperAngle = rj->GetUpperLimit() * (180.0f / syn::Math::PI);
               j.lowerAngle = rj->GetLowerLimit() * (180.0f / syn::Math::PI);
               break;
            }
         }
      }
      // graphics
      for (std::list<Graphic*>::const_iterator git = mGraphics.begin();
           git != mGraphics.end(); ++git)
      {
         Graphic* g = *git;
         actorDef.graphics.resize( actorDef.graphics.size() + 1 );
         GraphicDef& gd = *actorDef.graphics.rbegin();

         int count = 0;
         for (std::list<b2Body*>::iterator it = mBodies.begin();
               it != mBodies.end(); ++it)
         {
            if (*it == g->parentbody)
               gd.parentbody = count;
            ++count;
         }

         //gd.offset[3];
         //gd.scale[2];
         //gd.rot;
         gd.texture = g->textureFile;
         gd.vshader = g->vshaderFile;
         gd.fshader = g->fshaderFile;
         gd.verts = g->polygonVerts;
         gd.tverts = g->polygonTVerts;
         gd.show = g->show;
      }

      // write a default graphic for bodies that dont have one
      int count = 0;
      for (std::list<b2Body*>::const_iterator it = mBodies.begin();
               it != mBodies.end(); ++it)
      {
         const b2Body* body = *it;
         bool found = false;
         for (std::list<Graphic*>::const_iterator git = mGraphics.begin();
            git != mGraphics.end(); ++git)
         {
            Graphic* g = *git;
            if (g->parentbody == body)
            {
               found = true;
            }
         }

         if (!found)
         {
            actorDef.graphics.resize( actorDef.graphics.size() + 1 );
            GraphicDef& gd = *actorDef.graphics.rbegin();
            gd.parentbody = count;
            gd.show = true;//->show;
            gd.texture = "../../mantis.tga";
            gd.vshader = "../../tex.vshader";
            gd.fshader = "../../tex.fshader";
            std::vector<float> verts;
            body2verts( verts, body );
            gd.verts = verts;
            gd.tverts = verts;
         }
         ++count;
      }

      // write ActorDef to file
      printf( "writing actor...\n" );
      write( text, actorDef );
   }
*/

   // read
   void deserialize( const std::string& text, FileCache& fc )
   {
      // read ActorDef from file
      //printf( "reading actor [" );
      fflush( stdout );
      ActorDef a;
      read( a, text );
      //printf( "%s]\n", a.type.c_str() );

      // convert ActorDef to Actor
      mType = a.type;
      mIsUI = a.ui;
      for (int x = 0; x < a.graphics.size(); ++x)
      {
         //printf( "   + graphic...\n" );
         GraphicDef& gd = a.graphics[x];
         int count = 0;
         Graphic* g = new Graphic( this );
         if (gd.vshader != "")
            g->vshader = fc.GetFile( gd.vshader.c_str() );
         if (gd.fshader != "")
            g->fshader = fc.GetFile( gd.fshader.c_str() );

         g->show = gd.show;
         g->vshaderFile = gd.vshader;
         g->fshaderFile = gd.fshader;
         g->textureFile = gd.texture;

         if (gd.texture != "")
         {
            printf( "loading: %s\n", gd.texture.c_str() );
            g->image = fc.GetImageFile( gd.texture.c_str() );
            fflush( stdout );
         }
         g->mesh.mMaterial.mOpts[0] = Material::OPT_SRCDESTBLEND_RGBA;
         //g->mesh.mMaterial.mOpts[1] = Material::OPT_DEPTH;
         g->mesh.mMaterial.mOpts[1] = Material::OPT_END;
         g->mesh.mMaterial.mSrcBlendOp = Material::SRCBLEND_SRC_ALPHA;
         g->mesh.mMaterial.mDstBlendOp = Material::DSTBLEND_ONE_MINUS_SRC_ALPHA;
         g->mesh.mShaderVars.scalars["selected"] = &mSelected;

         g->SetPolygon( gd.verts, gd.tverts );

         mGraphics.push_back( g );
      }
   }

   struct Graphic
   {
      // data
      const corona::Image* image;
      const char* vshader, *fshader;
      std::vector<float> polygonVerts;
      std::vector<float> polygonTVerts;
      Actor* actor;

      // original filenames used to find data:
      std::string vshaderFile, fshaderFile, textureFile;

      // derived data
      Mesh mesh;
      bool hasInit;
      bool show;

      void release()
      {
         polygonVerts.clear();
         polygonTVerts.clear();
         actor = NULL; // ref to parent
         fshader = NULL; // was from filecache
         vshader = NULL; // was from filecache
         image = NULL;
      }

      Graphic( Actor* a ) : hasInit(false), image( NULL ),
                  vshader( NULL ), fshader( NULL ), actor( a ), show( true ) {}

      void SetPolygon( const std::vector<float >& verts,
                       const std::vector<float >& tverts )
      {
         assert( verts.size() != 0 && verts.size() == tverts.size() );

         // convert polygon to vertex buffer for rendering
         static const FvF fvf[] = {
            {FvF::VERTEX,   3, GL_FLOAT,               0, 12 * sizeof( float )},
            {FvF::COLOR,    4, GL_FLOAT, 3*sizeof(float), 12 * sizeof( float )},
            {FvF::TEXCOORD, 2, GL_FLOAT, 7*sizeof(float), 12 * sizeof( float )},
            {FvF::NORMAL,   3, GL_FLOAT, 9*sizeof(float), 12 * sizeof( float )},
            {FvF::END, 0, 0, 0, 0} };
         std::vector<char> vb;
         Polygon2VertexBuffer( vb, fvf, verts, tverts );
         mesh.SetMesh( &vb[0], vb.size(), fvf );

         // save polygon for editing later
         polygonVerts = verts;
         polygonTVerts = tverts;
      }

      void graphicsInit( Ctx& ctx )
      {
         if (mesh.mDatasize == 0) return;

         hasInit = true;

         if (strcmp( vshader, "" ) != 0 && strcmp( fshader, "" ) != 0)
         {
            mesh.SetShaderProgram( ctx.GetShaderProgram( vshader, fshader ) );
         }

         if (image && image->getWidth() != 0 && image->getHeight() != 0)
         {
            mesh.SetNumTextures( 1 );
            mesh.SetTexture( ctx.GetTexture( textureFile ), 0 );
         }
         else
         {
            mesh.SetNumTextures( 0 );
         }
         mesh.initGraphics();
      }
      void update()
      {
         mesh.SetXform( actor->mBodyMat );
      }
      void draw( Gfx::Scene& scene )
      {
         assert( hasInit );

         if (show)
         {
            // submit to GPU
            mesh.Draw( scene );
         }
      }
      void graphicsRelease()
      {
         mesh.releaseGraphics();
      }
   };
   void graphicsInit( Ctx& ctx )
   {
      for (int x = 0; x < mGraphics.size(); ++x)
      {
         mGraphics[x]->graphicsInit( ctx );
      }
   }
   void update()
   {
      for (std::vector<Graphic*>::iterator it = mGraphics.begin();
           it != mGraphics.end(); ++it)
         (*it)->update();
   }
   void draw( Gfx::Scene& scene, Ctx& ctx )
   {
      for (std::vector<Graphic*>::iterator it = mGraphics.begin();
           it != mGraphics.end(); ++it)
         (*it)->draw( scene );
   }
   void showAllGraphics( bool state )
   {
      for (std::vector<Graphic*>::iterator it = mGraphics.begin();
           it != mGraphics.end(); ++it)
         (*it)->show = state;
   }

   void GetBoundingBox( float b1[2], float b2[2] )
   {
      b1[0] = 99999999.0f;
      b1[1] = 99999999.0f;
      b2[0] = -99999999.0f;
      b2[1] = -99999999.0f;
      //vmVec4 v(0,0,0,1);
      //vmVec4 position = mBodyMat * v;
      for (int x = 0; x < mGraphics.size(); ++x)
      {
         Graphic* g = mGraphics[x];
         for (int y = 0; y < g->polygonVerts.size(); y += 2)
         {
            vmVec4 pos;
            pos[0] = g->polygonVerts[y + 0];
            pos[1] = g->polygonVerts[y + 1];
            pos[2] = 0.0f;
            pos[3] = 1.0f;
            pos = mBodyMat * pos;
            if (pos[0] < b1[0]) b1[0] = pos[0];
            if (pos[1] < b1[1]) b1[1] = pos[1];
            if (b2[0] < pos[0]) b2[0] = pos[0];
            if (b2[1] < pos[1]) b2[1] = pos[1];
         }
      }
      //printf( "b1 %f %f b2 %f %f\n", b1[0], b1[1], b2[0], b2[1] );
   }

   void ShowGraphic( size_t which, bool state = true )
   {
      if (which < mGraphics.size())
      {
         mGraphics[which]->show = state;
      }
      else if (0 < mGraphics.size())
      {
         mGraphics[mGraphics.size()-1]->show = state;
      }
   }

   const std::string& GetType() const { return mType; }
   const std::string& GetName() const { return mName; }

   // state:
   std::string mType;
   int mID;
   bool mIsUI; // does it respond to mouse
   std::string mName; // instance name
   std::vector<Graphic*> mGraphics;
   vmMatrix4 mBodyMat;
   float mSelected;
};

#endif

