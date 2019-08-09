/*
This code is modified from Audiere to fit the AudioIStream paradigm, namely
ability to query stream state, read floats, and uses ifstream for input 
instead of Audiere's file type...
*/

/*
  MP3 input for Audiere by Matt Campbell <mattcampbell@pobox.com>, based on
  libavcodec from ffmpeg (http://ffmpeg.sourceforge.net/).  I hope this will
  turn out better than mpegsound did.
*/

#include <vector>
#include <fstream>
#include <string.h>
#include <algorithm>
#include "mpaudec/mpaudec.h"
#include "Defines.h"
#include "AudioIStream.h"

struct Tag
{
   Tag( const std::string& k, const std::string& v, const std::string& t )
   {
      key = k;
      value = v;
      type = t;
   }

   std::string key;
   std::string value;
   std::string type;

};

/// Storage formats for sample data.
enum SampleFormat {
   SF_U8,        ///< unsigned 8-bit integer [0,255]
   SF_S16,       ///< signed 16-bit integer in host endianness [-32768,32767]
};

class QueueBuffer
{

public:
   QueueBuffer()
   {
      m_capacity = 256;
      m_size = 0;

      m_buffer = ( u8* ) malloc( m_capacity );
   }

   ~QueueBuffer()
   {
      m_buffer = ( u8* ) realloc( m_buffer, 0 );
   }

   int getSize()
   {
      return m_size;
   }

   void write( const void* buffer, int size )
   {
      bool need_realloc = false;

      while ( size + m_size > m_capacity )
      {
         m_capacity *= 2;
         need_realloc = true;
      }

      if ( need_realloc )
      {
         m_buffer = ( u8* ) realloc( m_buffer, m_capacity );
      }

      memcpy( m_buffer + m_size, buffer, size );
      m_size += size;
   }

   int read( void* buffer, int size )
   {
      int to_read = std::min( size, m_size );
      memcpy( buffer, m_buffer, to_read );
      memmove( m_buffer, m_buffer + to_read, m_size - to_read );
      m_size -= to_read;
      return to_read;
   }

   int read_u16_to_float( float* buffer, int sizebytes )
   {
      int bytes_to_read = std::min( sizebytes, m_size );
      int samps_to_read = bytes_to_read / sizeof( signed short );

      for ( int x = 0; x < samps_to_read; ++x )
      {
         buffer[x] = ( ( signed short* ) m_buffer ) [ x ];
         buffer[x] *= ( 1.0f / 32767.0f ); // scale to -1..1
      }

      memmove( m_buffer, m_buffer + bytes_to_read, m_size - bytes_to_read );

      m_size -= bytes_to_read;

      return bytes_to_read;
   }

   void clear()
   {
      m_size = 0;
   }

private:
   u8* m_buffer;
   int m_capacity;
   int m_size;

   // private and unimplemented to prevent their use
   QueueBuffer( const QueueBuffer& );
   QueueBuffer& operator=( const QueueBuffer& );
};


class MpegAudioIStream;

int GetFrameSize( MpegAudioIStream* source );

class MpegAudioIStream : public AudioIStream
{
public:
   MpegAudioIStream();
   MpegAudioIStream( const char* filename );
   virtual ~MpegAudioIStream();
   
   virtual bool open( const char* const filename );
   virtual void read_float( float* samples, int frame_count );
   int read( void* samples, int frame_count );
   virtual void resetStream();

   virtual int gcount() const { return mGetCount; }
   virtual bool isSeekable() const;
   virtual int frames() const;
   virtual int samples() const
   {
      return frames() * channels();
   }

   SampleFormat getFormat() const;

   virtual int channels() const
   {
      return m_channel_count;
   }

   virtual int samprate() const
   {
      return m_sample_rate;
   }

   virtual void setPosition( int position );
   virtual int getPosition() const;

   virtual bool good() const
   {
      return m_file && !m_eof && 0 < m_length;
   }

