
#include "common.h"
#include <iostream>
#include <stdio.h>
#include <cmath>
#include <samplerate.h>

#include "SndFileAudioIStream.h"
#include "WavAudioIStream.h"
#include "MpegAudioIStream.h"
#include "OggAudioIStream.h"
#include "AudioData.h"
#include "gio.h"
#include "StepSequencer.h"
#include "NullAudioInterface.h" // audio out
#ifdef USE_PORTAUDIO
#include "PaAudioInterface.h" // audio out
#endif
#ifdef USE_SDLAUDIO
#include "SDLAudioInterface.h" // audio out
#endif
#include "Gr.h"


// search locations for .wav/mp3 files and .seq files.
const char* base_paths[] = { "/mnt/usb", "/mnt/usb2", "/Volumes/BOOMBOOMPOW", "./testwav" };

// audio
AudioData track[4];
const size_t num_tracks = sizeof( track )/sizeof( AudioData );
AudioDataSampleRateIterator itrack[num_tracks];
//AudioDataIterator itrack[num_tracks];

struct StepSequencerAction : public StepSequencer::Action
{
   virtual ~StepSequencerAction() {}
   virtual void trigger( int tracknum ) { itrack[tracknum].reset(); }
   virtual void ratio( int tracknum, double r ) { itrack[tracknum].setRatio( r ); }
   virtual void accent( int tracknum, double a ) { itrack[tracknum].setVolume( a ); }
};
StepSequencer seq;
StepSequencerAction seqaction;

// application
enum
{
   BBP_START_BUTTON, BBP_STOP, BBP_PAUSE_BUTTON, BBP_SEQ_START, BBP_SEQ_SUSPEND, BBP_SEQ_RESUME, BBP_AUDIO_OFF, BBP_AUDIO_ON,
   BBP_TEMPO_FASTER, BBP_TEMPO_SLOWER, BBP_TRACKBUTTON,
};
struct Event
{
   Event() {}
   Event( int t, int x=0, int y=0 ) : type(t), data(x), data2(y) {}
   int type;
   size_t data, data2;
};
Event events[64];
const int events_size = sizeof( events ) / sizeof( Event );
int num_events = 0;
void bbpFireevent( Event ev )
{
   if (num_events < events_size)
   {
      events[num_events] = ev;
      ++num_events;
   }
}
bool audio_enabled = true;


// return 0 for success, 1 for fail
AudioIStream* openAudioFile( const char* audioFileName )
{
   AudioIStream* s = NULL;

   std::string ext = gio::tolower( gio::justExt( audioFileName ) );
   if (ext == "wav")
   {
      s = new WavAudioIStream( audioFileName );
   }
   else if (ext == "mp3")
   {
      s = new MpegAudioIStream( audioFileName );
   }
   else if (ext == "ogg")
   {
      s = new OggAudioIStream( audioFileName );
   }
   else
   {
      printf( "unknown extension in %s\n", audioFileName );
   }
   return s;
}

bool findAudioFile( std::string& filename, const char* path, const char* name )
{
   const char *exts[] = { ".ogg", ".mp3", ".flac", ".wav"  };
   const int num_exts = sizeof(exts)/sizeof(const char *);
   for (int x = 0; x < num_exts; ++x)
   {
      filename = std::string( name ) + exts[x];
      if (gio::fileExistsi( filename, path, filename.c_str() ))
         return true;
   }
   filename = name;
   return false;
}

void bbpClearBank()
{
   for (int x = 0; x < num_tracks; ++x)
   {
      track[x].clear();
      itrack[x].init( track[x] );
   }
}

std::string bbpFindBasePath()
{
   const int num_base_paths = sizeof( base_paths )/sizeof( const char * );
   std::string base_path;
   printf( "searching for soundbank directory...\n" );
   for (int x = 0; x < num_base_paths; ++x)
   {
      printf( " - in %s... ", base_paths[x] );
      if (opendir( base_paths[x] ) != NULL)
      {
         printf( " [found!]" );
         base_path = base_paths[x];
         x = num_base_paths;
      }
      printf( "\n" );
   }
   if (base_path == "")
   {
      printf( "!!!!! no sound banks found... [aborting]\n" );
      exit( -1 );
   }

   return base_path;
}

