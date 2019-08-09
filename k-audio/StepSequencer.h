#ifndef STEPSEQUENCER
#define STEPSEQUENCER

#include <stdio.h>
#include "synMath.h"

// sequencer
class StepSequencer
{
public:
   double mTempo; // seconds per beat
   double mWallClock;
   double mWallClockLast;
   double mWallClockDelta;
   double mTimeUntilNextStep;
   size_t mStepsPerBeat;

   int    mCurrentStep;
   bool   mPlaying;
   size_t mPageSize;    // number of steps in a page
   int    mDisplayPage; // which page is showing now.
   bool   mUseDebug;
   bool   mTick;

   size_t numPages() const { return size_steps() / mPageSize; }
   size_t curPage() const { return mCurrentStep / mPageSize; }

   struct StepEvent
   {
      StepEvent() : trigger( false ), ratio( 1.0 ), accent( 1.0 ) {}
      bool trigger;
      double ratio;
      double accent;
   };
   struct Action
   {
      virtual void trigger( int tracknum ) = 0;
      virtual void ratio( int tracknum, double r ) = 0;
      virtual void accent( int tracknum, double a ) = 0;
   };

   std::vector< std::vector<StepEvent> > mSeqTrack; // [tracks][steps]
   size_t size_tracks() const { return mSeqTrack.size(); }
   size_t size_steps() const { return mSeqTrack[0].size(); }
   void resize_tracks( size_t s )
   {
      assert( 0 < s );
      mSeqTrack.resize( s );
   }
   void resize_steps( size_t s )
   {
      assert( 0 < mSeqTrack.size() && 0 < s );
      for (size_t x = 0; x < mSeqTrack.size(); ++x)
      {
         mSeqTrack[x].resize( s );
      }
   }
   StepEvent& step( size_t track, size_t step )
   {
      assert( track < size_tracks() && step < size_steps() );
      return mSeqTrack[track][step];
   }

   bool& trigger( int step, int track )
   {
      assert( 0 <= step && step < size_steps() );
      assert( 0 <= track && track < size_tracks() );
      return mSeqTrack[track][step].trigger;
   }
   double& ratio( int step, int track )
   {
      assert( 0 <= step && step < size_steps() );
      assert( 0 <= track && track < size_tracks() );
      return mSeqTrack[track][step].ratio;
   }
   double& accent( int step, int track )
   {
      assert( 0 <= step && step < size_steps() );
      assert( 0 <= track && track < size_tracks() );
      return mSeqTrack[track][step].accent;
   }

   StepSequencer()
   {
      init();
   }

   void init()
   {
      //Half note               =  120 / BPM
      //Quarter note            =   60 / BPM
      //Eighth note             =   30 / BPM
      //Sixteenth note          =   15 / BPM
      //Dotted-quarter note     =   90 / BPM
      //Dotted-eighth note      =   45 / BPM
      //Dotted-sixteenth note   = 22.5 / BPM
      //Triplet-quarter note    =   40 / BPM
      //Triplet-eighth note     =   20 / BPM
      //Triplet-sixteenth note  =   10 / BPM
      resize_tracks( 4 );
      resize_steps( 32 );
      mTempo = 30.0 / 170.0;
      mWallClock = 0.0;
      mWallClockLast = 0.0;
      mWallClockDelta = 0.0;
      mPageSize = 8;
      mStepsPerBeat = mPageSize/2;
      mDisplayPage = -1;
      mUseDebug = false;
      mTick = false;

      mCurrentStep = size_steps()-1;
      mTimeUntilNextStep = mTempo;
      mPlaying = false;
      debugprint_laststep = -1;
      for (int x = 0; x < size_tracks(); ++x)
      {
         for (int y = 0; y < size_steps(); ++y)
         {
            mSeqTrack[x][y].ratio = 1.0;
            mSeqTrack[x][y].trigger = 0;
            mSeqTrack[x][y].accent = 1.0;
         }
      }
   }

   double getTempoBPM() const
   { return (1.0 / (mTempo/60.0)) / (double)mStepsPerBeat; }
   void setTempoBPM( double bpm, double stepsPerBeat )
   { mTempo = (1.0 / (bpm*stepsPerBeat)) * 60; }

   void save( const char* filename )
   {
      printf( "[save] %s\n", filename );
      FILE* f = fopen( filename, "wb" );
      fprintf( f, "tracks %ld\n", size_tracks() );
      fprintf( f, "steps %ld\n", size_steps() );
      fprintf( f, "tempo %lf\n", getTempoBPM() );
      fprintf( f, "steps_per_beat %ld\n", mStepsPerBeat );
      for (int x = 0; x < size_tracks(); ++x)
      {
         for (int y = 0; y < size_steps(); ++y)
         {
            fprintf( f, "%d  %lf  %lf\n", mSeqTrack[x][y].trigger,
                                     mSeqTrack[x][y].ratio,
                                     mSeqTrack[x][y].accent );
         }
         fprintf( f, "\n" );
      }
      fclose( f );
   }

