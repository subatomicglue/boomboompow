#ifndef LEVEL_H
#define LEVEL_H

#include <string>
#include <EASTL/fixed_string.h>
#include <list>
#include <algorithm>
#include "Gfx.h"
#include "Actor.h"
#include "Parser.h"
#include "LevelSchema.h"
#include "FileCache.h"

extern char* LoadScratchFile( const char* filename );

class Level
{
public:
   Level( Gfx::Scene& s ) : mScene( s )
   {
   }

   void graphicsRelease()
   {
      for (std::list<Actor*>::const_iterator ait = mActors.begin();
           ait != mActors.end(); ++ait)
      {
         Actor* a = *ait;
         a->graphicsRelease();
      }
   }
   void release()
   {
      for (std::list<Actor*>::const_iterator ait = mActors.begin();
           ait != mActors.end(); ++ait)
      {
         Actor* a = *ait;
         a->release();
      }
      mActors.clear();
      mNewActors.clear();
   }

   // write
   void serialize( std::string& text )
   {
      // convert Level to LevelDef
      LevelDef levelDef;
      levelDef.name = mName;

      // actors
      for (std::list<Actor*>::const_iterator ait = mActors.begin();
           ait != mActors.end(); ++ait)
      {
         const Actor* const a = *ait;
         levelDef.actors.resize( levelDef.actors.size() + 1 );
         ActorInstanceDef& actorDef = *levelDef.actors.rbegin();

         actorDef.name = a->GetName();
         actorDef.type = a->GetType();
         //actorDef.x = a->mBodyMat;
         //actorDef.y = a->mBodyMat;
      }

      // write LevelDef to file
      printf( "writing level...\n" );
      write( text, levelDef );
   }

   // read
   void deserialize( const std::string& text, FileCache& fc )
   {
      // read LevelDef from file
      //printf( "reading level [" );
      fflush( stdout );
      LevelDef l;
      read( l, text );
      //printf( "%s]\n", l.name.c_str() );

      // convert LevelDef to Level
      mName = l.name;
      for (int x = 0; x < l.actors.size(); ++x)
      {
         const ActorInstanceDef& a = l.actors[x];

         //printf( "   + actor[%d of %lu]... %s\n", x+1, l.actors.size(), a.name.c_str() );
         Actor* actor = new Actor();
         actor->deserialize( fc.GetFile( (a.type + ".actor").c_str() ), fc );
         actor->mName = a.name;
         actor->mID = a.id;
         AddActor( actor );
         //printf( "Level %f %f\n", a.x, a.y );
         actor->mBodyMat = vmMatrix4::identity();
         actor->mBodyMat.setTranslation( vmVec3( a.x, a.y, 0 ) );
      }
   }

   void AddActor( Actor* a )
   {
      if (a)
         mNewActors.push_back( a );
      else
         printf( "NULL actor!\n" );
   }

   void Draw( Ctx& ctx )
   {
      // init any newly added actors, then move to drawlist
      std::list<Actor*>::iterator it = mNewActors.begin();
      while (it != mNewActors.end())
      {
         if (NULL != (*it))
         {
            (*it)->graphicsInit( ctx );
            mActors.push_back( *it );
         }
         else
            printf( "NULL actor!!!\n" );
         ++it;
         mNewActors.pop_front();
      }

      // draw actors
      for (std::list<Actor*>::iterator it = mActors.begin();
            it != mActors.end();
            ++it)
      {
         (*it)->update();
         (*it)->draw( mScene, ctx );
      }
   }

   Actor* Lookup( const char* actor_name )
   {
      std::list<Actor*>::iterator it = mNewActors.begin();
      while (it != mNewActors.end())
      {
         if ((*it)->mName == actor_name)
            return (*it);
         ++it;
      }
      for (std::list<Actor*>::iterator it = mActors.begin();
            it != mActors.end();
            ++it)
      {
         if ((*it)->mName == actor_name)
            return (*it);
      }
      return NULL;
   }

   // removes actor, does not delete or release
   void RemoveActor( Actor* a )
   {
      std::list<Actor*>::iterator it;
      it = std::find( mActors.begin(), mActors.end(), a );
      if (it != mActors.end())
         mActors.erase( it );

      it = std::find( mNewActors.begin(), mNewActors.end(), a );
      if (it != mNewActors.end())
         mNewActors.erase( it );
   }

   std::list<Actor*> mActors;    // after init, actors move to here.
   std::list<Actor*> mNewActors; // put new ones here, for init
   std::string mName;
   Gfx::Scene& mScene;
};

#endif