void bbpLoadBank( const char* bankname = "" )
{
   std::string base_path = bbpFindBasePath();
   std::string path = gio::join( '/', base_path.c_str(), bankname, NULL );
   for (int x = 0; x < num_tracks; ++x)
   {
      if (0 == track[x].size())
      {
         char trackname[2] = " ";
         trackname[0] = x + 'A';
         char trackname2[2] = " ";
         trackname2[0] = x + '1';
         std::string filename;
         if (findAudioFile( filename, path.c_str(), trackname ) ||
             findAudioFile( filename, path.c_str(), trackname2 ))
         {
            filename = gio::join( '/', path.c_str(), filename.c_str(), NULL );
             printf( "%s\n", filename.c_str());
             //look up ExtAudioFile for reading wav files on ios...

             AudioIStream* s = openAudioFile( filename.c_str() );
            s->showinfo();
            printf( "reading %d frames, %d channels\n", s->frames(), s->channels() );
            track[x].init( (float)s->samprate(), s->frames(), s->channels() );
            s->read_float( track[x].data(), s->samples() );
            printf( "loaded...\n" );
            delete s;

            // samprate conversion
            if (track[x].samprate() != 44100.0f)
            {
               AudioData temp;
               float ratio = 44100.0f / track[x].samprate();
               temp.init( 44100.0f, track[x].frames() * ratio, track[x].channels() );
               printf( "*** resampling to 44100\n" );
               SRC_DATA data;
               data.src_ratio = ratio;
               data.input_frames = track[x].frames();
               data.output_frames = temp.frames();
               data.data_in = track[x].data();
               data.data_out = temp.data();
               if (src_simple (&data, SRC_SINC_FASTEST, track[x].channels()))
               {
                  printf( "*** conversion %0.0f -> %0.0f failed.\n\n", track[x].samprate(), temp.samprate() );
               }
               else
               {
                  printf( "*** converted %ld frames to %ld frames\n", data.input_frames_used, data.output_frames_gen );
                  track[x] = temp;
               }
            }
            if (track[x].channels() == 1)
            {
               AudioData temp;
               temp.init( track[x].samprate(), track[x].frames(), 2 );
               printf( "*** converting mono to stereo\n" );
               const int frames = track[x].frames();
               float* data = track[x].data();
               for (int i = 0; i < frames; ++i)
               {
                  temp.data()[i*2 + 0] = data[i];
                  temp.data()[i*2 + 1] = data[i];
               }
               printf( "*** converted %ld samples (%ld channel) to %ld samples (%ld channel)\n",
                       track[x].samples(), track[x].channels(), temp.samples(), temp.channels() );
               track[x] = temp;
            }
         }
         else
         {
            filename = gio::join( '/', path.c_str(), filename.c_str(), NULL );
            printf( "%s.* not found\n", filename.c_str() );
         }
      }
   }

   // set up iterators
   for (int x = 0; x < num_tracks; ++x)
      itrack[x].init( track[x] );
}

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