   virtual bool bad() const
   {
      return !m_file || m_eof || m_length <= 0;
   }

   virtual void close()
   {
      if (good())
      {
         m_file->close();
         resetValues();
      }
   }

    virtual void showinfo()
    {
       printf( "Filename    : \"%s\"\n", m_FileName.c_str() );
       printf( "Sample Rate : %d\n", samprate() );
       printf( "Frames      : %d\n", frames() );
       printf( "Samples     : %d\n", samples() );
       printf( "Channels    : %d\n", channels() );
       printf( "Format      : %d\n", getFormat() );
       printf( "Duration    : %s\n", AudioIStream::generate_duration_str() );
    }

protected:
   void addTag( const Tag& t )
   {
      m_tags.push_back( t );
   }

   void addTag( const std::string& k, const std::string& v, const std::string& t )
   {
      addTag( Tag( k, v, t ) );
   }

private:
   bool initialize( std::ifstream* file );
   void resetValues();
   void readID3v1Tags();
   void readID3v2Tags();
   bool decodeFrame();

   bool m_repeat;
   std::vector<Tag> m_tags;
   std::ifstream* m_file;
   bool m_eof;

   // from format chunk
   int m_channel_count;
   int m_sample_rate;
   SampleFormat m_sample_format;

   MPAuDecContext* m_context;

   QueueBuffer m_buffer;

   enum { INPUT_BUFFER_SIZE = 4096 };
   u8 m_input_buffer[ INPUT_BUFFER_SIZE ];
   int m_input_position;
   int m_input_length;
   u8* m_decode_buffer;
   bool m_first_frame;

   bool m_seekable;
   int m_length;
   int m_position;
   std::string m_FileName;
   std::vector<int> m_frame_sizes;
   std::vector<int> m_frame_offsets;
   int mGetCount;
};

MpegAudioIStream::MpegAudioIStream()
{
   resetValues();
}

void MpegAudioIStream::resetValues()
{
   m_eof = false;

   m_channel_count = 2;
   m_sample_rate = 44100;
   m_sample_format = SF_S16;

   m_context = 0;

   m_input_position = 0;
   m_input_length = 0;
   m_decode_buffer = 0;
   m_first_frame = true;

   m_seekable = false;
   m_length = 0;
   m_position = 0;
   m_repeat = false;
   m_FileName = "";
   mGetCount = 0;
}

MpegAudioIStream::MpegAudioIStream( const char* const filename )
{
   resetValues();
   open( filename );
}

MpegAudioIStream::~MpegAudioIStream()
{
   delete[] m_decode_buffer;

   if ( m_context )
   {
      mpaudec_clear( m_context );
      delete m_context;
   }
}

bool MpegAudioIStream::open( const char* const filename )
{
   m_file = new std::ifstream( filename, std::ios::binary | std::ios::in );
   m_FileName = filename;

   if ( m_file->bad() )
   {
      std::cerr << "couldn't open mp3 file " << filename << " for reading\n" << std::endl;
      return false;
   }

   initialize( m_file );

   if ( m_length <= 0 )
   {
      std::cerr << "file " << filename << " doesn't appear to have mp3 data\n" << std::endl;
      return false;
   }

   return true;
}

bool
MpegAudioIStream::initialize( std::ifstream* file )
{
   m_file = file;
   m_seekable = true;
   m_file->seekg( 0, std::ios::end );
   readID3v1Tags();
   readID3v2Tags();
   m_file->seekg( 0, std::ios::beg );
   m_eof = false;

   m_context = new MPAuDecContext;

   if ( !m_context )
      return false;

   if ( mpaudec_init( m_context ) < 0 )
   {
      delete m_context;
      m_context = 0;
      return false;
   }

   m_input_position = 0;
   m_input_length = 0;
   m_decode_buffer = new u8[ MPAUDEC_MAX_AUDIO_FRAME_SIZE ];

   if ( !m_decode_buffer )
      return false;

   m_first_frame = true;

   if ( m_seekable )
   {
      // Scan the file to determine the length.
      m_context->parse_only = 1;

      while ( !m_eof )
      {
         if ( !decodeFrame() )
            return false;

         if ( !m_eof )
            m_frame_sizes.push_back( m_context->frame_size );

         int frame_offset = int( m_file->tellg() ) -
                            ( m_input_length - m_input_position ) -
                            m_context->coded_frame_size;

         m_frame_offsets.push_back( frame_offset );

         m_length += m_context->frame_size;
      }

      resetStream();
   }

   // this should fill in the audio format if it isn't set already
   return decodeFrame();
}

