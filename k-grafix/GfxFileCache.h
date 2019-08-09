#ifndef GFX_FILE_CACHE_H
#define GFX_FILE_CACHE_H

#include <map>
#include <string>
#include "Gfx.h"
#include "FileCache.h"

class GfxFileCache : public FileCache
{
public:
   GfxFileCache() : FileCache()
   {
   }

   // cache from an image
   // must call within a graphics context
   Gfx::Texture& GetTexture( const std::string& s, const corona::Image& i,
                             bool reload = false )
   {
      if (!reload && texturecache.count( s ))
         return texturecache.find( s )->second;
      else
      {
         if (texturecache.count( s ))
         {
            Gfx::DeleteTexture( texturecache[s] );
         }
         texturecache[s] = Gfx::NullTexture();
         texturecache[s] = Gfx::CreateTexture(
                  i.getWidth(),
                  i.getHeight(),
                  i.getFormat() == corona::PF_R8G8B8A8 ? Gfx::RGBA : Gfx::RGB,
                  i.getFormat() == corona::PF_R8G8B8A8 ? 4 : 3,
                  (char*)i.getPixels(), false );
         return texturecache[s];
      }
   }

   // cache from a file
   // must call within a graphics context
   Gfx::Texture& GetTexture( const std::string& s, bool reload = false )
   {
      if (!reload && texturecache.count( s ))
         return texturecache.find( s )->second;
      else
      {
         corona::Image* i = FileCache::GetImageFile( s, reload );
         return GetTexture( s, *i, reload );
      }
   }

   // cache from a vertex shader, and a fragment shader
   // must call within a graphics context
   Gfx::ShaderProgram& GetShaderProgram( const std::string& vshader,
                                         const std::string& fshader,
                                         bool reload = false )
   {
      std::string s = vshader + fshader;
      if (!reload && shadercache.count( s ))
         return shadercache.find( s )->second;
      else
      {
         if (shadercache.count( s ))
         {
            Gfx::DeleteShaderProgram( shadercache[s] );
         }
         shadercache[s] = Gfx::CompileAndLinkShaderProgram( vshader.c_str(),
                                                            fshader.c_str() );
         return shadercache[s];
      }
   }

   // must call within a graphics context
   void graphicsRelease()
   {
      {
         std::map<std::string, Gfx::Texture>::iterator it;
         for (it = texturecache.begin(); it != texturecache.end(); ++it)
         {
            Gfx::DeleteTexture( it->second );
         }
      }
      {
         std::map<std::string, Gfx::ShaderProgram>::iterator it;
         for (it = shadercache.begin(); it != shadercache.end(); ++it)
         {
            Gfx::DeleteShaderProgram( it->second );
         }
      }
      shadercache.clear();
      texturecache.clear();
   }

private:
   std::map<std::string, Gfx::ShaderProgram> shadercache;
   std::map<std::string, Gfx::Texture> texturecache;
};

#endif


