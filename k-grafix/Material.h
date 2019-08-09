#ifndef MATERIAL_H
#define MATERIAL_H

#include "common.h"
#include "AlignAllocator.h"
#include "EASTL/map.h"
#include "Gfx.h"

class Material
{
   public:
      Material()
      {
         mBlendBufferId = 0;
         mOpts[0] = OPT_END;
      }
      enum Option
      {
#ifndef EXCLUDE_GLEW // not in opengl ES 2
         OPT_LOGIC,
         OPT_ALPHATEST,
#endif
         OPT_DEPTH,
         OPT_EQBLEND_RGBA,
         OPT_EQBLEND_RGB_A,
         OPT_SRCDESTBLEND_RGBA,
         OPT_SRCDESTBLEND_RGB_A,
         OPT_END
      };

#ifndef EXCLUDE_GLEW // not in opengl ES 2
      enum LogicOp
      {
         LOGIC_AND = GL_AND,
         LOGIC_AND_REVERSE = GL_AND_REVERSE,
         LOGIC_COPY = GL_COPY,
         LOGIC_AND_INVERTED = GL_AND_INVERTED,
         LOGIC_NOOP = GL_NOOP,
         LOGIC_OR = GL_OR,
         LOGIC_NOR = GL_NOR,
         LOGIC_EQUIV = GL_EQUIV,
         LOGIC_INVERT = GL_INVERT,
         LOGIC_OR_REVERSE = GL_OR_REVERSE,
         LOGIC_COPY_INVERTED = GL_COPY_INVERTED,
         LOGIC_OR_INVERTED = GL_OR_INVERTED,
         LOGIC_NAND = GL_NAND,
         LOGIC_SET = GL_SET
      };
#endif
      enum AlphaTest
      {
         ALPHA_NEVER = GL_NEVER,
         ALPHA_ALWAYS = GL_ALWAYS,
         ALPHA_LESS = GL_LESS,
         ALPHA_LEQUAL = GL_LEQUAL,
         ALPHA_EQUAL = GL_EQUAL,
         ALPHA_GEQUAL = GL_GEQUAL,
         ALPHA_GREATER = GL_GREATER,
         ALPHA_NOTEQUAL = GL_NOTEQUAL
      };
      enum EqBlend
      {
         EQBLEND_ADD = GL_FUNC_ADD,
         EQBLEND_SUB = GL_FUNC_SUBTRACT,
         EQBLEND_INVSUB = GL_FUNC_REVERSE_SUBTRACT,
#ifndef EXCLUDE_GLEW // not in opengl ES 2
         EQBLEND_MIN = GL_MIN,
         EQBLEND_MAX = GL_MAX
#endif
      };
      enum SrcBlend
      {
         SRCBLEND_ZERO = GL_ZERO,
         SRCBLEND_ONE = GL_ONE,
         SRCBLEND_ONE_MINUS_SRC_COLOR = GL_ONE_MINUS_SRC_COLOR,
         SRCBLEND_SRC_COLOR = GL_SRC_COLOR,
         SRCBLEND_ONE_MINUS_DST_COLOR = GL_ONE_MINUS_DST_COLOR,
         SRCBLEND_DST_COLOR = GL_DST_COLOR,
         SRCBLEND_ONE_MINUS_SRC_ALPHA = GL_ONE_MINUS_SRC_ALPHA,
         SRCBLEND_SRC_ALPHA = GL_SRC_ALPHA,
         SRCBLEND_ONE_MINUS_DST_ALPHA = GL_ONE_MINUS_DST_ALPHA,
         SRCBLEND_DST_ALPHA = GL_DST_ALPHA,
         SRCBLEND_ONE_MINUS_CONSTANT_COLOR = GL_ONE_MINUS_CONSTANT_COLOR,
         SRCBLEND_CONSTANT_COLOR = GL_CONSTANT_COLOR,
         SRCBLEND_ONE_MINUS_CONSTANT_ALPHA = GL_ONE_MINUS_CONSTANT_ALPHA,
         SRCBLEND_CONSTANT_ALPHA = GL_CONSTANT_ALPHA
      };
      enum DestBlend
      {
         DSTBLEND_ZERO = GL_ZERO,
         DSTBLEND_ONE = GL_ONE,
         DSTBLEND_ONE_MINUS_SRC_COLOR = GL_ONE_MINUS_SRC_COLOR,
         DSTBLEND_SRC_COLOR = GL_SRC_COLOR,
         DSTBLEND_ONE_MINUS_DST_COLOR = GL_ONE_MINUS_DST_COLOR,
         DSTBLEND_DST_COLOR = GL_DST_COLOR,
         DSTBLEND_ONE_MINUS_SRC_ALPHA = GL_ONE_MINUS_SRC_ALPHA,
         DSTBLEND_SRC_ALPHA = GL_SRC_ALPHA,
         DSTBLEND_ONE_MINUS_DST_ALPHA = GL_ONE_MINUS_DST_ALPHA,
         DSTBLEND_DST_ALPHA = GL_DST_ALPHA,
         DSTBLEND_ONE_MINUS_CONSTANT_COLOR = GL_ONE_MINUS_CONSTANT_COLOR,
         DSTBLEND_CONSTANT_COLOR = GL_CONSTANT_COLOR,
         DSTBLEND_ONE_MINUS_CONSTANT_ALPHA = GL_ONE_MINUS_CONSTANT_ALPHA,
         DSTBLEND_CONSTANT_ALPHA = GL_CONSTANT_ALPHA,
         DSTBLEND_SRC_ALPHA_SATURATE = GL_SRC_ALPHA_SATURATE
      };