bool
MpegAudioIStream::isSeekable() const
{
   return m_seekable;
}

int
MpegAudioIStream::getPosition() const
{
   return m_position;
}

void
MpegAudioIStream::setPosition( int position )
{
   if ( !m_seekable || position > m_length )
      return ;

   int scan_position = 0;

   int target_frame = 0;

   int frame_count = m_frame_sizes.size();

   while ( target_frame < frame_count )
   {
      int frame_size = m_frame_sizes[ target_frame ];

      if ( position <= scan_position + frame_size )
         break;
      else
      {
         scan_position += frame_size;
         target_frame++;
      }
   }

   // foobar2000's MP3 input plugin decodes and throws away the 10 frames
   // before the target frame whenever possible, presumably to ensure correct
   // output when jumping into the middle of a stream.  So we'll do that here.
   const int MAX_FRAME_DEPENDENCY = 10;

   target_frame = std::max( 0, target_frame - MAX_FRAME_DEPENDENCY );

   resetStream();

   m_file->seekg( m_frame_offsets[ target_frame ], std::ios::beg );

   int i;

   for ( i = 0; i < target_frame; i++ )
   {
      m_position += m_frame_sizes[ i ];
   }

   if ( !decodeFrame() || m_eof )
   {
      resetStream();
      return ;
   }

   int frames_to_consume = position - m_position; // PCM frames now

   if ( frames_to_consume > 0 )
   {
      u8 * buf = new u8[ frames_to_consume * GetFrameSize( this ) ];
      read( buf, frames_to_consume );
      delete[] buf;
   }
}

int
MpegAudioIStream::frames() const
{
   return m_length;
}

SampleFormat MpegAudioIStream::getFormat() const
{
   return m_sample_format;
}


void MpegAudioIStream::read_float( float* samples, int num_samples )
{
   const int frame_count = num_samples / channels();
   const int frame_size = GetFrameSize( this );

   int frames_read = 0;
   f32* out = (f32*)samples;

   while ( frames_read < frame_count )
   {
      // no more samples?  ask the MP3 for more

      if ( m_buffer.getSize() < frame_size )
      {
         if ( !decodeFrame() || m_eof )
         {
            // done decoding?
            mGetCount = frames_read;
            return;
//            return frames_read;
         }

         // if the buffer is still empty, we are done
         if ( m_buffer.getSize() < frame_size )
         {
            mGetCount = frames_read;
            return;
//            return frames_read;
         }
      }

      const int frames_left = frame_count - frames_read;

      const int frames_to_read = std::min( frames_left,
                                           m_buffer.getSize() / frame_size );

      m_buffer.read_u16_to_float( out, frames_to_read * frame_size );

      //m_buffer.read_u16_to_float(out, frames_to_read * frame_size);
      out += frames_to_read * /*frame_size*/ channels();

      frames_read += frames_to_read;

      m_position += frames_to_read;

      //printf( "%d:  %d %d %d %d %lu\n", frames_read, frame_count, frames_to_read, frames_left, frame_size, m_buffer.getSize() );
   }
   mGetCount = frames_read;
   return;
   //return frames_read;
}

