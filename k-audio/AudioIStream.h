#ifndef AUDIOISTREAM
#define AUDIOISTREAM


/// an input stream implementation on top of libsndlib.
/// libsndlib handles wav, aiff, many others...
class AudioIStream
{
public:
   virtual ~AudioIStream() {}
   virtual bool good() const = 0;
   virtual bool bad() const = 0;
   virtual int gcount() const = 0;
   virtual int samprate() const = 0;
   virtual int channels() const = 0;
   virtual int frames() const = 0;
   virtual int samples() const = 0;
   virtual void showinfo() = 0;
   virtual bool open( const char* const filename ) = 0;
   virtual void close() = 0;
   virtual void read_float( float* buf, int samps ) = 0;

   inline const char *  generate_duration_str (int frms=0)
   {
      if (samprate() < 1)
         return NULL;

      if ((frames() / samprate()) > 0x7FFFFFFF)
         return "unknown";

      static char str [128] = "\0";
      int f = (frms<=0 ? frames() : frms);
      const int seconds = f / samprate();
      sprintf(str, "%02d:%02d:%02d.%03d",
            seconds / 60 / 60,
            (seconds % (60 * 60)) / 60,
            seconds % 60,
            ((1000 * f) / samprate()) % 1000);

      return str;
   }
};

#endif

