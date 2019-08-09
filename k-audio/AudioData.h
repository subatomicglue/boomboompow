
#include "common.h"
#include <vector>
#include <assert.h>
#include <samplerate.h>

class AudioData
{
   public:
      inline AudioData() {}
      inline ~AudioData() {}
      inline float operator[]( size_t x ) const { assert( x < mData.size() ); return mData[x]; }
      inline float& operator[]( size_t x ) { assert( x < mData.size() ); return mData[x]; }
      inline size_t size() const { return mData.size(); }
      void init( float samprate, size_t frms, size_t ch ) { mSampRate = samprate; mFrames = frms; mChans = ch; mData.resize( frms * ch ); }
      float* data() { assert( 0 < size() ); return &mData[0]; }
      void clear() { mData.clear(); mFrames = mChans = mSampRate = 0; }
      size_t channels() const { return mChans; }
      size_t frames() const { return mFrames; }
      size_t samples() const { return mFrames * mChans; }
      float samprate() const { return mSampRate; }
   private:
      std::vector<float> mData;
      size_t mFrames;
      size_t mChans;
      float mSampRate;
};

// iterate frame by frame, FAST: no resampling possible
class AudioDataIterator
{
   public:
      AudioDataIterator() : mData( NULL ), mInit( false ) {}
      AudioDataIterator( AudioData& d ) : mData( NULL ), mInit(true) { init( d ); }
      ~AudioDataIterator() { mData = (AudioData*)0xdeadface; }
      static const size_t SRC_MAX_OUTCHANNELS = 2; // i only support mono or stereo
      void init( AudioData& d ) { mInit = true; mData = &d; mPos = mData->frames() - 1; mNoClick[0] = mNoClick[1] = 0.0f; }

      // only support ++it.   not it++.   for performance...
      inline AudioDataIterator& operator++( int ) { assert( false ); }

      inline AudioDataIterator& operator++()
      {
         assert( mData && 0 < mData->frames() );
         if (mData && 0 < mData->frames())
            mPos = (mPos+1) % mData->frames();
         return *this;
      }
      // loop play, ring buffer iteration
      inline AudioDataIterator& incLoop()
      {
         assert( mData && 0 < mData->frames() );
         if (mData && 0 < mData->frames())
            mPos = (mPos+1) % mData->frames();
         return *this;
      }
      // single shot play iteration
      inline AudioDataIterator& incPlayOnce()
      {
         assert( mData && 0 < mData->frames() );
         if (mData && 0 < mData->frames())
         {
            size_t x = mPos + 1;
            size_t y = mData->frames()-1;
            mPos = std::min( x, y );
         }
         return *this;
      }
      // for advanced users who will do their own bounds checking
      inline AudioDataIterator& incUnsafe()
      {
         ++mPos;
         return *this;
      }

      // retrigger - reset iterator back to beginning...
      void reset()
      {
         if (mData && mData->frames())
         {
            for (size_t x = 0; x < mData->channels(); ++x)
            {
               mNoClick[x] = (*mData)[mPos * mData->channels() + x];
            }
            mPos = 0;
         }
      }

      inline float* operator*() const
      {
         size_t ch = mData->channels();
         assert( mData && 0 < mData->frames() );
         assert( 0 < mData->channels() && mData->channels() <= SRC_MAX_OUTCHANNELS );
         for (size_t x = 0; x < ch; ++x)
         {
            mNoClick[x] = mNoClick[x] > 0.0f ? mNoClick[x] - 0.01f : 0.0f;
            mOut[x] = (*mData)[mPos*ch + x] + mNoClick[x];
         }
         return (float*)mOut;
      }
   private:
      mutable float mOut[SRC_MAX_OUTCHANNELS];
      mutable float mNoClick[SRC_MAX_OUTCHANNELS];
      bool mInit;
      AudioData* mData;
      size_t mPos;
};


// audio data iterator, resampling enabled (slower)
class AudioDataSampleRateIterator
{
   public:
      inline AudioDataSampleRateIterator() : mSRCstate( NULL ), mData( NULL ), mInit( false ), mRatio( 1.0 ), mVolume( 1.0 ) {}
      inline AudioDataSampleRateIterator( AudioData& d ) : mSRCstate( NULL ), mData( NULL ), mInit(true), mRatio( 1.0 ), mVolume( 1.0 ) { init( d ); }
      inline ~AudioDataSampleRateIterator() { mData = (AudioData*)0xdeadface; if (mSRCstate) src_delete( mSRCstate ); mSRCstate = (SRC_STATE*)0xdeadface; }

      static const size_t SRC_MAX_OUTCHANNELS = 2; // i only support mono or stereo

      inline static long SRCcallback( void *cb_data, float **data )
      {
         AudioDataSampleRateIterator* ths = (AudioDataSampleRateIterator*)cb_data;
         if (ths->mPos == 0)
         {

            *data = ths->mData->data();
            return ths->mData->frames();
         }
         return 0;
      }
      void init( AudioData& d )
      {
         mInit = true; mData = &d;
         for (size_t x = 0; x < SRC_MAX_OUTCHANNELS; ++x)
         {
            mSRCoutNoclick[x] = mSRCout[x] = mNoClick[x] = 0.0f;
         }
         if (mData && 0 < mData->size())
         {
            // sanity check.
            assert( 0 <= mData->channels() && mData->channels() <= SRC_MAX_OUTCHANNELS );
            //SRC_SINC_BEST_QUALITY
            //SRC_SINC_FASTEST
            //SRC_LINEAR
            mSRCstate = src_callback_new( SRCcallback, SRC_LINEAR , mData->channels(),
                                           &mSRCerror, (void*)this );
            src_set_ratio( mSRCstate, mRatio );
            mPos = mData->frames() - 1;
         }
      }