   void load( const char* filename )
   {
      printf( "[load] %s\n", filename );
      FILE* f = fopen( filename, "rb" );
      char buf[256];
      size_t nt, ns;
      fscanf( f, "%s", (char*)&buf ); // tracks
      fscanf( f, "%ld", &nt );
      fscanf( f, "%s", (char*)&buf ); // steps
      fscanf( f, "%ld", &ns );
      assert( nt <= size_tracks() );
      assert( ns <= size_steps() );
      fscanf( f, "%s", (char*)&buf ); // tempo
      fscanf( f, "%lf", &mTempo );
      fscanf( f, "%s", (char*)&buf ); // steps_per_beat
      fscanf( f, "%ld", &mStepsPerBeat );
      setTempoBPM( mTempo, mStepsPerBeat );
      for (int x = 0; x < nt; ++x)
      {
         for (int y = 0; y < ns; ++y)
         {
            int temp;
            fscanf( f, "%d", &temp );
            fscanf( f, "%lf", &mSeqTrack[x][y].ratio );
            fscanf( f, "%lf", &mSeqTrack[x][y].accent );
            mSeqTrack[x][y].trigger = temp;
         }
      }
      fclose( f );
   }

   void useDebug( bool d ) { mUseDebug = d; }

   void playStepOnPage( int step, unsigned int page )
   {
      mPlaying = true;
      mCurrentStep = ((page * mPageSize) + (step-1) + size_steps()) % size_steps();
      mTimeUntilNextStep = 0.0f;
   }

   void trigStepOnPage( int step, unsigned int page, Action& cb )
   {
      mCurrentStep = (page * mPageSize + step) % size_steps();

      // execute the step
      for (int x = 0; x < size_tracks(); ++x)
      {
         if (mSeqTrack[x][mCurrentStep].trigger)
         {
            // set the frequency/pitch for the track sound, then trigger the track sound
            cb.ratio( x, mSeqTrack[x][mCurrentStep].ratio );
            cb.accent( x, mSeqTrack[x][mCurrentStep].accent );
            cb.trigger( x );
         }
      }
   }

   void trigTrackSound( int track, float ratio, Action& cb )
   {
      // execute the step
      if (0 <= track && track < size_tracks())
      {
         // set the frequency/pitch for the track sound, then trigger the track sound
         cb.ratio( track, ratio );
         cb.accent( track, 1.0 );
         cb.trigger( track );
      }
   }

   void playStepOnCurrentPage( int step )
   {
      mPlaying = true;
      mCurrentStep = ((curPage() * mPageSize) + (step-1) + size_steps()) % size_steps();
      mTimeUntilNextStep = 0.0f;
      printf( "step:%d currentstep:%d page:%lu, ps:%ld\n", step, mCurrentStep, curPage(), mPageSize );
   }

   void playStep( int step )
   {
      mPlaying = true;
      mCurrentStep = (step + size_steps() - 1) % size_steps();
      mTimeUntilNextStep = 0.0f;
   }

   void play( bool state = true )
   {
      mPlaying = state;
      if (state)
      {
         mCurrentStep = size_steps() - 1;
      }
      else
      {
         mCurrentStep = 0;
      }
      mTimeUntilNextStep = 0.0f;
   }

