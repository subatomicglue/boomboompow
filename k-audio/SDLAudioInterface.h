#ifndef SDL_AUDIOINTERFACE
#define SDL_AUDIOINTERFACE

#include <unistd.h> // for sleep
#include "WallClock.h"
#include "AudioInterface.h"

class SDLAudioInterface : public AudioInterface
{
public:
   SDLAudioInterface() : AudioInterface()
   {
      mTime = 0.0f;
   }
   virtual ~SDLAudioInterface() {}

   void init( UpdateBufferFunc f, void* userdata )
   {
      AudioInterface::init( f, userdata );

      if (0 != SDL_InitSubSystem( SDL_INIT_AUDIO ))
      {
         printf( "couldn't SDL_InitSubSystem( SDL_INIT_AUDIO )\n" );
         exit(-1);
         return;
      }

      printf( "SDLAudioInterface: Initializing SDL Audio...\n" );
      int count = SDL_GetNumAudioDevices(0);
      for (int i = 0; i < count; ++i)
      {
         printf( "Audio device %d: %s\n", i, SDL_GetAudioDeviceName(i, 0) );
      }
      SDL_zero(want);
      want.freq = 44100;
      want.channels = 2;
      want.callback = SDL_AudioCallback;
      want.userdata = (void*)this;

#if TARGET_OS_IPHONE
      want.format = AUDIO_S16LSB;
      want.samples = 256;
#else
      want.format = AUDIO_F32;
      want.samples = 1024;
#endif
 
      dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
      if (dev == 0) {
         printf("Failed to open audio: %s\n", SDL_GetError());
         exit(0);
      } else {
         if (have.format != want.format)  // we let this one thing change.
            printf("We didn't get the requested audio format.\n");
         SDL_PauseAudioDevice(dev, 0);  // start audio playing.
      }
       
      switch (have.format)
      {
         case AUDIO_F32:    mFormat = AudioInterface::F32; break;
         case AUDIO_S16LSB: mFormat = AudioInterface::S16LSB; break;
         default: mFormat = AudioInterface::UNKNOWN;
      }
   }

   bool isActive()
   {
      return SDL_GetAudioDeviceStatus( dev ) == SDL_AUDIO_PLAYING;
   }

   // call this while isActive()
   void update()
   {
   }

   void sleep( int msec )
   {
      SDL_Delay(msec);
   }

   void close()
   {
      SDL_CloseAudioDevice(dev);
   }

private:
   WallClock mClock;
   float mTime;

   SDL_AudioSpec want, have;
   SDL_AudioDeviceID dev;

   static void SDL_AudioCallback(void* userdata, Uint8* stream, int len)
   {
      SDLAudioInterface* ths = (SDLAudioInterface*)(AudioInterface*)userdata;
      ths->mStarted = true;
      int sampsize = sizeof(short);
      switch (ths->have.format)
      {
         case AUDIO_F32:    sampsize = sizeof(float); break;
         case AUDIO_S16LSB: sampsize = sizeof(short); break;
         default: printf( "unknown in SDL_AudioCallback\n" ); exit(0);
      }
      ths->mUpdateBufferFunc( NULL, stream,
                              len / (ths->have.channels * sampsize),
                              ths->mTime, ths->mFormat,
                              ths->mUserData );
      ths->mTime += ths->mClock.getDelta();
   }
   
   static void testcallback( void* userdata, Uint8* stream, int len )
   {
      static float bok = 0;
      SDLAudioInterface* ths = (SDLAudioInterface*)(AudioInterface*)userdata;
      ths->mStarted = true;
      int sampsize = sizeof(short);
      switch (ths->have.format)
      {
         case AUDIO_F32:    sampsize = sizeof(float); break;
         case AUDIO_S16LSB: sampsize = sizeof(short); break;
         default: printf( "unknown in SDL_AudioCallback\n" ); exit(0);
      }

      // audio
      int framesPerBuffer = len / (ths->have.channels * sampsize);
      short* out = (short*)stream;
      for (int i = 0; i < framesPerBuffer; ++i)
      {
         bok += 1.0f/44100.0f;
         float w = sinf(2.0f * 3.14159265359f * 220.0f * bok) * 0.5f;
         *out++ = (short)floorf(w * 32767.0f);
         *out++ = (short)floorf(w * 32767.0f);
      }
   }
};

#endif


