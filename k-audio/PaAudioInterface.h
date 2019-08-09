#ifndef PORTAUDIO_AUDIOINTERFACE
#define PORTAUDIO_AUDIOINTERFACE

#include <portaudio.h>
#include "AudioInterface.h"

class PaAudioInterface : public AudioInterface
{
public:
   PaAudioInterface()
   {
   }
   virtual ~PaAudioInterface() {}

   void init( UpdateBufferFunc f, void* userdata )
   {
      printf( "PaAudioInterface: Initializing PortAudio Audio...\n" );

      mUpdateBufferFunc = f;
      mUserData = userdata;

      mStream = NULL;
      PaStreamParameters  outputParameters;
      PaError             err;
      PaTime              streamOpened;
      int                 i, totalSamps;
      err = Pa_Initialize();
      if( err != paNoError ) error( "pa init failed" );
      outputParameters.device = Pa_GetDefaultOutputDevice(); /* Default output device. */
      if (outputParameters.device == paNoDevice)
      error( "Error: No default output device." );
      outputParameters.channelCount = 2;                     /* Stereo output. */
      outputParameters.sampleFormat = paFloat32;
      outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
      outputParameters.hostApiSpecificStreamInfo = NULL;
      err = Pa_OpenStream( &mStream,
                           NULL,      /* No input. */
                           &outputParameters,
                           44100,
                           1024,       /* Frames per buffer. */
                           paClipOff, /* We won't output out of range samples so don't bother clipping them. */
                           callback,
                           (void*)this /* user data pointer */ );
      if( err != paNoError ) error( "pa openstream failed" );

      streamOpened = Pa_GetStreamTime( mStream ); /* Time in seconds when stream was opened (approx). */

      err = Pa_StartStream( mStream );
      if( err != paNoError ) error( "pa startstream failed" );
   }

   bool isActive()
   {
      PaError err = Pa_IsStreamActive( mStream );
      if( err < 0 ) error( "pa is stream active failed" );
      return err == 1;
   }

   // call this while isActive()
   void update()
   {
      if (!mStarted)
         printf( "starting audio........\n" );
   }

   void sleep( int msec )
   {
      Pa_Sleep(100);
   }

   void close()
   {
      PaError err = Pa_CloseStream( mStream );
      if( err != paNoError ) error( "pa close stream failed" );
   }

private:
   PaStream* mStream;

   static int callback( const void *inputBuffer, void *outputBuffer,
                              unsigned long framesPerBuffer,
                              const PaStreamCallbackTimeInfo* timeInfo,
                              PaStreamCallbackFlags statusFlags,
                              void *userData )
   {
      AudioInterface* ths = (AudioInterface*)userData;
      ths->mStarted = true;
      return ths->mUpdateBufferFunc( inputBuffer, outputBuffer,
                                    framesPerBuffer,
                                    timeInfo->currentTime,
                                    ths->mUserData );
   }

   void error( const char* e = NULL )
   {
      if (e)
         printf( "%s\n", e );
      Pa_Terminate();
      exit(-1);
   }
};

#endif


