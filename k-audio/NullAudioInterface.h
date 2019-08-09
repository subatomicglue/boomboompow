#ifndef NULL_AUDIOINTERFACE
#define NULL_AUDIOINTERFACE

#include <unistd.h> // for sleep
#include "WallClock.h"
#include "AudioInterface.h"

class NullAudioInterface : public AudioInterface
{
public:
   NullAudioInterface() : AudioInterface()
   {
      mTime = 0.0f;
   }
   virtual ~NullAudioInterface() {}

   void init( UpdateBufferFunc f, void* userdata )
   {
      printf( "NullAudioInterface: Initializing NULL Audio...\n" );
      AudioInterface::init( f, userdata );
   }

   bool isActive()
   {
      return true;
   }

   // call this while isActive()
   void update()
   {
      mTime += mClock.getDelta();
      mUpdateBufferFunc( NULL, NULL,
                        0,
                        mTime, mFormat,
                        mUserData );
   }

   void sleep( int msec )
   {
      sleep( msec );
   }

   void close()
   {
   }

private:
   WallClock mClock;
   float mTime;
};

#endif


