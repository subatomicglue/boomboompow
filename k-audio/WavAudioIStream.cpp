
#include <assert.h>
#include "WavAudioIStream.h"

   template< class Type >
   inline static void  byteReverse(Type& bytes)
   {
      const int size = sizeof(Type);
      Type buf = 0;
      int x, y;

      //we want to index the bytes in the buffer
      unsigned char* buffer = (unsigned char*) &buf;

      for ( x = 0, y = size-1; 
         x < size;
         ++x, --y )
      {
         buffer[x] |= ((unsigned char*)&bytes)[y];
      }
      bytes = buf;
   }

   enum Endianness
   {
      BIG, LITTLE
   };

   //: check the system for endianess
   inline bool static isLittle() 
   {
      union 
      {
         short   val;
         char    ch[sizeof(short)];
      } un;

      // initialize the memory that we'll probe
      un.val = 256;       // 0x10

      // If the 1 byte is in the low-order position (un.ch[1]), this is a
      // little-endian system.  Otherwise, it is a big-endian system.
      return un.ch[1] == 1;
   }

   //: check the system for endianess
   inline bool static isBig()
   {
      return !isLittle();
   }

   template< class Type >
   inline void  static byteReverse( const Endianness& e, Type& bytes )
   {
      if (e == BIG && isLittle())
         byteReverse( bytes );
      if (e == LITTLE && isBig())
         byteReverse( bytes );
   }


WavAudioIStream::WavAudioIStream() : mFilename( "filename_not_set.wav" )
{
   actual_samples_read = 0;
}


/**
 * open the stream
 * specify the audio format you would like returned
 * (independant of file attributes)
 */