      void draw()
      {
         Option* opt = mOpts;
         register Option op = *mOpts;
         while (op != OPT_END)
         {
            switch (op)
            {
#ifndef EXCLUDE_GLEW // not in opengl ES 2
            case OPT_LOGIC:
               glEnable( GL_COLOR_LOGIC_OP );
               glLogicOp( mLogicOp );
            break;
#endif
            case OPT_DEPTH:
               glEnable( GL_DEPTH_TEST );
            break;
#ifndef EXCLUDE_GLEW // not in opengl ES 2
            case OPT_ALPHATEST:
               glEnable(GL_ALPHA_TEST);
               glAlphaFunc(mAlphaTest, mAlphaRef);
            break;
#endif
            case OPT_EQBLEND_RGBA:
               glEnable(GL_BLEND);
               glBlendEquation(mEqBlend);
            break;
            case OPT_EQBLEND_RGB_A:
               glEnable(GL_BLEND);
               glBlendEquationSeparate(mEqBlend, mEqBlendAlpha);
            break;
            case OPT_SRCDESTBLEND_RGBA:
               glEnable(GL_BLEND);
               glBlendFunc(mSrcBlendOp, mDstBlendOp);
            break;
            case OPT_SRCDESTBLEND_RGB_A:
               //glEnablei(GL_BLEND, mBlendBufferId);
               glEnable(GL_BLEND);
               glBlendFuncSeparate(mSrcBlendOp, mDstBlendOp,
                              mSrcBlendAlphaOp, mDstBlendAlphaOp);
            break;
            case OPT_END: assert( false ); break;
            }
            op = *(++opt);
         }
      }

      void undraw()
      {
         Option* opt = mOpts;
         register Option op = *mOpts;
         while (op != OPT_END)
         {
            switch (op)
            {
#ifndef EXCLUDE_GLEW // not in opengl ES 2
            case OPT_LOGIC:
               glDisable( GL_COLOR_LOGIC_OP );
            break;
#endif
            case OPT_DEPTH:
               glDisable( GL_DEPTH_TEST );
            break;
#ifndef EXCLUDE_GLEW // not in opengl ES 2
            case OPT_ALPHATEST:
               glDisable(GL_ALPHA_TEST);
            break;
#endif
            case OPT_EQBLEND_RGBA:
            case OPT_EQBLEND_RGB_A:
            case OPT_SRCDESTBLEND_RGBA:
            case OPT_SRCDESTBLEND_RGB_A:
               // glDisablei(GL_BLEND, mBlendBufferId);
               glDisable(GL_BLEND);
               break;
            case OPT_END: assert( false ); break;
            }
            op = *(++opt);
         }
      }

#ifndef EXCLUDE_GLEW
      LogicOp mLogicOp;      // LOGIC_DISABLED to disable
#endif

      AlphaTest mAlphaTest;  // ALPHA_DISABLED to disable
      float mAlphaRef;

      // only one Eq or Src/Dst can be chosen.  Eq wins if both are set
      // Auto Blend Equation
      EqBlend mEqBlend;      // EQBLEND_DISABLED to disable (priority)
      EqBlend mEqBlendAlpha; // set this to separatly specify alpha

      // Specify Blend Equation using Src/Dest
      SrcBlend mSrcBlendOp;  // BLEND_DISABLED to disable
      DestBlend mDstBlendOp;
      SrcBlend mSrcBlendAlphaOp; // set this to separatly specify alpha
      DestBlend mDstBlendAlphaOp;
      int mBlendBufferId;        // -1  for all draw buffers.

      // use this to set up chain...
      Option mOpts[OPT_END+1];
};

class ShaderVars
{
   public:
      void draw( Gfx::ShaderProgram p )
      {
         for (MatMap::iterator it = mats.begin();
               it != mats.end();
               ++it)
         {
            vmMatrix4* m = it->second;
            GLint loc = glGetUniformLocation( p, it->first.c_str() );
            if (loc != -1)
            {
               glUniformMatrix4fv( loc, 1, GL_FALSE, (float*)m );
            }
         }

         for (VecMap::iterator it = vecs.begin();
               it != vecs.end();
               ++it)
         {
            vmVec4* v = it->second;
            GLint loc = glGetUniformLocation( p, it->first.c_str() );
            if (loc != -1)
            {
               glUniform4fv( loc, 1, (float*)v );
            }
         }

         for (ScalarMap::iterator it = scalars.begin();
               it != scalars.end();
               ++it)
         {
            float* s = it->second;
            GLint loc = glGetUniformLocation( p, it->first.c_str() );
            if (loc != -1)
            {
               //printf( "%s %lx %f %d %d\n", it->first.c_str(), s, *s, loc, p );
               glUniform1f( loc, *s );
            }
         }
      }

      typedef eastl::map<std::string, vmMatrix4*, eastl::less<std::string>, align_allocator<16> > MatMap;

      typedef eastl::map<std::string, vmVec4*, eastl::less<std::string>, align_allocator<16> > VecMap;

      typedef eastl::map<std::string, float*, eastl::less<std::string>, align_allocator<16> > ScalarMap;


      //EASTLAllocatorType
      MatMap      mats;    // map of name to data
      VecMap      vecs;    // map of name to data
      ScalarMap   scalars; // map of name to data
};

typedef ShaderVars MaterialShader ;

#endif