   // true pauses, false unpauses
   void pause( bool state = true )
   {
      mPlaying = !state; // true == pause(stop), false == unpause(play)
   }

private:
   inline bool eventsAfter( int step = 8 ) const
   {
      bool any = false;
      for (int x = 0; x < size_tracks(); ++x)
      {
         for (int y = step; y < size_steps(); ++y)
         {
            any |= mSeqTrack[x][y].trigger;
         }
      }
      return any;
   }
   // look at blank space in the tracks, return number of steps that are actually filled with content
   // can specify the page size boundary, useful for "pages" of tracks
   inline size_t computeNumberOfStepsFilled( size_t mPageSize = 1 ) const
   {
      size_t steps = 0;
      for (steps = 0; steps < size_steps() && eventsAfter( steps ); steps += mPageSize) {}
      return steps;
   }
public:
   inline void update( double t, Action& cb )
   {
      // emit a tick on the tempo...
      mWallClockLast = mWallClock == 0.0 ? t : mWallClock;
      mWallClock = t;
      mWallClockDelta = mWallClock - mWallClockLast;
      mTimeUntilNextStep -= mWallClockDelta;
      mTick = false;
      if (mPlaying)
      {
         if (mTimeUntilNextStep <= 0.0)
         {
            mTimeUntilNextStep = mTempo;
            mTick = true;
         }
      }
      else
      {
         mTimeUntilNextStep = 0.0f;
      }

      if (mTick)
      {
         size_t steps = computeNumberOfStepsFilled( mPageSize );
         mCurrentStep = (mCurrentStep+1) % steps;

         if (mUseDebug) debugprint();

         // execute the step
         for (int x = 0; x < size_tracks(); ++x)
         {
            if (mSeqTrack[x][mCurrentStep].trigger)
            {
               // set the frequency/pitch for the track sound, then trigger the track sound
               cb.accent( x, mSeqTrack[x][mCurrentStep].accent );
               cb.ratio( x, mSeqTrack[x][mCurrentStep].ratio );
               cb.trigger( x );
            }
         }
      }
   }

   int debugprint_laststep;
   void debugprint()
   {
      if (debugprint_laststep == mCurrentStep)
      {
         return;
      }
      debugprint_laststep = mCurrentStep;

      size_t steps = computeNumberOfStepsFilled( mPageSize );
      printf( "step sequencer [steps:%ld|active:%ld|tracks:%ld]\n", size_steps(), steps, size_tracks() );
      for (int x = size_tracks()-1; 0 <= x; --x)
      {
         printf( "(%02d).", x );
         for (int y = 0; y < size_steps(); ++y)
         {
            if (0 == (y % mStepsPerBeat))
               printf( "|" );
            if (y == mCurrentStep)
            {
               printf( "*" );
            }
            else
               if (mSeqTrack[x][y].trigger)
                  printf( "X" );
               else
                  printf( "-" );
         }
         printf( "|.\n" );
      }
      printf( "\n\n" );
   }

   // 69 is A4 - above C4 which is middle C
   // A4/69 is default unchanged pitch for the wavform...
   static float note2ratio( int v )
   {
      return 440.0f/syn::Math::note2freq( v );
   }

   void loadDemo()
   {
      // bd
      mSeqTrack[0][0].trigger = true;
      mSeqTrack[0][0].ratio = note2ratio( 71 );
      mSeqTrack[0][1].trigger = true;
      mSeqTrack[0][1].accent = 0.7;
      mSeqTrack[0][5].trigger = true;
      mSeqTrack[0][8].trigger = true;
      mSeqTrack[0][12].trigger = true;
      mSeqTrack[0][12].ratio = note2ratio( 72 );
      mSeqTrack[0][14].trigger = true;
      mSeqTrack[0][14].ratio = note2ratio( 72 );

      // sd
      mSeqTrack[1][2].trigger = true;
      mSeqTrack[1][6].trigger = true;
      mSeqTrack[1][10].trigger = true;

      // clap
      mSeqTrack[3][14].trigger = true;
      mSeqTrack[3][14].ratio = note2ratio( 71 );

      // hh
      mSeqTrack[2][0].trigger = true;
      mSeqTrack[2][0].accent = 1.0;
      mSeqTrack[2][3].trigger = true;
      mSeqTrack[2][3].accent = 0.7;
      mSeqTrack[2][5].trigger = true;
      mSeqTrack[2][5].accent = 1.0;
      mSeqTrack[2][7].trigger = true;
      mSeqTrack[2][7].accent = 0.7;
      mSeqTrack[2][8].trigger = true;
      mSeqTrack[2][8].accent = 1.0;
      mSeqTrack[2][10].trigger = true;
      mSeqTrack[2][10].accent = 0.7;
      mSeqTrack[2][12].trigger = true;
      mSeqTrack[2][12].accent = 1.0;
      mSeqTrack[2][14].trigger = true;
      mSeqTrack[2][14].accent = 0.7;
      mSeqTrack[2][14].ratio = note2ratio( 73 );
      mSeqTrack[2][15].trigger = true;
      mSeqTrack[2][15].accent = 1.0;
      mSeqTrack[2][15].ratio = note2ratio( 64 );

/*
      for (int x = 0; x < 12; ++x)
      {
         mSeqTrack[0][x].trigger = true;
         mSeqTrack[0][x].ratio = note2ratio( 69-12 + x*2 );
      }
      */
      /*
      for (int x = 69-24; x <= 69+36; ++x)
      {

         // 69 == A == 440
         printf( "r: %d %f\n", x, note2ratio( x ) );
      }
      */
   }
};


#endif