bool bbpPreviousPlaybackState = false;
bool bbpPreviousPlaybackStatePresent = false;
int bbpAudioUpdateBuffer( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                         const double currentTime, AudioInterface::Format format,
                           void *userData )
{
   StepSequencer& seq = *(StepSequencer*)userData;
   (void) inputBuffer; /* Prevent unused variable warnings. */

   // DEBUG sin wave!
   /*
   static float bok = 0;
   short* out = (short*)outputBuffer;
   for (int i = 0; i < framesPerBuffer; ++i)
   {
      bok += 1.0f/44100.0f;
      float w = sinf(2.0f * 3.14159265359f * 220.0f * bok) * 0.5f;
      *out++ = (short)floorf(w * 32767.0f);
      *out++ = (short)floorf(w * 32767.0f);
   }
   return 0;
   */

   // event handling
   int size_events = num_events;
   while (num_events)
   {
      int event_index = size_events - num_events;
      assert( 0 <= event_index && event_index < size_events );
      int x = events[event_index].data;
      int y = events[event_index].data2;
      switch (events[event_index].type)
      {
         case BBP_START_BUTTON:
            seq.mPlaying = !seq.mPlaying;
            if (seq.mPlaying)
               seq.mCurrentStep = seq.size_steps()-1;
            else
               seq.mCurrentStep = 0;
            break;
         case BBP_PAUSE_BUTTON:
            seq.mPlaying = !seq.mPlaying;
            break;
         case BBP_SEQ_START:
            seq.mPlaying = true;
            seq.mCurrentStep = seq.size_steps()-1;
            break;
         case BBP_SEQ_SUSPEND:
            bbpPreviousPlaybackState = seq.mPlaying;
            bbpPreviousPlaybackStatePresent = true;
            seq.mPlaying = false;
            break;
         case BBP_SEQ_RESUME:
            if (bbpPreviousPlaybackStatePresent)
            {
               seq.mPlaying = bbpPreviousPlaybackState;
               bbpPreviousPlaybackState = false;
               bbpPreviousPlaybackStatePresent = false;
            }
            break;
         case BBP_AUDIO_OFF: audio_enabled = false; break;
         case BBP_AUDIO_ON: audio_enabled = true; break;
         case BBP_TEMPO_FASTER: seq.mTempo += 5.0f; break;
         case BBP_TEMPO_SLOWER: seq.mTempo -= 5.0f; break;
         case BBP_TRACKBUTTON:
            seq.mSeqTrack[x][y].trigger = !seq.mSeqTrack[x][y].trigger;
            if (seq.mSeqTrack[x][y].trigger)
            {
               seqaction.trigger( x );
            }
            break;
      }
      --num_events;
   }

   // seq logic
   if (audio_enabled)
   {
      seq.update( currentTime, seqaction );
   }

   // mix the audio
   switch (format)
   {
      case AudioInterface::F32:
      {
         float *out = (float*)outputBuffer;
         // bail if disabled
         if (!audio_enabled)
         {
            for (int i = 0; i < framesPerBuffer; ++i)
            {
               *out++ = 0.0f;
               *out++ = 0.0f;
            }
            return 0;
         }
         // mix
         float o1, o2;
         for (int i = 0; i < framesPerBuffer; ++i)
         {
            o1 = 0.0f; o2 = 0.0f;
            for (size_t t = 0; t < num_tracks; ++t)
            {
               itrack[t].incPlayOnce();
               o1 += (*itrack[t])[0];  // left
               o2 += (*itrack[t])[1];  // right
            }
            *out++ = o1;
            *out++ = o2;
         }
      }
      break;
      case AudioInterface::S16LSB:
      {
         short *out = (short*)outputBuffer;
         // bail if disabled
         if (!audio_enabled)
         {
            for (int i = 0; i < framesPerBuffer; ++i)
            {
               *out++ = 0;
               *out++ = 0;
            }
            return 0;
         }
         // mix
         float o1, o2;
         short so1, so2;
         for (int i = 0; i < framesPerBuffer; ++i)
         {
            o1 = 0.0f; o2 = 0.0f; // comment out for some aphex twin... :)
            for (size_t t = 0; t < num_tracks; ++t)
            {
               itrack[t].incPlayOnce();
               o1 += (*itrack[t])[0];  // left
               o2 += (*itrack[t])[1];  // right
            }
            so1 = (short)floorf(o1 * 32767.0f);
            so2 = (short)floorf(o2 * 32767.0f);
            *out++ = so1;
            *out++ = so2;
         }
      }
      break;
      default:
         printf( "unknown format in bbpAudioUpdateBuffer\n" );
         exit(0);
   }
   return 0; // never finished (return 1 if finished)
}

//Gr::State flashing[256]
//int flashing_queue = 0;

int bbpCurrentPageShowing()
{
   int page = seq.curPage();
   if (0 <= seq.mDisplayPage)
      page = seq.mDisplayPage;
   return page;
}

