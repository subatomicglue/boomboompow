#ifndef DATACACHE_H
#define DATACACHE_H

#include <map>
#include <string>
#include <vector>
#include <corona/corona.h>
#include <string.h> // strcasecmp

class FileCache
{
public:
   FileCache()
   {
      if (imagecache.count( "default" ))
      {
         static unsigned char data[4*4*3] = {0xff,0xff,0xff,0x0,0x0,0x0,0xff,0xff,0xff,0x0,0x0,0x0,0x0,0x0,0x0,0xff,0xff,0xff,0x0,0x0,0x0,0xff,0xff,0xff,0xff,0xff,0xff,0x0,0x0,0x0,0xff,0xff,0xff,0x0,0x0,0x0,0x0,0x0,0x0,0xff,0xff,0xff,0x0,0x0,0x0,0xff,0xff,0xff,};
         imagecache["default"] = corona::CreateImage( 4, 4, corona::PF_R8G8B8, data );
      }
   }
   const char* GetFile( const std::string& s, bool reload = false )
   {
      if (!reload && filecache.count( s ))
         return &filecache.find( s )->second[0];
      else
      {
         std::vector<char>& data = filecache[s];
         data.clear();
         size_t size = gio::computeFileSize( s.c_str() ) + 1; // for \0
         //printf( "size: %lu\n", size );
         data.resize( size );
         return gio::readFile( &data[0], data.size(), s.c_str() );
      }
   }

   corona::Image* GetImageFile( const std::string& s, bool reload = false )
   {
      if (!reload && imagecache.count( s ))
         return imagecache.find( s )->second;
      else
      {
         if (imagecache.count( s ))
         {
            corona::Image* &i = imagecache.find( s )->second;
            delete i;
            i = NULL;
         }

         if (gio::fileExists( s.c_str() ))
         {
            corona::Image* &image = imagecache[s];
            corona::Image* i = corona::OpenImage( s.c_str(),
                                                  corona::PF_R8G8B8A8,
                                                  corona::FF_AUTODETECT );

            if (i == NULL)
            {
               printf( "FileCache::GetImageFile() ERROR: image '%s' not found, if it is a PNG, see if xcode is compressing them in the build settings. (dont use PNG with apps on iOS, use jpg).  using 'default' image\n", s.c_str() );
               return imagecache["default"];
            }
            image = i;
            //printf( "corona image format- %d\n", image->getFormat() );
            return image;
         }
         else
         {
            printf( "FileCache::GetImageFile() ERROR: file '%s' not found, using 'default' image\n", s.c_str() );
            return imagecache["default"];
         }
      }
      return NULL;
   }

   void release()
   {
      {
         std::map<std::string, corona::Image*>::iterator it;
         for (it = imagecache.begin(); it != imagecache.end(); ++it)
         {
            delete it->second;
            it->second = NULL;
         }
      }
      filecache.clear();
      imagecache.clear();
   }

private:
   std::map<std::string, std::vector<char> > filecache;
   std::map<std::string, corona::Image*> imagecache;
};

#endif

