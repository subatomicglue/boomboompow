
#include "common.h"
#ifdef WIN32
   #define NOMINMAX
   #include <windows.h>  // make the app win32 friendly. :)
#endif
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <functional>
#include <locale>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <map>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h> // for sleep

#include <EASTL/vector.h>
#include <EASTL/map.h>

#include <corona/corona.h>

#include <synMath.h>

#include "Actor.h"
#include "Level.h"
#include "Gfx.h"

#include "gio.h"
#include "BuiltInText.h"


// EASTL expects us to define these, see allocator.h line 194
void* operator new[](size_t size, const char* pName, int flags,
    unsigned debugFlags, const char* file, int line)
{
    return malloc( size );
}
void* operator new[](size_t size, size_t alignment, size_t alignmentOffset,
    const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
    // this allocator doesn't support alignment
    EASTL_ASSERT(alignment <= 8);
    return malloc( size );
}

// EASTL also wants us to define this (see string.h line 197)
int Vsnprintf8(char8_t* pDestination, size_t n, const char8_t* pFormat, va_list arguments)
{
    #ifdef _MSC_VER
        return _vsnprintf(pDestination, n, pFormat, arguments);
    #else
        return vsnprintf(pDestination, n, pFormat, arguments);
    #endif
}


#define MILLESECONDS_PER_FRAME 16       /* about 60 frames per second */
bool bbpSDLredisplay = false;
void bbpSDLpostredisplay()
{
   bbpSDLredisplay = true;
}

static bool gIsRunning = true;
SDL_Renderer *renderer = NULL;

static void renderTextWithShadow( float r, float g, float b, const char* text, float x, float y, float z, float size, float offset )
{
#if !TARGET_OS_IPHONE && !defined(EXCLUDE_GLEW)
   glColor3f( 0.0f,0.0f,0.0f );
   BuiltInText::renderUpsideDown( text, offset+x,offset+y,z,size );
   glColor3f( r, g, b );
   BuiltInText::renderUpsideDown( text, x,y,z,size );
#endif
}

// top is > bottom
// right is > left
static void drawColorRect( float* topcolor, float* bottomcolor,
                  float l, float t, float b, float r )
{
   glDisable( GL_DEPTH_TEST );
   glDisable( GL_TEXTURE_2D );
#if !TARGET_OS_IPHONE && !defined(EXCLUDE_GLEW)
   glBegin( GL_TRIANGLES );
      glColor4fv( bottomcolor );
      glVertex3f( l, b, 0 );
      glColor4fv( topcolor );
      glVertex3f( l, t, 0 );
      glColor4fv( topcolor );
      glVertex3f( r, t, 0 );
      glColor4fv( bottomcolor );
      glVertex3f( r, b, 0 );
      glColor4fv( bottomcolor );
      glVertex3f( l, b, 0 );
      glColor4fv( topcolor );
      glVertex3f( r, t, 0 );
   glEnd();
#endif
}

class Gr;
extern Gr gr;

// a place to store application data...
class Gr
{
public:
   Gr() : mLevel( scene )
   {
      width = 100; // non zero
      height = 100; // non zero
      mainWin_contextID = -1;
      once = true;
      windowscalar = 1.0f;
      ButtonsAndLights b[4][8] = { {Ba1,Ba2,Ba3,Ba4,Ba5,Ba6,Ba7,Ba8},
                                   {Bb1,Bb2,Bb3,Bb4,Bb5,Bb6,Bb7,Bb8},
                                   {Bc1,Bc2,Bc3,Bc4,Bc5,Bc6,Bc7,Bc8},
                                   {Bd1,Bd2,Bd3,Bd4,Bd5,Bd6,Bd7,Bd8} };
      ButtonsAndLights l[4][8] = { {La1,La2,La3,La4,La5,La6,La7,La8},
                                   {Lb1,Lb2,Lb3,Lb4,Lb5,Lb6,Lb7,Lb8},
                                   {Lc1,Lc2,Lc3,Lc4,Lc5,Lc6,Lc7,Lc8},
                                   {Ld1,Ld2,Ld3,Ld4,Ld5,Ld6,Ld7,Ld8} };
      for (int y = 0; y < 4; ++y)
      {
         for (int x = 0; x < 8; ++x)
         {
            tracks_buttonIDs[y][x] = b[y][x];
            tracks_lightIDs[y][x] = l[y][x];
         }
      }

      mUpdateLightsFunc = NULL;
      mButtonPressFunc = NULL;
      mUserData = NULL;
      mBlinkToggle = true;
      mBlinkTimer = 0.0f;
      setTempoBlinker( false, false );

      selected = o;
      selectedID = -1;
      gameProjToWorld = vmMatrix4::identity();
      //init();
   }

   // call this when ever the values change, to make BLINK mode work.
   void setTempoBlinker( bool fullTempo, bool halfTempo )
   {
      mTempoBlinkToggle = fullTempo;
      mHalfTempoBlinkToggle = halfTempo;
   }