int MpegAudioIStream::read( void* samples, int frame_count )
{
   const int frame_size = GetFrameSize( this );

   int frames_read = 0;
   u8* out = (u8*)samples;

   while ( frames_read < frame_count )
   {
      // no more samples?  ask the MP3 for more
      if ( m_buffer.getSize() < frame_size )
      {
         if ( !decodeFrame() || m_eof )
         {
            // done decoding?
            return frames_read;
         }

         // if the buffer is still empty, we are done
         if ( m_buffer.getSize() < frame_size )
         {
            return frames_read;
         }
      }

      const int frames_left = frame_count - frames_read;
      const int frames_to_read = std::min( frames_left,
                                           m_buffer.getSize() / frame_size );
      m_buffer.read(out, frames_to_read * frame_size);
      out += frames_to_read * frame_size;
      frames_read += frames_to_read;
      mGetCount = frames_read;
      m_position += frames_to_read;
   }

   return frames_read;
}


void MpegAudioIStream::resetStream()
{
   if ( !m_file->good() )
   {
      m_file->clear();
   }

   m_file->seekg( 0, std::ios::beg );
   m_eof = false;

   m_buffer.clear();

   mpaudec_clear( m_context );
   mpaudec_init( m_context );

   m_input_position = 0;
   m_input_length = 0;
   m_position = 0;
}


bool MpegAudioIStream::decodeFrame()
{
   int output_size = 0;

   while ( output_size == 0 )
   {
      if ( m_input_position == m_input_length )
      {
         m_input_position = 0;
         m_file->read( ( char* ) m_input_buffer, INPUT_BUFFER_SIZE );
         m_input_length = m_file->gcount();

         if ( m_input_length == 0 )
         {
            m_eof = true;
            return true;
         }
      }

      int rv = mpaudec_decode_frame(
                  m_context, ( s16* ) m_decode_buffer,
                  &output_size,
                  ( unsigned char* ) m_input_buffer + m_input_position,
                  m_input_length - m_input_position );

      if ( rv < 0 )
         return false;

      m_input_position += rv;
   }

   if ( m_first_frame )
   {
      m_channel_count = m_context->channels;
      m_sample_rate = m_context->sample_rate;
      m_sample_format = SF_S16;
      m_first_frame = false;
   }

   else if ( m_context->channels != m_channel_count ||
             m_context->sample_rate != m_sample_rate )
   {
      // Can't handle format changes mid-stream.
      return false;
   }

   if ( !m_context->parse_only )
   {
      if ( output_size < 0 )
      {
         // Couldn't decode this frame.  Too bad, already lost it.
         // This should only happen when seeking.
         output_size = m_context->frame_size;
         memset( m_decode_buffer, 0, output_size * GetFrameSize( this ) );
      }

      m_buffer.write( m_decode_buffer, output_size );
   }

   return true;
}