bool WavAudioIStream::open( const char* const filename )
{
   WavHeader wave;

   mFile.open( filename, std::ios::in | std::ios::binary );

   if (mFile.bad())
   {
      std::cout<<"Failed to open "<<filename<<"\n"<<std::flush;
      mFile.close();
      return false;
   }

   // read header:
   mFile.read( (char*)&wave.riff, 4 );
   if (0 == ::strncmp( "RIFX", wave.riff, 4))
   {
      std::cout << "big endian 'RIFX' type wave files are not supported" << std::endl;
      mFile.close();
      return false;
   }
   if (0 != ::strncmp( "RIFF", wave.riff, 4))
   {
      std::string temp; temp.copy( wave.riff, 4 );
      std::cout << "unsupported wave file format, expected 'RIFF', found: " << temp << std::endl;
      mFile.close();
      return false;
   }
   mFile.read( (char*)&wave.riffSize, 4 );
   byteReverse( LITTLE, wave.riffSize );
   mFile.read( (char*)&wave.wave, 4 );
   if (0 != ::strncmp( "WAVE", wave.wave, 4))
   {
      std::string temp; temp.copy( wave.wave, 4 );
      std::cout << "unsupported wave file format, expected 'WAVE', found: " << temp << std::endl;
      mFile.close();
      return false;
   }

   // WAVE chunk 1 (fmt )
   mFile.read( (char*)&wave.fmt, 4 );
   if (0 != ::strncmp( "fmt ", wave.fmt, 4))
   {
      std::string temp; temp.copy( wave.fmt, 4 );
      std::cout << "unsupported chunk in wave file, expected 'fmt ', found: " << temp << std::endl;
      mFile.close();
      return false;
   }

   mFile.read( (char*)&wave.fmtSize, 4 );
   byteReverse( LITTLE, wave.fmtSize );

   mFile.read( (char*)&wave.Format, 2 );
   byteReverse( LITTLE, wave.Format );

   mFile.read( (char*)&wave.Channels, 2 );
   byteReverse( LITTLE, wave.Channels );

   mFile.read( (char*)&wave.SamplesPerSec, 4 );
   byteReverse( LITTLE, wave.SamplesPerSec );

   mFile.read( (char*)&wave.BytesPerSec, 4 );
   byteReverse( LITTLE, wave.BytesPerSec );

   mFile.read( (char*)&wave.BlockAlign, 2 );
   byteReverse( LITTLE, wave.BlockAlign );

   mFile.read( (char*)&wave.BitsPerSample, 2 );
   byteReverse( LITTLE, wave.BitsPerSample );

   if (wave.fmtSize > 16)
   {
      //std::cout << "Warning: WAVE file may not be PCM data, loader may not get data correctly" << std::endl;

      // read the extra params...
      unsigned short extraParamSize;
      mFile.read( (char*)&extraParamSize, 2 );
      byteReverse( LITTLE, extraParamSize );
   
      std::vector<char> temp;
      temp.resize( extraParamSize );
      mFile.read( (char*)&temp[0], extraParamSize );
   }
   
   // WAVE chunk 2 (data)
   mFile.read( (char*)&wave.data, 4 );
   if (0 != ::strncmp( "data", wave.data, 4))
   {
      std::string temp; temp.copy( wave.data, 4 );
      std::cout << "unsupported chunk in wave file, expected 'data', found: " << temp << std::endl;
      mFile.close();
      return false;
   }
   
   // 8-bit samples are stored as unsigned bytes, ranging from 0 to 255. 
   // 16-bit samples are stored as 2's-complement signed integers, 
   //        ranging from -32768 to 32767.
   mFile.read( (char*)&wave.dataSize, 4 );
   byteReverse( LITTLE, wave.dataSize );

   if (mFile.eof() == true || mFile.bad() == true)
   {
      std::cout<<"Invalid wave file\n"<<std::flush;
      mFile.close();
      return false;
   }

   // save the streampos at the beginning of the data...
   mDataPosition = mFile.tellp();

   // save the important data read from the header...
   mChannels = wave.Channels;
   mSamplingRateHz = wave.SamplesPerSec;
   mBytesPerSample = wave.BitsPerSample / 8;
   printf( "%d\n", wave.BitsPerSample );
   // 1 == signed8 or 2 == signed16
   mNumBytes = wave.dataSize;

   // make sure everything is sane, then return.
   assert( mFile.bad() == false ); // to record a loss of integrity of the stream buffer
   assert( mFile.fail() == false ); // to record a failure to extract a valid field from a stream
   assert( mFile.eof() == false ); // to record end-of-file while extracting from a stream
   assert( mFile.good() == true ); // no bits set
   assert( mFile.is_open() == true ); // we didn't accidentally close it.
   return true;
}

void WavAudioIStream::read_float( float* data, int samples )
{
   //memset( data, 0, samples * sizeof( float ) );
   actual_samples_read = 0;
   for (int x = 0; x  < samples && !mFile.eof(); ++x)
   {
      if (mBytesPerSample == 1)
      {
         char samp;
         mFile.read( (char*)&samp, 1 );
         data[x] = (((float( samp ) + 128.0f) / 255.0f) - 0.5f) * 2.0f;
      }
      else if (mBytesPerSample == 2)
      {
         short samp;
         mFile.read( (char*)&samp, 2 );
         if (isBig())
            byteReverse( samp );
         data[x] = (((float( samp ) + 32768.0f) / 65535.0f) - 0.5f) * 2.0f;
      }
      else if (mBytesPerSample == 4)
      {
         mFile.read( (char*)&data[x], 4 );
         if (isBig())
            byteReverse( data[x] );
      }
      ++actual_samples_read;
   }
   //for (int x = actual_samples_read; x  < samples; ++x)
   //{
      //data[x] = 0.0f;
   //}
}




/**
 * close the stream...
 */
void WavAudioIStream::close()
{
   mFile.close();
}


bool WavAudioIStream::good() const
{
   return mFile.good();
}

bool WavAudioIStream::bad() const
{
   return mFile.bad();
}

int WavAudioIStream::gcount() const
{
   return actual_samples_read;
}