   enum ButtonsAndLights
   {
      beginning = 0, //nothing

      BaT,Ba1,Ba2,Ba3,Ba4,Ba5,Ba6,Ba7,Ba8,
      BbT,Bb1,Bb2,Bb3,Bb4,Bb5,Bb6,Bb7,Bb8,
      BcT,Bc1,Bc2,Bc3,Bc4,Bc5,Bc6,Bc7,Bc8,
      BdT,Bd1,Bd2,Bd3,Bd4,Bd5,Bd6,Bd7,Bd8,
      LaT,La1,La2,La3,La4,La5,La6,La7,La8,
      LbT,Lb1,Lb2,Lb3,Lb4,Lb5,Lb6,Lb7,Lb8,
      LcT,Lc1,Lc2,Lc3,Lc4,Lc5,Lc6,Lc7,Lc8,
      LdT,Ld1,Ld2,Ld3,Ld4,Ld5,Ld6,Ld7,Ld8,
      Lbang,

      BtU, Lt, BtD, // tempo
      Bps, Lp, Bpp, Bplay, Bpause, Bstop, // play/stop/pause
      Lpage, Bpage, // pages
      TC, Pwr, bg,
      FreqUp, FreqDown, Sel, // +note, -note, selection graphic
      o, // nothing
   };
   const char* getButtonText( ButtonsAndLights bnl ) const
   {
      const char* t[] = {
         "beginning",

         "BaT","Ba1","Ba2","Ba3","Ba4","Ba5","Ba6","Ba7","Ba8",
         "BbT","Bb1","Bb2","Bb3","Bb4","Bb5","Bb6","Bb7","Bb8",
         "BcT","Bc1","Bc2","Bc3","Bc4","Bc5","Bc6","Bc7","Bc8",
         "BdT","Bd1","Bd2","Bd3","Bd4","Bd5","Bd6","Bd7","Bd8",
         "LaT","La1","La2","La3","La4","La5","La6","La7","La8",
         "LbT","Lb1","Lb2","Lb3","Lb4","Lb5","Lb6","Lb7","Lb8",
         "LcT","Lc1","Lc2","Lc3","Lc4","Lc5","Lc6","Lc7","Lc8",
         "LdT","Ld1","Ld2","Ld3","Ld4","Ld5","Ld6","Ld7","Ld8",
         "Lbang",

         "BtU","Lt","BtD",
         "Bps","Lp","Bpp", "Bplay", "Bpause", "Bstop",
         "Lpage","Bpage",
         "TC", "Pwr", "bg",
         "FreqUp", "FreqDown", "Sel",
         "o"};
      return t[bnl];
   }
   ButtonsAndLights getButtonEnum( const char* name ) const
   {
      for (int64_t x = beginning; x < o; ++x)
      {
         if (strcmp( name, getButtonText( (ButtonsAndLights)x )) == 0)
         {
            return (ButtonsAndLights)x;
         }
      }
      return o;
   }

   ButtonsAndLights tracks_buttonIDs[4][8]; // track mapping for buttons and lights.
   ButtonsAndLights tracks_lightIDs[4][8];  // track mapping for buttons and lights.
   ButtonsAndLights selected;
   int selectedID;

   // return true if found. x and y will be set.
   bool getButtonXY( ButtonsAndLights bnl, int &step, int &track )
   {
      for (step = 0; step < 8; ++step)
      {
         for (track = 0; track < 4; ++track)
         {
            if (tracks_lightIDs[track][step] == bnl)
               return true;
            if (tracks_buttonIDs[track][step] == bnl)
               return true;
         }
      }
      return false;
   }
   void init()
   {
   }
   void reBindWidgets()
   {
      mWidgets.clear();

      std::list<Actor*>::iterator it;
      for (it = mLevel.mNewActors.begin(); it != mLevel.mNewActors.end(); ++it)
      {
         Actor* a = *it;
         if (a)
            addWidget( Widget( getButtonEnum( a->mName.c_str() ), a->mID, a, this ) );
         printf( "%s[%d]\n", a->mName.c_str(), a->mID );
      }
      for (it = mLevel.mActors.begin(); it != mLevel.mActors.end(); ++it)
      {
         Actor* a = *it;
         if (a)
            addWidget( Widget( getButtonEnum( a->mName.c_str() ), a->mID, a, this ) );
         printf( "%s[%d]\n", a->mName.c_str(), a->mID );
      }
      /*
      for (ButtonsAndLights i = beginning; i < o; ++i)
      {
         Actor* a = mLevel.Lookup( getButtonText( i ) );
         if (a)
            addWidget( Widget( i, a ) );
      }
       */
   }

   void setTheme( const char* which )
   {
      // delete graphics objects
      mLevel.graphicsRelease();
      ctx.graphicsRelease();

      // release memory
      mLevel.release();
      ctx.release();

      // load new level
      std::string previous = gio::cwd();
      gio::chdir( which );
      mLevel.deserialize( ctx.GetFile( "level.level" ), ctx );
      gio::chdir( previous.c_str() );

      // recreate UI bindings.
      reBindWidgets();
   }