int bbpLightsFunc( void* userdata )
{
   if (NULL == userdata) return 0;

   AudioInterface& audiointerface = *(AudioInterface*)userdata;
   if (audiointerface.isActive())
   {
      audiointerface.update();
      //audiointerface.sleep( 100 );

      int tempolight = seq.mCurrentStep % seq.mStepsPerBeat;
      bool fullTempo = tempolight == 0;
      bool halfTempo = seq.mTimeUntilNextStep <= (seq.mTempo*0.5f);
      gr.setTempoBlinker( fullTempo, halfTempo );

      if (seq.mTick)
      {
         if (seq.mPlaying)
            gr.setState( Gr::Lp, Gr::Widget::LIT );
         else
            gr.setState( Gr::Lp, Gr::Widget::NORMAL );
      }

      static int laststep = -99;
      if (laststep == seq.mCurrentStep)
      {
         return 0;
      }
      laststep = seq.mCurrentStep;
      if (seq.mPlaying)
         gr.setState( Gr::Lt, Gr::Widget::TBLINK );
      else
         gr.setState( Gr::Lt, Gr::Widget::NORMAL );

      int page = bbpCurrentPageShowing();
      for (int x = seq.size_tracks()-1; 0 <= x; --x)
      {
         for (int yy = 0; yy < seq.mPageSize; ++yy)
         {
            //(seq.size_steps() / seq.mPageSize);
            int y = page * seq.mPageSize + yy;

            bool lit = false;
            if (y == seq.mCurrentStep)
               lit = true;
            else
               if (seq.mSeqTrack[x][y].trigger)
                  lit = true;
               else
                  lit = false;

            gr.setState( gr.tracks_lightIDs[x][yy],
                          lit ? Gr::Widget::LIT : Gr::Widget::NORMAL, yy );
            //printf( "%d %d %d %d [%d]\n", yy, y, x, gr.tracks_lightIDs[x][yy], (int)lit );
         }
      }

      Gr::Widget::State pageState = Gr::Widget::LIT;
      if (seq.mDisplayPage < 0)
         pageState = Gr::Widget::BLINK;
      for (int x = 0; x < 4; ++x)
         gr.setState( Gr::Lpage, page == x ? pageState : Gr::Widget::NORMAL, x );
      Gr::Widget* w = gr.findWidget( Gr::bg );
      if (w)
      {
         if (seq.mDisplayPage < 0)
            w->setPage( 0 );
         else
            w->setPage( 1 + seq.mDisplayPage );
      }
   }
   return 0;
}