      void setRatio( double r ) { mRatio = r; }
      void setVolume( double v ) { mVolume = v; }

      // operator++ for samplerate sampling
      // ratio = set the sample ratio.  1.0 for normal speed.
      // frame = returns channels() * float
      inline AudioDataSampleRateIterator& incPlayOnce()
      {
         assert( mData && 0 < mData->frames() );
         if (mData && mPos < mData->frames())
         {
            ++mPos;
            src_callback_read( mSRCstate, mRatio, 1, (float*)mSRCout );
            if (mData->channels())
               mSRCout[1] *= mVolume;
            mSRCout[0] *= mVolume;
         }
         return *this;
      }

      // loop play, ring buffer iteration
      inline AudioDataSampleRateIterator& incLoop()
      {
         assert( false && "implement me" );
         if (mData && mPos < mData->frames())
         {
            ++mPos;
            if (0 == src_callback_read( mSRCstate, mRatio, 1, (float*)mSRCout ))
            {
               mPos = 0;
               src_reset( mSRCstate );
               src_callback_read( mSRCstate, mRatio, 1, (float*)mSRCout );
            }
            if (mData->channels())
               mSRCout[1] *= mVolume;
            mSRCout[0] *= mVolume;
         }
         return *this;
      }

      // only support ++it.   not it++.   for performance...
      inline AudioDataSampleRateIterator& operator++( int ) { assert( false ); }
      inline AudioDataSampleRateIterator& operator++()
      {
         incLoop();
         return *this;
      }

      // retrigger - reset iterator back to beginning...
      void reset()
      {
         if (mData && mData->frames())
         {
            for (size_t x = 0; x < mData->channels(); ++x)
            {
               mNoClick[x] = (*mData)[std::min( mPos, mData->frames()-1 ) * mData->channels() + x];
            }

            mPos = 0;
            src_reset( mSRCstate );
            src_callback_read( mSRCstate, mRatio, 1, (float*)mSRCout );
         }
      }

      // return an array of up to SRC_MAX_OUTCHANNELS floats
      // number of floats returned will be mData->channels()...
      inline float* operator*()
      {
         assert( mData && 0 <= mData->channels() && mData->channels() <= SRC_MAX_OUTCHANNELS );
         for (size_t x = 0; x < SRC_MAX_OUTCHANNELS; ++x)
         {
            mNoClick[x] = mNoClick[x] > 0.0f ? mNoClick[x] - 0.01f : 0.0f;
            mSRCoutNoclick[x] = mSRCout[x] + mNoClick[x];
         }
         return (float*)mSRCoutNoclick;
      }
   private:
      // samplerate iteration
      double mRatio;
      double mVolume;
      int mSRCerror;
      SRC_STATE* mSRCstate;
      SRC_DATA mSRCdata;
      float mSRCout[SRC_MAX_OUTCHANNELS];
      float mSRCoutNoclick[SRC_MAX_OUTCHANNELS];
      bool mInit;
      AudioData* mData;
      size_t mPos;
      mutable float mNoClick[SRC_MAX_OUTCHANNELS];
};

inline static void testAudioData()
{
   AudioData d;
   d.init( 44100, 4, 1 );
   d[0] = 0.1;
   d[1] = 0.2;
   d[2] = 0.3;
   d[3] = 0.4;
   if (1)
   {
      AudioDataIterator it( d );
/*      printf( "%f (new)\n", (*it)[0] ); it.incLoop();
      printf( "%f\n", (*it)[0] ); it.incLoop();
      printf( "%f\n", (*it)[0] ); it.incLoop();
      printf( "%f\n", (*it)[0] ); it.incLoop();
      printf( "%f\n", (*it)[0] ); it.incLoop();
      printf( "%f\n", (*it)[0] ); it.incLoop();
      printf( "----\n" );
*/      it.reset();
      printf( "%f (reset)\n", (*it)[0] ); it.incPlayOnce();
      printf( "%f\n", (*it)[0] ); it.incPlayOnce();
      printf( "%f\n", (*it)[0] ); it.incPlayOnce();
      printf( "%f\n", (*it)[0] ); it.incPlayOnce();
      printf( "%f\n", (*it)[0] ); it.incPlayOnce();
      printf( "%f\n", (*it)[0] ); it.incPlayOnce();
      it.reset();
      printf( "%f (reset)\n", (*it)[0] ); it.incPlayOnce();
      printf( "%f\n", (*it)[0] ); it.incPlayOnce();
      printf( "%f\n", (*it)[0] ); it.incPlayOnce();
   }/*
   printf( "---------------\n" );
   {
      AudioDataSampleRateIterator it( d );
      printf( "begin\n" );
      it.reset();
      printf( "%f (reset)\n", (*it)[0] ); it.incPlayOnce();
      printf( "%f\n", (*it)[0] ); it.incPlayOnce();
      printf( "%f\n", (*it)[0] ); it.incPlayOnce();
      printf( "%f\n", (*it)[0] ); it.incPlayOnce();
      printf( "%f\n", (*it)[0] ); it.incPlayOnce();
      printf( "%f\n", (*it)[0] ); it.incPlayOnce();
      it.reset();
      printf( "%f (reset)\n", (*it)[0] ); it.incPlayOnce();
      printf( "%f\n", (*it)[0] ); it.incPlayOnce();
      printf( "%f\n", (*it)[0] ); it.incPlayOnce();
   }*/
}