   // theme toggle, round robbin
   void changeTheme()
   {
      static int n = 0;
      const char* which[] = { "kid", "kid2", "pro" };
      n = (n+1) % (sizeof(which) / sizeof(char*));
      printf( "changing Theme to %d %s\n", n, which[n] );
      setTheme( which[n] );
   }

   void graphicsInit()
   {
      const char *version = (char*)glGetString(GL_VERSION);
      printf( "initializing graphics for 'OpenGL %s'\n", version );
      printf( "==============================================\n" );
      setTheme( "kid2" );
   }


   void release()
   {
   }

   float windowscalar;
   float width, height;
   int mainWin_contextID;
   bool once;
   std::vector<std::string> argv;
   int argc;
   bool mInitialized;
   struct Touch
   {
      enum TouchPhase { BEGIN, MOVING, END };
      float deltaPosition[2];  // The position delta since last change.
      float deltaTime;  // Amount of time that has passed since the last recorded change in Touch values.
      int fingerId;  // The unique index for the touch.
      TouchPhase phase;  // Describes the phase of the touch.
      int position[2];  // The position of the touch in pixel coordinates.
      int tapCount;  // Number of taps.

      void setPos( float xx, float yy, bool isdown ) { x = xx; y = yy; m = isdown; }
      float x, y;
      bool m;
   };
   //std::vector<Touch> touches;
   typedef std::map<int, Touch> MultiTouchMap;
   MultiTouchMap touches;

   struct Widget
   {
      Widget( ButtonsAndLights e, int id = -1, Actor* actor = NULL, Gr* gr = NULL )
      {
         bnl = e;
         state = NORMAL;
         a = actor;
         this->id = id;
         this->gr = gr;
         if (NULL != a)
         {
            //printf( "UI binding to actor %s::%s\n", a->GetType().c_str(), a->GetName().c_str() );
            float b1[2], b2[2];
            a->GetBoundingBox( b1, b2 );
            size[0] = b2[0] - b1[0];
            size[1] = b2[1] - b1[1];
            pos[0] = b1[0];
            pos[1] = b1[1];
         }
         selected = false;
      }
      Actor* a;
      Gr* gr;
      ButtonsAndLights bnl;
      int id;
      float size[2], pos[2];
      typedef void (*OnButton)(ButtonsAndLights b, void*, int );
      OnButton mOnButton;
      void* mOnButtonUserData;
      bool selected, mouseover;

      enum State { NORMAL, CLICKED, LIT, BLINK, TBLINK, HTBLINK };
      State state; // 0 normal, 1 click, 2 lit

      bool intersect( float x, float y ) const
      {
         return pos[0] <= x && x <= (pos[0]+size[0]) &&
                pos[1] <= y && y <= (pos[1]+size[1]);
      }

      bool fingerIntersects( const MultiTouchMap& mtm ) const
      {
         bool i = false;
         for (MultiTouchMap::const_iterator it = mtm.begin();
             it != mtm.end(); ++it)
         {
            i |= intersect( it->second.x, it->second.y );
         }
         return i;
      }

      // return true if intersected
      bool mouseclick( int id, float x, float y, bool m,
                       const MultiTouchMap& mtm, bool another_was_hit )
      {
         bool i = fingerIntersects( mtm );
         if (i && m == true && !another_was_hit)
         {
            if (mOnButton)
            {
               mOnButton( bnl, mOnButtonUserData, this->id );
            }
         }
         return i;
      }

      // return true if intersected
      bool mousemove(int id, float x, float y, bool m, const MultiTouchMap& mtm, bool another_was_hit)
      {
         bool i = fingerIntersects( mtm );
         //gr->touches[id] = {x, y, m};

         //if (a) if (!a->mIsUI) return;
         if (i && !another_was_hit)
         {
            mouseover = true;
         }
         else
            mouseover = false;
         return i;
      }

      void setState( State s ) { state = s; }
      void setPage( size_t p )
      {
         if (a && !a->mIsUI)
         {
            a->showAllGraphics( false );
            a->ShowGraphic( p );
         }
      }

      void draw( bool blinkToggle, bool tBlinkToggle, bool htBlinkToggle )
      {
         if (a && a->mIsUI)
         {
            a->showAllGraphics( false );
            if (selected)
            {
               a->ShowGraphic( 5 );
            }
            if (mouseover)
            {
               a->ShowGraphic( 1 );
            }
            switch (state)
            {
               case NORMAL: a->ShowGraphic( 0 ); break;
               case CLICKED: a->ShowGraphic( 2 ); break;
               case LIT: a->ShowGraphic( 3 ); break;
               case BLINK: if (blinkToggle) a->ShowGraphic( 4 ); else a->ShowGraphic( 0 ); break;
               case TBLINK: if (tBlinkToggle) a->ShowGraphic( 4 ); else a->ShowGraphic( 0 ); break;
               case HTBLINK: if (htBlinkToggle) a->ShowGraphic( 4 ); else a->ShowGraphic( 0 ); break;
            }
         }
      }
   };

public:
   Widget& addWidget( const Widget& w )
   {
      mWidgetsChanged++;
      mWidgets.push_back( w );
      mWidgets[mWidgets.size()-1].mOnButton = mButtonPressFunc;
      mWidgets[mWidgets.size()-1].mOnButtonUserData = mUserData;
      return mWidgets[mWidgets.size() - 1];
   }
   Widget* findWidget( ButtonsAndLights bnl, int id = -1 )
   {
      for (float x = 0; x < mWidgets.size(); ++x)
      {
         if (mWidgets[x].bnl == bnl && mWidgets[x].id == id)
            return &mWidgets[x];
      }
      return NULL;
   }

