#ifndef AUDIOINTERFACE
#define AUDIOINTERFACE

class AudioInterface
{
public:
    enum Format { S16LSB, F32, UNKNOWN };
   typedef int (*UpdateBufferFunc)(const void *inputBuffer, void *outputBuffer,
                                    unsigned long framesPerBuffer,
                                    const double currentTime, Format f,
                                    void *userData );
   UpdateBufferFunc mUpdateBufferFunc;
   void* mUserData;
   bool mStarted;
    Format mFormat;


public:
   AudioInterface()
   {
      mUpdateBufferFunc = NULL;
      mStarted = false;
      mUserData = NULL;
      mFormat = AudioInterface::UNKNOWN;
   }
   virtual ~AudioInterface() {}

   virtual void init( UpdateBufferFunc f, void* userdata )
   {
      mUpdateBufferFunc = f;
      mUserData = userdata;
   }

   virtual bool isActive() = 0;
   virtual void update() = 0;
   virtual void sleep( int msec ) = 0;
   virtual void close() = 0;

private:
};

#endif