void bbpButtonsFunc( Gr::ButtonsAndLights bnl, void* userdata, int id = -1 )
{
   if (NULL == userdata) return;
   int page = bbpCurrentPageShowing();

   // reset the selection, if no note, or note modifier, is touched
   switch (bnl)
   {
      case Gr::Ba1:case Gr::Ba2:case Gr::Ba3:case Gr::Ba4:case Gr::Ba5:case Gr::Ba6:case Gr::Ba7:case Gr::Ba8:
      case Gr::Bb1:case Gr::Bb2:case Gr::Bb3:case Gr::Bb4:case Gr::Bb5:case Gr::Bb6:case Gr::Bb7:case Gr::Bb8:
      case Gr::Bc1:case Gr::Bc2:case Gr::Bc3:case Gr::Bc4:case Gr::Bc5:case Gr::Bc6:case Gr::Bc7:case Gr::Bc8:
      case Gr::Bd1:case Gr::Bd2:case Gr::Bd3:case Gr::Bd4:case Gr::Bd5:case Gr::Bd6:case Gr::Bd7:case Gr::Bd8:
      case Gr::La1:case Gr::La2:case Gr::La3:case Gr::La4:case Gr::La5:case Gr::La6:case Gr::La7:case Gr::La8:
      case Gr::Lb1:case Gr::Lb2:case Gr::Lb3:case Gr::Lb4:case Gr::Lb5:case Gr::Lb6:case Gr::Lb7:case Gr::Lb8:
      case Gr::Lc1:case Gr::Lc2:case Gr::Lc3:case Gr::Lc4:case Gr::Lc5:case Gr::Lc6:case Gr::Lc7:case Gr::Lc8:
      case Gr::Ld1:case Gr::Ld2:case Gr::Ld3:case Gr::Ld4:case Gr::Ld5:case Gr::Ld6:case Gr::Ld7:case Gr::Ld8:
      case Gr::FreqUp:
      case Gr::FreqDown:
         break;
      default:
         printf( "RESETTING SELECTION\n" );
            gr.setStepSelection( Gr::o );
   }

   //printf( "bnl:%d\n", (int)bnl );
   switch (bnl)
   {
      case Gr::Pwr:
         gr.Quit();
         break;
      case Gr::TC:
         printf( "change theme button pressed...\n" );
         gr.changeTheme();
         break;
      case Gr::BtU:
         printf( "tempo up\n" );
         seq.setTempoBPM( seq.getTempoBPM() + 10, seq.mStepsPerBeat );
         break;
      case Gr::BtD:
         printf( "tempo down\n" );
         seq.setTempoBPM( seq.getTempoBPM() - 10, seq.mStepsPerBeat );
         break;
      case Gr::Bps:
         printf( "playstop\n" );
         seq.play( !seq.mPlaying );
         gr.setState( Gr::Lp, seq.mPlaying ? Gr::Widget::LIT : Gr::Widget::NORMAL );
         break;
      case Gr::Bplay:
         printf( "play\n" );
         seq.play();
         gr.setState( Gr::Lp, seq.mPlaying ? Gr::Widget::LIT : Gr::Widget::NORMAL );
         break;
      case Gr::LaT:
         printf( "trigger track[%d] sound\n", id );
         seq.trigTrackSound( id, 1.0, seqaction );
         break;
      case Gr::Lbang:
         printf( "trigger step[%d] sound\n", id );
         if (seq.mPlaying) seq.playStepOnPage( id, page ); else seq.trigStepOnPage( id, page, seqaction );
         break;
      case Gr::Bstop:
         printf( "stop\n" );
         seq.play( false );
         gr.setState( Gr::Lp, seq.mPlaying ? Gr::Widget::LIT : Gr::Widget::NORMAL );
         break;
      case Gr::Bpause:
         printf( "pause\n" );
         seq.pause();
         gr.setState( Gr::Lp, seq.mPlaying ? Gr::Widget::LIT : Gr::Widget::NORMAL );
         break;
      case Gr::Bpp:
         printf( "playpause\n" );
         seq.pause( seq.mPlaying ); // if playing pause it, if not playing unpause it
         gr.setState( Gr::Lp, seq.mPlaying ? Gr::Widget::LIT : Gr::Widget::NORMAL );
         break;

      case Gr::bg:
         seq.mDisplayPage = (seq.numPages() - 1);
         goto PageButton;
      case Gr::Lpage: // specific page button
         printf( "specific page button %d\n", id );
         if (seq.mDisplayPage == id) seq.mDisplayPage = (seq.numPages() - 1);
         else seq.mDisplayPage = id-1;
         goto PageButton;
PageButton:
      case Gr::Bpage: // page cycle button
         printf( "page cycle button %d + 1\n", seq.mDisplayPage );
         if (seq.mDisplayPage < 0)
            seq.mDisplayPage = 0;//seq.curPage();
         else if (seq.mDisplayPage == (seq.numPages() - 1))
            seq.mDisplayPage = -1;
         else
            seq.mDisplayPage = (seq.mDisplayPage + 1);
         printf( "                - %d\n", seq.mDisplayPage );
         for (int x = 0; x < 4; ++x)
            gr.setState( Gr::Lpage, seq.mDisplayPage == x ? Gr::Widget::LIT : Gr::Widget::NORMAL, x );
         /*Gr::Widget* w = gr.findWidget( Gr::bg );
         if (w)
         {
            if (seq.mDisplayPage == -1)
               w->setPage( 0 );
            else
               w->setPage( 1 + seq.mDisplayPage );
         }*/
         break;
      case Gr::FreqUp:
      case Gr::FreqDown:
         printf( "FREQUENCY CHANGE:\n" );
         if (gr.getStepSelection() != Gr::o)
         {
            int displaystep, track;
            int page = bbpCurrentPageShowing();
            bool result = gr.getButtonXY( gr.getStepSelection(), displaystep, track );
            if (result)
            {
               int step = displaystep + page * seq.mPageSize;
               seq.ratio( step, track ) *= (bnl == Gr::FreqDown ? 13.0f/12.0f : 11.0f/12.0f);
               if (seq.ratio( step, track ) < 0.1)
                  seq.ratio( step, track ) = 0.1;
               //gr.incFreq( held, seq.ratio( step, track ) );
               seq.trigTrackSound( track, seq.ratio( step, track ), seqaction );

               printf( "%s: bnl:%d  result:%d  track:%d | displaystep:%d ratio:%f\n",
                  bnl == Gr::FreqUp ? "freq+": "freq-",
                  (int)gr.getStepSelection(), result, track, displaystep,
                  seq.ratio( step, track ) );
            }
         }
         break;
      default:
         {
            int displaystep, track;
            int page = bbpCurrentPageShowing();
            bool result = gr.getButtonXY( bnl, displaystep, track );
            printf( "bnl:%d  result:%d  track:%d | displaystep:%d\n", (int)bnl, result, track, displaystep );
            if (result)
            {
               int step = displaystep + page * seq.mPageSize;
               //printf( "step:%d  track:%d | displaystep:%d\n", step, track, displaystep );
               seq.trigger( step, track ) = !seq.trigger( step, track );
               if (seq.trigger( step, track ))
               {
                  seq.trigTrackSound( track, seq.ratio( step, track ), seqaction );
               }
               //seq.ratio( step, track ) = 1.0;
               gr.setState( bnl,
                            seq.trigger( step, track ) ? Gr::Widget::LIT : Gr::Widget::NORMAL,
                            id );

               printf( "SETTING SELECTION\n" );
               gr.setStepSelection( bnl, id );
            }
         }
         break;
   }
}