   void setStepSelection( ButtonsAndLights bnl, int id = -1 )
   {
      Widget* w;
      w = findWidget( selected, selectedID );
      if (w)
      {
         printf( "unselecting...\n" );
         w->selected = false;
      }

      selected = bnl;
      selectedID = id;

      w = findWidget( bnl, id );
      if (w)
      {
         printf( "selecting...\n" );
         w->selected = true;
      }
   }

   ButtonsAndLights getStepSelection() const
   {
      return selected;
   }
   int getStepSelectionID() const
   {
      return selectedID;
   }

   // bnl must exist, or NULL dereference.
   void setState( ButtonsAndLights bnl, Widget::State state, int id = -1 )
   {
      Widget* w = findWidget( bnl, id );
      if (w)
      {
         w->setState( state );
      }
      else
      {
         const char* txt = getButtonText( bnl );
         printf( "Gr::setState( bnl:%s(%lu), id:%d ):  Gr::findWidget returned null\n", txt ? txt : "undefined", (size_t)bnl, id );
         exit( -1 );
      }
   }

   Actor a;

   Gfx::Scene scene;
   Ctx ctx;
   Level mLevel;
   float aspect;
   size_t mWidgetsChanged;

   void draw()
   {
      static WallClock c;
      mBlinkTimer += c.getDelta();
      if (0.20f < mBlinkTimer)
      {
         mBlinkTimer = 0.0f;
         mBlinkToggle = !mBlinkToggle;
      }

//#define TESTME
#if defined( TESTME )
      {
      // set up the render target
      //glDrawBuffer( GL_BACK );
      glClearColor( 0.9f, 0.3f, 0.3f, 1.0f );
      glClearDepthf( 1.0f );
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

      // initial render state
      //glEnable( GL_DEPTH_TEST );
      //glEnable( GL_BLEND );
      //glCullFace( GL_BACK );
      //glEnable( GL_CULL_FACE );

      vmMatrix4 world2cam;
      vmMatrix4 uiLocalToProj;
      vmMatrix4 cam2proj;

      world2cam = vmMatrix4::translation( vmVec3( 0,0,-25.0f ) ) *
                  vmMatrix4::rotation( syn::Math::PI / 180.0f,
                                          vmVec3( 0,1,0 ) );

      // 0,0 at top left of the screen, just like photoshop
      uiLeft   = 0;
      uiTop    = 0;
      uiRight  = width;
      uiBottom = width;
      uiLocalToProj = vmMatrix4::orthographic( uiLeft, uiRight,
                                             uiBottom, uiTop,
                                             -100.0f, 100.0f );

      scene.SetCameraToProj( uiLocalToProj );//cam2proj );
      scene.SetWorldToCamera( world2cam );
      scene.SetLocalToWorld( vmMatrix4::identity() );
      //mGameProjToWorld = scene.proj2world; // for mouse interaction screenspace to ui space

      glClearDepthf( 1.0f );
      glClear( /*GL_COLOR_BUFFER_BIT | */GL_DEPTH_BUFFER_BIT );
      float topcolor[] = {0.0f,0.0f,0.0f,1.0f};
      float bottomcolor[] = {1.0f,0.0f,0.0f,1.0f};
      drawColorRect( topcolor, bottomcolor, 0+20, 0+20, height-20, width-20 );
      glFlush();
      return;
      }
#endif
      // some changing value from 0 to 1
         static float changing = 0.0f;
         changing += 0.0001f;
         if (changing > 1.0f) changing = 0.0f;

         // set up the render target
         //glDrawBuffer( GL_BACK );
         glClearColor( 0.3f, 0.3f, 0.3f, 1.0f );
         glClearDepthf( 1.0f );
         glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
         // initial render state
         //glEnable( GL_DEPTH_TEST );
         //glEnable( GL_BLEND );
         //glCullFace( GL_BACK );
         //glEnable( GL_CULL_FACE );

         vmMatrix4 cam2proj;
         //if (!editing)
         {
            //aspect = 1.0f;
            cam2proj = vmMatrix4::perspective( 50.0f * syn::Math::PI / 180.0f,
                                               aspect, 0.1f, 10000.0f );

         }
         /*
         else
         {
            editLeft   = -15.0f * aspect * zoom;
            editRight  =  15.0f * aspect * zoom;
            editBottom = -15.0f * zoom;
            editTop    =  15.0f * zoom;
            cam2proj = vmMatrix4::orthographic( editLeft, editRight,
                                                editBottom, editTop,
                                                -100.0f, 100.0f );
         }
         */

         vmMatrix4 world2cam;
         static float spinn = 0;
         //spinn += 0.1f;
         world2cam = vmMatrix4::translation( vmVec3( 0,0,-25.0f ) ) *
                     vmMatrix4::rotation( spinn * syn::Math::PI / 180.0f,
                                           vmVec3( 0,1,0 ) );

         // 0,0 at top left of the screen, just like photoshop
         vmMatrix4 uiLocalToProj;
         uiLeft   = 0.0f;
         uiTop    = 0.0f;
         uiRight  = 50.0f;//width;
         uiBottom = aspect * 50.0f;//height;
         uiLocalToProj = vmMatrix4::orthographic( uiLeft, uiRight,
                                                uiBottom, uiTop,
                                                -100.0f, 100.0f );

         scene.SetCameraToProj( uiLocalToProj );//cam2proj );
         scene.SetWorldToCamera( world2cam );
         scene.SetLocalToWorld( vmMatrix4::identity() );
         gameProjToWorld = scene.proj2world; // for interaction

         mLevel.Draw( ctx );

         /*
         Gfx::SetShaderProgram( 0 );
         scene.SetCameraToProj( uiLocalToProj );
         scene.SetWorldToCamera( vmMatrix4::identity() );
         scene.SetLocalToWorld( vmMatrix4::identity() );
         scene.Draw( -1 );
         */
         glFlush();

      for (float x = 0; x < mWidgets.size(); ++x)
      {
         mWidgets[x].draw( mBlinkToggle, mTempoBlinkToggle, mHalfTempoBlinkToggle );
      }

      if (mUpdateLightsFunc)
         mUpdateLightsFunc( mUserData );
   }

