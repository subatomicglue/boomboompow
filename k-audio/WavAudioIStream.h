
/****************** <SYN heading BEGIN do not edit this line> *****************
 *
 * subsynth - modular audio synthesizer
 * subsynth is (C) Copyright 2001-2002 by Kevin Meinert
 *
 * Original Author: Kevin Meinert
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * -----------------------------------------------------------------
 * File:          $RCSfile: WavAudioIStream.h,v $
 * Date modified: $Date: 2002/02/16 19:24:16 $
 * Version:       $Revision: 1.19 $
 * -----------------------------------------------------------------
 *
 ****************** <SYN heading END do not edit this line> ******************/
#ifndef SUBSYNTH_WAV_INPUT_STREAM
#define SUBSYNTH_WAV_INPUT_STREAM

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "AudioIStream.h"

class WavHeader
{
	public:
	char riff[4]; // 'RIFF'
	unsigned int riffSize;
	char wave[4]; // 'WAVE'

	// WAVE chunk 1
	char fmt[4]; // 'fmt '
	unsigned int fmtSize;
	unsigned short Format;
	unsigned short Channels;
	unsigned int SamplesPerSec;
	unsigned int BytesPerSec;
	unsigned short BlockAlign;
	unsigned short BitsPerSample;
	
	// if fmtSize == 16, then it is PCM,
	// if fmtSize > 16, there are extra params:
	// unsigned short extraParamSize;
	// char bytes[extraParamSize]
	
	// WAVE chunk 2
	char data[4]; // 'data'
	unsigned int dataSize;
};

/** 
   * stream for .wav files
   */
class WavAudioIStream : public AudioIStream
{
public:
   WavAudioIStream();
   WavAudioIStream( const char* const filename )
   {
      open( filename );
   }

   virtual ~WavAudioIStream(){}

   /**
      * open the stream
      * specify the audio format you would like returned
      * (independant of file attributes)
      */
   virtual bool open( const char* const filename );

   virtual bool good() const;
   virtual bool bad() const;
   virtual int samprate() const { return mSamplingRateHz; }
   virtual int channels() const { return mChannels; }
   virtual int samples() const { return mNumBytes / mBytesPerSample; }
   virtual int frames() const { return samples() / mChannels; }
   virtual void showinfo()
   {
      printf( "Filename    : \"%s\"\n", mFilename.c_str() );
      printf ("Sample Rate : %d\n", mSamplingRateHz);
		printf ("Frames      : %d\n", frames());
		printf ("Channels    : %d\n", channels());
		printf ("Samples     : %d\n", samples());
   }


   /**
      * read data out of the stream in the format you specified.
      * returns this->gcount(); 
      * samples are in terms of [channels * bytes_per_samp]
      */
   virtual void read_float( float* buf, int samples );

   virtual int gcount() const;

   /**
      * close the stream...
      */
   void close();


private:
   std::vector<unsigned char> actual_data;
   std::vector<float> float_data;
   mutable std::fstream mFile; // mutable cause irix's is_open() is non-const.
   std::string mFilename;
   std::streamoff mDataPosition;
   int mNumBytes;
   int mChannels;
   int mSamplingRateHz;
   int mBytesPerSample;
   int actual_samples_read;

   /** @link dependency */
   /*#  WavHeader lnkWavHeader; */
};

#endif

