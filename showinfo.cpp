

#include "SndFileAudioIStream.h"
#include "MpegAudioIStream.h"
#include "OggAudioIStream.h"

#include "gio.h"
#include "assert.h"

int main( int argc, char* argv[] )
{
   AudioIStream* s = NULL;

   std::string ext = gio::tolower( gio::justExt( argv[1] ) );
   printf( "%s\n", ext.c_str() );
   if (ext == "wav")
   {
      s = new SndFileAudioIStream( argv[1] );
   }
   else if (ext == "mp3")
   {
      s = new MpegAudioIStream( argv[1] );
   }
   else if (ext == "ogg")
   {
      s = new OggAudioIStream( argv[1] );
   }
   if (s)
   {
       s->showinfo();
       printf( "reading %d samples\n", s->samples() );
       float* wavdata = new float[s->samples()];
       s->read_float( wavdata, s->frames() );
       size_t wavsamps = s->samples();
       assert( s->gcount() != 0 );
       printf( "loaded...\n" );
       //s->close();
   }
   else
   {
      printf( "unknown extension\n" );
   }
   return 1;
}