   typedef void (*InitFunc)();
   typedef void (*SuspendFunc)();
   typedef void (*ResumeFunc)();
   typedef int (*UpdateLightsFunc)( void *userData );
   UpdateLightsFunc mUpdateLightsFunc;
   typedef void (*ButtonPressFunc)( ButtonsAndLights bnl, void *userData, int id );
   ButtonPressFunc mButtonPressFunc;
   SuspendFunc mSuspendFunc;
   ResumeFunc mResumeFunc;
   void* mUserData;
   std::vector<Widget> mWidgets;
   bool mBlinkToggle;
   float mBlinkTimer;
   bool mTempoBlinkToggle, mHalfTempoBlinkToggle;
   float uiLeft, uiTop, uiRight, uiBottom;
   vmMatrix4 gameProjToWorld;
   bool safeToDraw;

   void Quit()
   {
      SDL_Event event;
      event.quit.type = SDL_QUIT;
      SDL_PushEvent( &event );
   }

static void OnContextInit()
{
   gr.graphicsInit();
}

static void OnRedisplay()
{
   // call once to init your display lists, tex objects, shader programs, etc...
   if (gr.once)
   {
#ifndef EXCLUDE_GLEW
      GLenum err = glewInit();
#endif
      //OnContextInit();
      gr.once = false;
   }

   SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
   SDL_RenderClear(renderer);
   glClearColor( 0.1f, 0.2f, 0.3f, 1.0f );
   glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
   //glEnable( GL_DEPTH_TEST );
   //glEnable( GL_BLEND );
   //glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

   gr.draw();

   // ////////////////////////////////////////

   // swaps the front and back frame buffers.
   // hint: you've been drawing on the back, offscreen, buffer.
   // This command then brings that framebuffer onscreen.
   SDL_RenderPresent(renderer);
}



//////////////////////////////////////////////////
// This is called repeatedly, as fast as possible
//////////////////////////////////////////////////
static void OnIdle()
{
   bbpSDLpostredisplay();
}

/////////////////////////////////////////////
// This is called on a Resize of the glut window
/////////////////////////////////////////////
static void OnReshape( int width, int height )
{
   // save these params in case your app needs them
   gr.width = width;
   gr.height = height;
   gr.aspect = float( height ) / float( width );
   printf( "w:%d h:%d aspect:%f\n", width, height, gr.aspect );

   // set your viewport to the extents of the window
   glViewport( 0, 0, gr.width, gr.height );

   // let the app run idle, while resizing,
   // glut does not do this for us automatically, so call OnIdle explicitly.
   OnIdle();
   OnRedisplay();
}

////////////////////////////////
// This is called on a Down Keypress
////////////////////////////////
static void OnKeyboardDown( unsigned char k,
                            unsigned int keycode,
                            unsigned int scancode,
                            unsigned int mod, bool pressed,
                            int x, int y )
{
   switch (keycode)
   {
   case SDLK_q:
   case SDLK_ESCAPE:
      gr.release();
      gIsRunning = false;
      SDL_Quit();
      break;

   case SDLK_SPACE:
     if (gr.mButtonPressFunc && pressed == SDL_PRESSED)
        gr.mButtonPressFunc( Gr::Bpp, gr.mUserData, -1 );
     break;

   default:
      // do nothing if no key is pressed
      break;
   }
   bbpSDLpostredisplay();

}
////////////////////////////////
// This is called on a Up Keypress
////////////////////////////////
static void OnKeyboardUp( unsigned char k, int x, int y )
{
   switch (k)
   {
   case 'a':
      break;

   case 'z':
      break;

   default:
      // do nothing if no key is pressed
      break;
   }
}

////////////////////////////////
// This is called on a Down Keypress
// of a "special" key such as the grey arrows.
////////////////////////////////
static void OnSpecialKeyboardDown(int k, int x, int y)
{
   switch (k)
   {
   default:
      // do nothing if no special key pressed
      break;
   }
   bbpSDLpostredisplay();
}

////////////////////////////////
// This is called on a Up Keypress
////////////////////////////////
static void OnSpecialKeyboardUp( int k, int x, int y )
{
   switch (k)
   {
   default:
      // do nothing if no special key pressed
      break;
   }
}



////////////////////////////////
// This is called when mouse clicks. when norm x&y are 0..1
////////////////////////////////
static void OnMouseClick( SDL_FingerID id, int button, int state,
                           float x, float y, bool norm = false )
{
   vmVec3 world_mouse(0,0,0);
   if (!norm)
   {
      world_mouse = syn::window2world(
            x, y, gr.width, gr.height,
            gr.gameProjToWorld );
      y = gr.height-y;
   }
   else
   {
      world_mouse = syn::window2world(
            x*2.0f-1.0f, y*2.0f-1.0f,
            gr.gameProjToWorld );
      y = 1.0f-y;
   }

   //print( gr.gameProjToWorld );
   printf( "click: %f %f | world %f %f %f\n", x, y, world_mouse[0], world_mouse[1], world_mouse[2] );
   bool m = (button == SDL_BUTTON_LEFT && state == SDL_PRESSED);
   gr.touches[id].setPos( world_mouse[0], world_mouse[1], m );

   size_t widgetschanged = gr.mWidgetsChanged;
   bool hit = false;
   for (long int wIt = gr.mWidgets.size()-1; 0 <= wIt &&
         widgetschanged == gr.mWidgetsChanged; --wIt)
   {
      //printf( "wIT  = == %ld/%lu  bnl:%d|%s\n" , wIt, gr.mWidgets.size(), gr.mWidgets[wIt].bnl, gr.getButtonText( gr.mWidgets[wIt].bnl ) );
      bool h = gr.mWidgets[wIt].mouseclick( id, world_mouse[0], world_mouse[1], m, gr.touches, hit );
      if (h) hit = true;
   }

   if (m == false)
   {
      gr.touches.erase( id );
   }

   printf( "touches = %ld\n", gr.touches.size() );

   bbpSDLpostredisplay();
}


////////////////////////////////
// This is called when mouse changes position
// x and y are the screen position
// in your 2D window's coordinate frame
////////////////////////////////
static void OnMousePos( SDL_FingerID id, float x, float y, bool norm = false )
{
   vmVec3 world_mouse(0,0,0);
   if (!norm)
   {
      world_mouse = syn::window2world(
                                      x, y, gr.width, gr.height,
                                      gr.gameProjToWorld );
      y = gr.height-y;
   }
   else
   {
      world_mouse = syn::window2world(
                                      x*2.0f-1.0f, y*2.0f-1.0f,
                                      gr.gameProjToWorld );
      y = 1.0f-y;
   }

   //printf( "move: %d %d | world %f %f\n", x, y, world_mouse[0], world_mouse[1] );
   //print( gr.gameProjToWorld );
   gr.touches[id].setPos( world_mouse[0], world_mouse[1], gr.touches[id].m );

   size_t widgetschanged = gr.mWidgetsChanged;
   bool hit = false;
   for (long int wIt = gr.mWidgets.size()-1; 0 <= wIt &&
                                           widgetschanged == gr.mWidgetsChanged; --wIt)
   {
      bool h = gr.mWidgets[wIt].mousemove( id, world_mouse[0], world_mouse[1],
                                        gr.mWidgets[wIt].state == 2,
                                        gr.touches, hit );
      if (h) hit = true;
   }
   bbpSDLpostredisplay();
}

// Initialize the application
// initialize the state of your app here if needed...
static void OnApplicationInit()
{
   // Don't put open GL code here, this func may be called at anytime
   // even before the API is initialized
   // (like before a graphics context is obtained)
   gr.init();
}

void doSimpleLoop()
{
   Uint32 startFrame;
   Uint32 endFrame;
   int delay;
   int done;

   /* enter main loop */
   done = 0;
   SDL_Event event;
   while (!done) {
      startFrame = SDL_GetTicks();
      while (SDL_PollEvent(&event)) {
         switch (event.type) {
            case SDL_MOUSEBUTTONDOWN:
               //handleMouseButtonDown(&event);
               break;
            case SDL_MOUSEBUTTONUP:
               //handleMouseButtonUp(&event);
               break;
            case SDL_QUIT:
               done = 1;
               break;
         }
      }
      //render(renderer);               /* draw buttons */
      endFrame = SDL_GetTicks();

      /* figure out how much time we have left, and then sleep */
      delay = MILLESECONDS_PER_FRAME - (endFrame - startFrame);
      if (delay < 0) {
         delay = 0;
      } else if (delay > MILLESECONDS_PER_FRAME) {
         delay = MILLESECONDS_PER_FRAME;
      }
      SDL_Delay(delay);
   }
}

static int theSDLEventFilter(void* userdata, SDL_Event* event)
{
   Gr* ths = (Gr*)userdata;
   switch (event->type)
   {
//#if TARGET_OS_IPHONE
      case SDL_APP_TERMINATING:
         break;
      case SDL_APP_LOWMEMORY:
         break;
      case SDL_APP_DIDENTERBACKGROUND:
         break;
      case SDL_APP_DIDENTERFOREGROUND:
         break;
      case SDL_APP_WILLENTERBACKGROUND:
         SDL_Log("App will enter background (iOS)");
         ths->safeToDraw = false;
         ths->mSuspendFunc();
         break;
      case SDL_APP_WILLENTERFOREGROUND:
         SDL_Log("App will enter foreground (iOS)");
         ths->safeToDraw = true;
         ths->mResumeFunc();
         break;
//#endif
   }
   return 1;
}


int run( int argc, char** argv, InitFunc initFunc, UpdateLightsFunc updateFunc, ButtonPressFunc buttonsFunc,
         SuspendFunc suspendFunc, ResumeFunc resumeFunc, void* userData, bool windowed )
{
   gr.mUpdateLightsFunc = updateFunc;
   gr.mButtonPressFunc = buttonsFunc;
   gr.mUserData = userData;
   gr.mSuspendFunc = suspendFunc;
   gr.mResumeFunc = resumeFunc;

    gr.argc = 2;
    gr.argv.resize( 2 );
    gr.argv[0] = "bbp";
    gr.argv[1] = ".";

    SDL_Window *window;
    Uint32 startFrame;
    Uint32 endFrame;
    int delay;
    int done;

    /* initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Could not initialize SDL");
    }
    SDL_SetEventFilter( theSDLEventFilter, &gr );
    atexit(SDL_Quit);
    window = SDL_CreateWindow(NULL,
#if TARGET_OS_IPHONE
          0, 0, 1024, 768,
#else
          50, 0, 1024, 768,
#endif
                                SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS |
                                (windowed ? 0 : SDL_WINDOW_FULLSCREEN) );

    renderer = SDL_CreateRenderer(window, 0, 0);

    // audio init
    initFunc();
    //doSimpleLoop(); // good for debugging audio, removes all the graphics stuff from the mainloop...

    // gfx init
    OnContextInit();
    OnApplicationInit();


    printf( "\n" );
    printf( "%s - by kevin meinert - kevin@subatomicglue.com\n", gio::justFilename( gr.argv[0].c_str() ).c_str() );
    printf( "       usage:  %s <path>\n", gio::justFilename( gr.argv[0].c_str() ).c_str() );
    printf( "\n" );

    SDL_SetWindowSize(window, 1024, 768);


    /* main loop */
    gIsRunning = true;
    gr.safeToDraw = true;
    while (gIsRunning) {
        startFrame = SDL_GetTicks();
        SDL_Event event;
        while (gIsRunning && SDL_PollEvent(&event)) {
            switch ((long int)event.type)
            {
               case SDL_QUIT: gIsRunning = false; break;
#if TARGET_OS_IPHONE
               case SDL_FINGERMOTION:
                  SDL_Log("Finger Motion[%ld] %f | %f", (long int)event.tfinger.fingerId, (float)event.tfinger.x, (float)event.tfinger.y );
                  OnMousePos( event.tfinger.fingerId, event.tfinger.x,
                              event.tfinger.y, true );
                  break;
               case SDL_FINGERUP:
                  SDL_Log("Finger Up[%ld] %f | %f", (long int)event.tfinger.fingerId, (float)event.tfinger.x, (float)event.tfinger.y );
                  OnMouseClick( event.tfinger.fingerId, SDL_BUTTON_LEFT,
                              SDL_RELEASED, event.tfinger.x, event.tfinger.y,
                              true );
                  break;
               case SDL_FINGERDOWN:
                  SDL_Log("Finger Down[%ld] %f | %f", (long int)event.tfinger.fingerId, (float)event.tfinger.x, (float)event.tfinger.y );
                  OnMouseClick( event.tfinger.fingerId, SDL_BUTTON_LEFT,
                              SDL_PRESSED, event.tfinger.x, event.tfinger.y,
                              true );
                  break;
#endif
#if !TARGET_OS_IPHONE
               case SDL_MOUSEBUTTONUP:
               case SDL_MOUSEBUTTONDOWN:
                  SDL_Log("Mouse which:%d btn:%d state:%d [%d, %d]",
                                             event.button.which,
                                             (int)event.button.button,
                                             (int)event.button.state,
                                             (int)event.button.x,
                                             (int)event.button.y);
                  OnMouseClick( 0, event.button.button, event.button.state,
                        event.button.x,
                        event.button.y );
                  break;
               case SDL_MOUSEMOTION:
                  SDL_Log("Mouse which:%d state:%d [%d, %d]",
                                             event.motion.which,
                                             event.motion.state,
                                             (int)event.motion.x,
                                             (int)event.motion.y);
                  OnMousePos( 0, event.motion.x, event.motion.y );
                  break;
#endif
               case SDL_KEYUP:
               case SDL_KEYDOWN:
                  SDL_Log("Key state:%d rpt:%d scancode:%s|'%c'|%d modifier:%d",
                           (int)(event.key.state == SDL_PRESSED),
                           (int)event.key.repeat,
                           SDL_GetScancodeName( event.key.keysym.scancode ),
                           (char)event.key.keysym.scancode,
                           (char)event.key.keysym.scancode,
                           (int)event.key.keysym.mod );
                  OnKeyboardDown(
                     SDL_GetScancodeName( event.key.keysym.scancode )[0],
                     event.key.keysym.sym,
                     event.key.keysym.scancode,
                     event.key.keysym.mod,
                     event.key.state,
                     -1, -1 );
                  break;


               case SDL_WINDOWEVENT:
               {
                  switch ((long int)event.window.event) {
                  case SDL_WINDOWEVENT_SHOWN:
                        SDL_Log("Window %d shown", event.window.windowID);
                        break;
                  case SDL_WINDOWEVENT_EXPOSED:
                        SDL_Log("Window %d exposed", event.window.windowID);
                        break;
                  case SDL_WINDOWEVENT_HIDDEN:
                        SDL_Log("Window %d hidden", event.window.windowID);
                        break;
                  case SDL_WINDOWEVENT_MOVED:
                        SDL_Log("Window %d moved to %d,%d",
                              event.window.windowID, event.window.data1,
                              event.window.data2);
                        break;
                  case SDL_WINDOWEVENT_RESIZED:
                        SDL_Log("Window %d resized to %dx%d",
                              event.window.windowID, event.window.data1,
                              event.window.data2);
                        break;
                  case SDL_WINDOWEVENT_SIZE_CHANGED:
                        SDL_Log("Window %d size changed to %dx%d",
                              event.window.windowID, event.window.data1,
                              event.window.data2);
                        OnReshape( event.window.data1, event.window.data2 );
                        break;
                  case SDL_WINDOWEVENT_MINIMIZED:
                        SDL_Log("Window %d minimized", event.window.windowID);
                        break;
                  case SDL_WINDOWEVENT_MAXIMIZED:
                        SDL_Log("Window %d maximized", event.window.windowID);
                        break;
                  case SDL_WINDOWEVENT_RESTORED:
                        SDL_Log("Window %d restored", event.window.windowID);
                        break;
                  case SDL_WINDOWEVENT_ENTER:
                        SDL_Log("Mouse entered window %d",
                              event.window.windowID);
                        break;
                  case SDL_WINDOWEVENT_LEAVE:
                        SDL_Log("Mouse left window %d", event.window.windowID);
                        break;
                  case SDL_WINDOWEVENT_FOCUS_GAINED:
                        SDL_Log("Window %d gained keyboard focus",
                              event.window.windowID);
                        break;
                  case SDL_WINDOWEVENT_FOCUS_LOST:
                        SDL_Log("Window %d lost keyboard focus",
                              event.window.windowID);
                        break;
                  case SDL_WINDOWEVENT_CLOSE:
                        SDL_Log("Window %d closed", event.window.windowID);
                        break;
                  default:
                        SDL_Log("Window %d got unknown event %d",
                              event.window.windowID, event.window.event);
                        break;
                  }
               }
               break;
            }
        }

      if (gIsRunning && gr.safeToDraw)
      {
         OnRedisplay();
         OnIdle();
      }
      endFrame = SDL_GetTicks();

      /* figure out how much time we have left, and then sleep */
      delay = MILLESECONDS_PER_FRAME - (endFrame - startFrame);
      if (delay < 0) {
            delay = 0;
      } else if (delay > MILLESECONDS_PER_FRAME) {
            delay = MILLESECONDS_PER_FRAME;
      }
      SDL_Delay(delay);
    }

#if _DEBUG_NEW_REDEFINE_NEW
    new_autocheck_flag = true;
    new_verbose_flag = true;
    new_progname = argv[0];
    check_leaks();
    check_mem_corruption();
#endif

    /* shutdown SDL */
    SDL_Quit();

    return 0;
}


};

Gr gr;
