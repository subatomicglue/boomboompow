#ifndef OGGAUDIOISTREAM
#define OGGAUDIOISTREAM

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
#include "AudioIStream.h"

class OggAudioIStream : public AudioIStream
{
public:
   OggAudioIStream()
   {
      reset();
   }
   OggAudioIStream( const char* const filename )
   {
      reset();
      open( filename );
   }
   virtual ~OggAudioIStream()
   {
      if (good())
      {
         close();
      }
      reset();
   }

   virtual  bool good() const { return fp != NULL; }
   virtual  bool bad() const { return fp == NULL; }
   virtual  int gcount() const { return mGetCount; }
   virtual  int samprate() const { return vi->rate; }
   virtual  int channels() const { return vi->channels; }

   // number of total frames, each frame may have multiple channels (a sample for each channel)
   virtual  int samples() const { return mSamples;}
   virtual  int frames() const { return samples() / channels();}

   virtual double totalTime() { return ov_time_total(&vf,-1); }

   virtual void showinfo()
   {
      printf( "Filename    : \"%s\"\n", m_FileName.c_str() );
      printf( "Sample Rate : %d\n", samprate() );
      printf( "Frames      : %d\n", frames() );
      printf( "Samples     : %d\n", samples() );
      printf( "Channels    : %d\n", channels() );
      printf( "Duration    : %s\n", AudioIStream::generate_duration_str() );

      char **ptr=ov_comment(&vf,-1)->user_comments;
      //vorbis_info *vi=ov_info(&vf,-1);
      while (*ptr)
      {
         printf("User Comment : %s\n", *ptr);
         ++ptr;
      }
      printf( "Encoded by  : %s\n\n", ov_comment(&vf,-1)->vendor );
   }

   virtual bool open( const char* const filename )
   {
      //int ov_test(FILE *f,OggVorbis_File *vf,char *initial,long ibytes);
      fp = fopen( filename, "rb" );
      m_FileName = filename;
	   if (ov_open( fp, &vf, NULL,0) < 0)
	   {
		   std::cout << "file " << filename << " does not appear to be an ogg bitstream\n" << std::endl;
         fclose( fp );
		 fp = NULL;
         return false;
	   }
      vi = ov_info(&vf,-1);
      if (!vi)
	  {
		  fclose( fp );
		  fp = NULL;
		  return false;
	  }
      mSamples = vi->channels * (long)ov_pcm_total(&vf,-1);
      return true;
   }
   virtual void close()
   {
      if (good())
      {
         ov_clear( &vf ); // also closes the file
         reset();
      }
   }

   virtual void read_float( float* buf, int samps )
   {
      int current_section;
      float **pcm = NULL;
      int total_read = 0;
      int amt_wanted = std::min( samps, mSamples ) / channels(); // frames, ogg says samples, but it's frames
      // discussion:  http://lists.xiph.org/pipermail/vorbis-dev/2002-January/014323.html
      while (0 < amt_wanted)
      {
         int amt_read = ov_read_float(&vf, &pcm, amt_wanted, &current_section);
         switch (channels())
         {
            case 1:
               {
                  memcpy( buf, pcm[0], amt_read * sizeof( float ) );
               }
               break;
            case 2:
               {
                //  printf( "1 %d %d %d\n", amt_wanted, amt_read, total_read );
                  float* pcm0 = pcm[0];
                  float* pcm1 = pcm[1];
                  for (int x = 0; x < amt_read; ++x)
                  {
                     buf[x*2] = pcm0[x];
                  }
                  for (int x = 0; x < amt_read; ++x)
                  {
                     buf[x*2 + 1] = pcm1[x];
                  }
               }
               break;
            default:
               {
                  const int num_channels = channels();
                  for (int32_t x = 0; x < amt_read; ++x)
                  {
                     for (int c = 0; c < num_channels; ++c)
                     {
                        buf[x*num_channels + c] = pcm[x][c];
                     }
                  }
               }
               break;
         }
         buf += amt_read;
         total_read += amt_read;
         amt_wanted -= amt_read;
      }
      mGetCount = total_read;
   }
private:
   FILE* fp;
   OggVorbis_File vf;
   vorbis_info* vi;
   int mSamples;
   int mGetCount;
   std::string m_FileName;
	void reset()
   {
      mGetCount = 0;
      mSamples = 0;
      vi = NULL;
      fp = NULL;
      m_FileName = "";
   }
};

#endif
