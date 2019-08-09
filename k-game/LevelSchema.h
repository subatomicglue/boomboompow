#ifndef LEVEL_SCHEMA
#define LEVEL_SCHEMA

#include <vector>


struct ActorInstanceDef {
    ActorInstanceDef() : id(-1) {}
    std::string type;
    int id;
    std::string name;
    float x;
    float y;
};

struct LevelDef {
    std::string name;
    std::vector<ActorInstanceDef >  actors;
};


void read( LevelDef& a, const std::string& text )
{
   std::string name;
   bool found;
   int pos = 0;

   name = p::NextName( pos, text );
   if (name == "level")
   {
      found = p::NextChar( "{", pos, text );
      do
      {
         name = p::NextName( pos, text );
         if (name == "name") a.name = p::NextName( pos, text );
         else if (name == "actor")
         {
            a.actors.resize( a.actors.size() + 1 );
            ActorInstanceDef& d = *a.actors.rbegin();

            found = p::NextChar( "{", pos, text );
            do
            {
               name = p::NextName( pos, text );
               if (name == "x") d.x = p::NextFloat( pos, text );
               else if (name == "y") d.y = p::NextFloat( pos, text );
               else if (name == "name") d.name = p::NextName( pos, text );
               else if (name == "type") d.type = p::NextName( pos, text );
               else if (name == "id") d.id = p::NextInt( pos, text );
               else
               {
                  printf( "3 name not known: \"%s\"\n", name.c_str() );
                  assert( false );
               }
               p::Eat( ";", pos, text );
            } while (!p::NextChar( "}", pos, text ));
         }
         else
         {
            printf( "2 name not known: \"%s\"\n", name.c_str() );
            assert( false );
         }
         p::Eat( ";", pos, text );
      } while (!p::NextChar( "}", pos, text ));
   }
   else
   {
      printf( "1 name not known: \"%s\"\n", name.c_str() );
      assert( false );
   }
}

void write( std::string& text, const LevelDef& a )
{
   text += "level\n{\n";
   text += "   name ";
   text += a.name;
   text += ";\n";
   for (int x = 0; x < a.actors.size(); ++x)
   {
      const ActorInstanceDef& b = a.actors[x];
      text += "   actor { ";
      text += "x "; text += p::ToString( b.x ); text += "; ";
      text += "y "; text += p::ToString( b.y ); text += "; ";
      text += "name "; text += p::ToString( b.name ); text += "; ";
      text += "type "; text += "copy-" + p::ToString( b.type ); text += "; ";
      text += "id "; text += p::ToString( b.id ); text += "; ";
      text += "}\n";
   }
   text += "}\n";
}

#endif