void bbpInitSeq()
{
   std::string base_path = bbpFindBasePath();
   std::string seqfile = gio::join( '/', base_path.c_str(), "seq.seq", NULL );
   std::string defaultfile = gio::join( '/', base_path.c_str(), "default.seq", NULL );
   if (gio::fileExists( seqfile.c_str() ))
   {
      seq.load( seqfile.c_str() );
   }
   else
   {
      if (!gio::fileExists( defaultfile.c_str() ))
      {
         printf( "where is default.seq??? saving out a copy for ya.\n" );
         seq.save( defaultfile.c_str() );
      }
      seq.load( defaultfile.c_str() );
      seq.save( seqfile.c_str() );
   }
}

AudioInterface* audiointerface = NULL;
void bbpInitFunc()
{
}

void bbpSuspend()
{
   bbpFireevent( BBP_SEQ_SUSPEND );
}

void bbpResume()
{
   bbpFireevent( BBP_SEQ_RESUME );
}
#if defined __APPLE__
#include <CoreFoundation/CoreFoundation.h>
std::string getBundleDir()
{
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
    char path[PATH_MAX];
    if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX))
        return "";
    CFRelease(resourcesURL);
    //chdir(path);
    return path;
}
void initWorkingDir()
{
   chdir( getBundleDir().c_str() );
}
#else
void initWorkingDir() {}
#endif
int main(int argc, char *argv[])
{
   bool windowed = true;
   for (int x = 1; x < argc; ++x)
   {
      if (strcmp( argv[x], "-f" ) == 0)
      {
         windowed = false;
      }
   }

   initWorkingDir();
   std::string cwd = gio::cwd();
   //FILE* f = fopen( "/Users/kevin/boomboompow/bok", "w" );
   //fwrite( cwd.c_str(), cwd.size(), 1, f );
   //fwrite( getBundleDir().c_str(), getBundleDir().size(), 1, f );
   //fclose( f );

   bbpClearBank();
   bbpLoadBank( "1" );  // default to bank in the 1 directory
   bbpLoadBank();       // fallback to root folder, fill in any remaining blank slots
   seq.loadDemo();
   seq.useDebug( false );

   bbpInitSeq();

   bbpFireevent( BBP_SEQ_START );

   audiointerface = new NullAudioInterface;
#ifdef USE_PORTAUDIO
   delete audiointerface;
   audiointerface = new PaAudioInterface;
#endif
#ifdef USE_SDLAUDIO
   delete audiointerface;
   audiointerface = new SDLAudioInterface;
#endif
   audiointerface->init( bbpAudioUpdateBuffer, &seq );

   gr.run( argc, argv, bbpInitFunc, bbpLightsFunc, bbpButtonsFunc, bbpSuspend, bbpResume, audiointerface, windowed );
   audiointerface->close();

   return 0;
}

#ifdef _WIN32
#ifndef _CONSOLE

int WINAPI WinMain(
  HINSTANCE /*instance*/,
  HINSTANCE /*prev_instance*/,
  LPSTR     /*command_line*/,
  int       /*show_command*/)
{
  return main(__argc, __argv);
}

#endif
#endif
