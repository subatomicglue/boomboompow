#ifndef SNDFILEAUDIOISTREAM
#define SNDFILEAUDIOISTREAM

#include <sndfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <string>

#include "AudioIStream.h"

/// an input stream implementation on top of libsndlib.
/// libsndlib handles wav, aiff, many others...
class SndFileAudioIStream : public AudioIStream
{
   SF_INFO fmt;
   SNDFILE*  sndfile;
   int mGetCount;
   std::string m_FileName;
	
	void reset()
   {
      sndfile = NULL;
      mGetCount = 0;
      m_FileName = "";
   }

public:
   SndFileAudioIStream()
   {
      reset();
   }
   SndFileAudioIStream( const char* const filename )
   {
      reset();
      open( filename );
   }
   virtual ~SndFileAudioIStream()
   {
      if (good())
      {
         close();
      }
      reset();
   }

   virtual bool good() const { return NULL != sndfile; }
   virtual bool bad() const { return NULL == sndfile; }
   virtual int gcount() const { return mGetCount; }
   virtual int samprate() const { return fmt.samplerate; }
   virtual int channels() const { return fmt.channels; }

   // number of total frames, each frame may have multiple channels (a sample for each channel)
   virtual int frames() const { return fmt.frames;}
   virtual int samples() const { return fmt.frames * fmt.channels;}

   virtual void showinfo()
   {
      printf( "Filename    : \"%s\"\n", m_FileName.c_str() );
      printf ("Sample Rate : %d\n", fmt.samplerate);
		if (fmt.frames > 0x7FFFFFFF)
			printf ("Frames      : unknown\n");
		else
			printf ("Frames      : %d\n", frames());
		printf ("Channels    : %d\n", channels());
		printf ("Samples     : %d\n", samples());
		printf ("Format      : 0x%08X\n", fmt.format);
		printf ("Sections    : %d\n", fmt.sections);
		printf ("Seekable    : %s\n", (fmt.seekable ? "TRUE" : "FALSE"));
      printf ("Duration    : %s\n", AudioIStream::generate_duration_str());
   }

   virtual bool open( const char* const filename )
   {
      sndfile = sf_open( filename, SFM_READ, &fmt );
      if (!sndfile)
      {
         std::cout << "file " << filename << " couldn't be read... error: " << sf_strerror(sndfile) << "\n" << std::endl;
         return false;
      }
      m_FileName = filename;
      return true;
   }
   virtual void close()
   {
      if (good())
      {
         sf_close( sndfile );
         reset();
      }
   }

   virtual void read_float( float* buf, int samps )
   {
      mGetCount = sf_read_float( sndfile, buf, samps );
   }
};

#endif