const char* getGenre( u8 code )
{
   const char * genres[] =
      {
         // From Appendix A.3 at http://www.id3.org/id3v2-00.txt and

         "Blues", "Classic Rock", "Country", "Dance", "Disco", "Funk",
         "Grunge", "Hip-Hop", "Jazz", "Metal", "New Age", "Oldies", "Other",
         "Pop", "R&B", "Rap", "Reggae", "Rock", "Techno", "Industrial",
         "Alternative", "Ska", "Death Metal", "Pranks", "Soundtrack",
         "Euro-Techno", "Ambient", "Trip-Hop", "Vocal", "Jazz+Funk",
         "Fusion", "Trance", "Classical", "Instrumental", "Acid", "House",
         "Game", "Sound Clip", "Gospel", "Noise", "AlternRock", "Bass",
         "Soul", "Punk", "Space", "Meditative", "Instrumental Pop",
         "Instrumental Rock", "Ethnic", "Gothic", "Darkwave",
         "Techno-Industrial", "Electronic", "Pop-Folk", "Eurodance",
         "Dream", "Southern Rock", "Comedy", "Cult", "Gangsta", "Top 40",
         "Christian Rap", "Pop/Funk", "Jungle", "Native American",
         "Cabaret", "New Wave", "Psychadelic", "Rave", "Showtunes",
         "Trailer", "Lo-Fi", "Tribal", "Acid Punk", "Acid Jazz", "Polka",
         "Retro", "Musical", "Rock & Roll", "Hard Rock", "Folk", "Folk-Rock",
         "National Folk", "Swing", "Fast Fusion", "Bebob", "Latin", "Revival",
         "Celtic", "Bluegrass", "Avantgarde", "Gothic Rock",
         "Progressive Rock", "Psychedelic Rock", "Symphonic Rock",
         "Slow Rock", "Big Band", "Chorus", "Easy Listening", "Acoustic",
         "Humour", "Speech", "Chanson", "Opera", "Chamber Music", "Sonata",
         "Symphony", "Booty Bass", "Primus", "Porn Groove", "Satire",
         "Slow Jam", "Club", "Tango", "Samba", "Folklore", "Ballad",
         "Power Ballad", "Rhythmic Soul", "Freestyle", "Duet", "Punk Rock",
         "Drum Solo", "Acapella", "Euro-House", "Dance Hall",

         // http://lame.sourceforge.net/doc/html/id3.html

         "Goa", "Drum & Bass", "Club-House", "Hardcore", "Terror", "Indie",
         "BritPop", "Negerpunk", "Polsk Punk", "Beat", "Christian Gangsta",
         "Heavy Metal", "Black Metal", "Crossover", "Contemporary C",
         "Christian Rock", "Merengue", "Salsa", "Thrash Metal", "Anime",
         "JPop", "SynthPop",
      };

   const int genre_count = sizeof( genres ) / sizeof( *genres );

   return ( code < genre_count ? genres[ code ] : "" );
}


// Return a null-terminated std::string from the beginning of 'buffer'
// up to 'maxlen' chars in length.
std::string getString( u8* buffer, int maxlen )
{
   char * begin = reinterpret_cast<char*>( buffer );
   int end = 0;

   for ( ; end < maxlen && begin[ end ]; ++end )
   {}

   return std::string( begin, begin + end );
}


void MpegAudioIStream::readID3v1Tags()
{
   // Actually, this function reads both ID3v1 and ID3v1.1.

   if ( !m_file->seekg( -128, std::ios::end ) )
   {
      return ;
   }

   u8 buffer[ 128 ];
   m_file->read( ( char* ) buffer, 128 );

   if ( m_file->gcount() != 128 )
   {
      return ;
   }

   // Verify that it's really an ID3 tag.
   if ( memcmp( buffer + 0, "TAG", 3 ) != 0 )
   {
      return ;
   }

   std::string title = getString( buffer + 3, 30 );
   std::string artist = getString( buffer + 33, 30 );
   std::string album = getString( buffer + 63, 30 );
   std::string year = getString( buffer + 93, 4 );
   std::string comment = getString( buffer + 97, 30 );
   std::string genre = getGenre( buffer[ 127 ] );

   addTag( "title", title, "ID3v1" );
   addTag( "artist", artist, "ID3v1" );
   addTag( "album", album, "ID3v1" );
   addTag( "year", year, "ID3v1" );
   addTag( "comment", comment, "ID3v1" );
   addTag( "genre", genre, "ID3v1" );

   // This is the ID3v1.1 part.

   if ( buffer[ 97 + 28 ] == 0 && buffer[ 97 + 29 ] != 0 )
   {
      char track[ 20 ];
      sprintf( track, "%d", int( buffer[ 97 + 29 ] ) );
      addTag( "track", track, "ID3v1.1" );
   }
}


void MpegAudioIStream::readID3v2Tags()
{
   // ID3v2 is super complicated.
}


inline int GetFrameSize( MpegAudioIStream* source )
{
   return ( source->getFormat() == SF_U8 ? 1 : 2 ) * source->channels();
}

