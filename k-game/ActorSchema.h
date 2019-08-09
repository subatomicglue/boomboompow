#ifndef ACTOR_SCHEMA
#define ACTOR_SCHEMA

#include <vector>

enum ShapeTypeDef {
    RECTANGLE,
    CIRCLE,
    POLYGON,
};
namespace p
{
   ShapeTypeDef NextShapeTypeDef( int& pos, const std::string& text )
   {
      std::string s = p::NextName( pos, text );
      if (s == "RECTANGLE") return RECTANGLE;
      else if (s == "CIRCLE") return CIRCLE;
      else if (s == "POLYGON") return POLYGON;
      assert( false );
      return RECTANGLE;
   }
   std::string ToString( ShapeTypeDef s )
   {
      switch (s)
      {
         case RECTANGLE: return "RECTANGLE"; break;
         case CIRCLE: return "CIRCLE"; break;
         case POLYGON: return "POLYGON"; break;
         default: assert( false ); return "POLYGON";
      }
   }
} // parser

enum PhysicsTypeDef {
    STATIC,
    KINEMATIC,
    DYNAMIC,
};
namespace p
{
   PhysicsTypeDef NextPhysicsTypeDef( int& pos, const std::string& text )
   {
      std::string s = p::NextName( pos, text );
      if (s == "DYNAMIC") return DYNAMIC;
      else if (s == "KINEMATIC") return KINEMATIC;
      else if (s == "STATIC") return STATIC;
      assert( false );
      return STATIC;
   }
   std::string ToString( PhysicsTypeDef s )
   {
      switch (s)
      {
         case STATIC: return "STATIC"; break;
         case KINEMATIC: return "KINEMATIC"; break;
         case DYNAMIC: return "DYNAMIC"; break;
         default: assert( false ); return "STATIC";
      }
   }
} // parser

enum JointTypeDef {
    REVOLUTE,
};
namespace p
{
   JointTypeDef NextJointTypeDef( int& pos, const std::string& text )
   {
      std::string s = p::NextName( pos, text );
      if (s == "REVOLUTE") return REVOLUTE;
      assert( false );
      return REVOLUTE;
   }
   std::string ToString( JointTypeDef s )
   {
      switch (s)
      {
         case REVOLUTE: return "REVOLUTE"; break;
         default: assert( false ); return "REVOLUTE";
      }
   }
} // parser

struct GraphicDef {
   GraphicDef() : show( true ) {}
   bool show;
   p::FileName texture;
   p::FileName vshader;
   p::FileName fshader;
   std::vector<float > verts;
   std::vector<float > tverts;
   p::FileName mesh;
};

struct ActorDef {
    ActorDef() : ui( true ) {}
    std::string type;
    bool ui; // is it UI?  is it meant to respond to mouse...
    std::vector<GraphicDef >  graphics;
};


void read( ActorDef& a, const std::string& text )
{
   std::string name;
   bool found;
   int pos = 0;

   name = p::NextName( pos, text );
   if (name == "actor")
   {
      found = p::NextChar( "{", pos, text );
      do
      {
         name = p::NextName( pos, text );
         if (name == "type") a.type = p::NextName( pos, text );
         else if (name == "ui") a.ui = p::NextBool( pos, text );
         else if (name == "graphic")
         {
            a.graphics.resize( a.graphics.size() + 1 );
            GraphicDef& d = *a.graphics.rbegin();

            found = p::NextChar( "{", pos, text );
            do
            {
               name = p::NextName( pos, text );
               if (name == "mesh") d.mesh = p::NextFilename( pos, text );
               else if (name == "show") d.show = p::NextBool( pos, text );
               else if (name == "texture") d.texture = p::NextFilename( pos, text );
               else if (name == "vshader") d.vshader = p::NextFilename( pos, text );
               else if (name == "fshader") d.fshader = p::NextFilename( pos, text );
               else if (name == "verts")
               {
                  while (!p::NextChar( ";", pos, text ))
                  {
                     float f = p::NextFloat( pos, text );
                     p::NextChar( ",", pos, text );
                     d.verts.push_back( f );
                  }
               }
               else if (name == "tverts")
               {
                  while (!p::NextChar( ";", pos, text ))
                  {
                     float f = p::NextFloat( pos, text );
                     p::NextChar( ",", pos, text );
                     d.tverts.push_back( f );
                  }
               }
               else
               {
                  printf( "name not known: '%s'\n", name.c_str() );
                  assert( false );
               }
               p::Eat( ";", pos, text );
            } while (!p::NextChar( "}", pos, text ));
         }
         else
         {
            printf( "name not known: %s\n", name.c_str() );
            assert( false );
         }
         p::Eat( ";", pos, text );
      } while (!p::NextChar( "}", pos, text ));
   }
}

void write( std::string& text, const ActorDef& a )
{
   text += "actor\n{\n";
   text += "   type ";
   text += "   ui "; text += p::ToString( a.ui ); text += ";\n";
   text += a.type;
   text += ";\n";
   for (int y = 0; y < a.graphics.size(); ++y)
   {
      const GraphicDef& d = a.graphics[y];
      text += "   graphic\n   {\n";
      text += "      show "; text += p::ToString( d.show ); text += ";\n";
      text += "      mesh "; text += p::ToString( d.mesh ); text += ";\n";
      text += "      texture "; text += p::ToString( d.texture ); text += ";\n";
      text += "      vshader "; text += p::ToString( d.vshader ); text += ";\n";
      text += "      fshader "; text += p::ToString( d.fshader ); text += ";\n";
      text += "      verts ";
         for (int z = 0; z < d.verts.size(); ++z)
         {
            if (0 < z) text += ",";
            text += p::ToString( d.verts[z] );
         }
         text += ";\n";
      text += "      tverts ";
         for (int z = 0; z < d.tverts.size(); ++z)
         {
            if (0 < z) text += ",";
            text += p::ToString( d.tverts[z] );
         }
         text += ";\n";
      text += "   }\n";
   }
   text += "}\n";
}

#endif

