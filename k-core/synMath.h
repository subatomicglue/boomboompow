
/****************** <SYN heading BEGIN do not edit this line> *****************
 *
 * subsynth - modular audio synthesizer
 * subsynth is (C) Copyright 2001-2002 by Kevin Meinert
 *
 * Original Author: Kevin Meinert
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * -----------------------------------------------------------------
 * File:          $RCSfile$
 * Date modified: $Date: 2012-03-11 18:26:54 -0500 (Sun, 11 Mar 2012) $
 * Version:       $Revision: 2058 $
 * -----------------------------------------------------------------
 *
 ****************** <SYN heading END do not edit this line> ******************/
#ifndef SYNMATH
#define SYNMATH
#include <assert.h>
#include <math.h>

#if defined( SYN_REFERENCEMATH ) && defined( SYN_SSEMATH )
   error, you can't have both defined.
#endif
#if (defined( __SSE__ ) || defined( SYN_SSEMATH )) && !defined( SYN_REFERENCEMATH )
// headers for fast math intrinsics
//#include <mmintrin.h>
#include <xmmintrin.h>
#endif

#include <stdint.h> // types. int32_t, etc...
typedef float float32_t;
typedef double float64_t;

#include <vectormath_aos.h>
#include <boolInVec.h>
#include <floatInVec.h>

typedef Vectormath::boolInVec       vmVecB;
typedef Vectormath::floatInVec      vmVecS;
typedef Vectormath::Aos::Vector3    vmVec3;
typedef Vectormath::Aos::Vector4    vmVec4;
typedef Vectormath::Aos::Quat       vmQuat;
typedef Vectormath::Aos::Matrix3    vmMatrix3;
typedef Vectormath::Aos::Matrix4    vmMatrix4;
typedef Vectormath::Aos::Transform3 vmTransform3;
typedef Vectormath::Aos::Point3     vmPoint3;

// bullet seems to have print() defined already
void print( const vmMatrix4& m )
{
   printf( "[%.02f %.02f %.02f %.02f]\n[%.02f %.02f %.02f %.02f]\n[%.02f %.02f %.02f %.02f]\n[%.02f %.02f %.02f %.02f]\n", m[0][0],m[0][1],m[0][2],m[0][3],
                                           m[1][0],m[1][1],m[1][2],m[1][3],
                                           m[2][0],m[2][1],m[2][2],m[2][3],
                                           m[3][0],m[3][1],m[3][2],m[3][3] );
}

#ifdef SYNFORCEINLINE
#undef SYNFORCEINLINE
#endif

#if defined( __GNUC__ )
//broken for template functions in 3.4 gcc :(  reenable!
//#   define SYNFORCEINLINE __attribute__((always_inline))
#   define SYNFORCEINLINE __attribute__((__always_inline__))
//#   define SYNFORCEINLINE inline
#   define SYN_ALIGN_16 __attribute__((aligned(16)))
#elif defined( WIN32 ) && !defined( __GNUC__ )
#   define SYNFORCEINLINE __forceinline
#   define SYN_ALIGN_16 __declspec(align(16))
#endif

#ifdef WIN32
#define SYNRESTRICT __restrict // put this after the *    i.e.    float* SYNRESTRICT myvar;
#endif

/// auto loop unrolling up to 8x, useful when the number of iterations is not known.
/// WARNING: when number of iterations is known, use 'for' loops!  let the compiler unroll!
/// It's ugly, but check out the assembly...
/// reference: http://www.lysator.liu.se/c/duffs-device.html
#define DUFFS_DEVICE_8X( size, code_to_every8, code_to_exec ) \
{ \
    register size_t __DUFFS_DEVICE_n = (size >> 3); \
    switch (size & 7) \
    { \
        case 0: while (__DUFFS_DEVICE_n-- > 0) \
                { \
                    code_to_every8; \
                    code_to_exec; \
        case 7:     code_to_exec; \
        case 6:     code_to_exec; \
        case 5:     code_to_exec; \
        case 4:     code_to_exec; \
        case 3:     code_to_exec; \
        case 2:     code_to_exec; \
        case 1:     code_to_exec; \
                } \
    } \
}

/// auto loop unrolling up to 16x, useful when the number of iterations is not known.
/// WARNING: when number of iterations is known, use 'for' loops!  let the compiler unroll!
/// It's ugly, but check out the assembly...
/// reference: http://www.lysator.liu.se/c/duffs-device.html
#define DUFFS_DEVICE_16X( size, code_to_exec_every16, code_to_exec ) \
{ \
    register size_t __DUFFS_DEVICE_n = (size >> 4); \
    switch (size & 15) \
    { \
        case 0: while (__DUFFS_DEVICE_n-- > 0) \
                { \
                    code_to_exec_every16; \
                    code_to_exec; \
        case 15:    code_to_exec; \
        case 14:    code_to_exec; \
        case 13:    code_to_exec; \
        case 12:    code_to_exec; \
        case 11:    code_to_exec; \
        case 10:    code_to_exec; \
        case 9:     code_to_exec; \
        case 8:     code_to_exec; \
        case 7:     code_to_exec; \
        case 6:     code_to_exec; \
        case 5:     code_to_exec; \
        case 4:     code_to_exec; \
        case 3:     code_to_exec; \
        case 2:     code_to_exec; \
        case 1:     code_to_exec; \
                } \
    } \
}


namespace syn
{
   namespace Math
   {
      /** operator += */
      struct ADDEQUAL { SYNFORCEINLINE static void oper( float& i, const float o ) { i += o; } };

      /** operator = */
      struct EQUAL { SYNFORCEINLINE static void oper( float& i, const float o ) { i = o; } };

      /** operator *= */
      struct MULTEQUAL { SYNFORCEINLINE static void oper( float& i, const float o ) { i *= o; } };

      const float PI = 3.14159265358979323846f; //3.14159265358979323846264338327950288419716939937510;
      const float PI_OVER_2 = 1.57079632679489661923f;
      const float PI_OVER_4 = 0.78539816339744830962f;

#if (defined( __SSE__ ) || defined( SYN_SSEMATH )) && !defined( SYN_REFERENCEMATH )
      /** min returns the minimum of 2 values */
      template <class T>
      SYNFORCEINLINE T Min( const register T x, const register T y )
      {
         return ( x < y ) ? x : y;                      // tested faster on intel p4 2.26ghz x86 family 15 model 2...
         //return (float&)_mm_min_ss(_mm_set_ss(x),_mm_set_ss(y));
      }
      /** max returns the maximum of 2 values */
      template <class T>
      SYNFORCEINLINE T Max( const register T x, const register T y )
      {
         return ( x > y ) ? x : y;                      // tested faster on intel p4 2.26ghz x86 family 15 model 2...
         //return (float&)_mm_max_ss(_mm_set_ss(x),_mm_set_ss(y));
      }
      /// reciprocal 1/x
      SYNFORCEINLINE float Reciprocal( register float value )
      {
          return 1.0f / value;                          // tested faster on intel p4 2.26ghz x86 family 15 model 2...
          //return (float&)_mm_rcp_ss(_mm_set_ss(value));
      }
      /// sqrt
      template <typename T>
      SYNFORCEINLINE T Sqrt( const register T fValue );
      SYNFORCEINLINE float Sqrt( register float x )
      {
          /*__asm
          {
              fld x
              fsqrt
          }
          */
          return ::sqrt( x ); // compiler intrinsic    // tested faster on intel p4 2.26ghz x86 family 15 model 2...
      }
      SYNFORCEINLINE double Sqrt( register double x )
      {
          return ::sqrt( x ); // compiler intrinsic
      }
      /// reciprocal sqrt
      SYNFORCEINLINE float SqrtR( register float x )
      {
          // major LHS here?...

          //return 1.0f / ::sqrt( x );
          SYN_ALIGN_16 float out[4];
          // tested faster on intel p4 2.26ghz x86 family 15 model 2...
          _mm_store_ps(out, _mm_rsqrt_ss( _mm_set_ss( x ) ) );
          return out[0];
      }
#else
      //----------------------------------------------------------------------------
      /** min returns the minimum of 2 values */
      template <class T>
      SYNFORCEINLINE T Min( const register T x, const register T y )
      {
         return ( x > y ) ? y : x;
      }
      /** max returns the maximum of 2 values */
      template <class T>
      SYNFORCEINLINE T Max( const register T x, const register T y )
      {
         return ( x > y ) ? x : y;
      }
      /// reciprocal 1/x
      SYNFORCEINLINE float Reciprocal( register float value )
      {
          return 1.0f / value;
      }
      /// sqrt
      template <typename T>
      SYNFORCEINLINE T Sqrt( const register T x )
      {
          return ::sqrt( x );
      }
      /// reciprocal sqrt
      SYNFORCEINLINE float SqrtR( register float x )
      {
          return 1.0f / ::sqrt( x );
      }
#endif
      /// This is a approximate yet fast inverse square-root. (from Box2D)
      SYNFORCEINLINE float fastSqrtR( register float x )
      {
            union
            {
               float x;
               int32_t i;
            } convert;

            convert.x = x;
            float xhalf = 0.5f * x;
            convert.i = 0x5f3759df - (convert.i >> 1);
            x = convert.x;
            x = x * (1.5f - xhalf * x * x);
            return x;
      }
      /** min returns the minimum of 3 values */
      template <class T>
      SYNFORCEINLINE T Min( const register T x, const register T y, const register T z )
      {
         return Math::Min( Math::Min( x, y ), z );
      }
      /** min returns the minimum of 4 values */
      template <class T>
      SYNFORCEINLINE T Min( const register T w, const register T x, const register T y, const register T z )
      {
         return Math::Min( Math::Min( w, x ), Math::Min( y, z ) );
      }
      /** max returns the maximum of 3 values */
      template <class T>
      SYNFORCEINLINE T Max( const register T x, const register T y, const register T z )
      {
         return Math::Max( Math::Max( x, y ), z );
      }
      /** max returns the maximum of 4 values */
      template <class T>
      SYNFORCEINLINE T Max( const register T w, const register T x, const register T y, const register T z )
      {
         return Math::Max( Math::Max( w, x ), Math::Max( y, z ) );
      }
      /** Linear Interpolation between number [a] and [b]
       * use double or float only...
       */
      /// clamp between two values.
      template <class T>
      SYNFORCEINLINE T clamp( const register T x, const register T low, const register T high )
      {
          // no measurable difference between these methods on intel p4 2.26ghz x86 family 15 model 2...
         //   if ( x < low ) return low;
         //   else if (high < x) return high;
         //   else return x;
         return Math::Min( high, Math::Max( x, low ) ); // leaving this one in since min/max are intrinsics on some platforms, and because conditionals are really expensive on some platforms...
      }
      template <class T, typename U>
      SYNFORCEINLINE T lerp( const register U lerp, const register T a, const register T b )
      {
          register T size = b - a;
          return ((U)a) + (((U)size) * lerp);
      }


      // remap value1 from a1..b1 space to a2..b2 space...
      SYNFORCEINLINE float remap( const register float value1,
                                  const register float a1,
                                  const register float b1,
                                  const register float a2,
                                  const register float b2 )
      {
          return ((value1 - a1) / (b1-a1)) * (b2 - a2) + a2;
      } 
      SYNFORCEINLINE double remap( const register double value1,
                                   const register double a1,
                                   const register double b1,
                                   const register double a2,
                                   const register double b2 )
      {
          return ((value1 - a1) / (b1-a1)) * (b2 - a2) + a2;
      }
      //----------------------------------------------------------------------------
      SYNFORCEINLINE float floor( const register float fValue )
      {
          return ::floorf( fValue );
      }
      SYNFORCEINLINE double floor( const register double fValue )
      {
          return ::floor( fValue );
      }
      //----------------------------------------------------------------------------
      /** cut off the digits after the decimal place */
 //     template <class T>
 //     SYNFORCEINLINE T trunc( T val )
 //     {
 //        return T( (val < ((T)0)) ? Math::ceil( val ) : Math::floor( val ) );
  //    }
      /** round to nearest integer */
      SYNFORCEINLINE float round( const register float p ) { return Math::floor( p + 0.5f ); }
      SYNFORCEINLINE double round( const register double p ) { return Math::floor( p + 0.5 ); }
      inline float unitRandom()
      {
         // return between 0 and 1
         return float(rand())/float(RAND_MAX);
         //float ret_val;
         //ret_val = drand48();
         //assert( (ret_val >= 0.0f) && (ret_val <= 1.0f) );
         //return ret_val;
      }

      /** return a random number between x1 and x2
       * RETURNS: random number between x1 and x2
       */
      inline float rangeRandom( float x1, float x2 )
      {
         float r = Math::unitRandom();
         float size = x2 - x1;
         return float( r * size + x1 );
      }

      //----------------------------------------------------------------------------
      // don't allow non-float types, because their ret val wont be correct
      // i.e. with int, the int retval will be rounded up or down.
      //      we'd need a float retval to do it right, but we can't specialize by ret
      template <typename T>
      inline T aCosSafe( const register T fValue );
      inline float aCosSafe( const register float fValue )
      {
          if ( -1.0f < fValue )
          {
              if ( fValue < 1.0f )
                  return float( ::acosf( fValue ) );
              else
                  return 0.0f;
          }
          else
          {
              return (float)Math::PI;
          }
      }
      inline double aCosSafe( const register double fValue )
      {
          if ( -1.0 < fValue )
          {
              if ( fValue < 1.0 )
                  return double( ::acos( fValue ) );
              else
                  return 0.0;
          }
          else
          {
              return (double)PI;
          }
      }
      //----------------------------------------------------------------------------
      template <typename T>
      inline T aSinSafe( const register T fValue );
      inline float aSinSafe( const register float fValue )
      {
          if ( -1.0f < fValue )
          {
              if ( fValue < 1.0f )
                  return float( ::asinf( fValue ) );
              else
                  return (float)-PI_OVER_2;
          }
          else
          {
              return (float)PI_OVER_2;
          }
      }
      inline double aSinSafe( const register double fValue )
      {
          if ( -1.0 < fValue )
          {
              if ( fValue < 1.0 )
                  return ::asin( fValue );
              else
                  return (double)-PI_OVER_2;
          }
          else
          {
              return (double)PI_OVER_2;
          }
      }
      template <typename T>
      SYNFORCEINLINE T aTan( const register T fValue );
      SYNFORCEINLINE float aTan( const register float fValue )
      {
          return ::atanf( fValue );
      }
      SYNFORCEINLINE double aTan( const register double fValue )
      {
          return ::atan( fValue );
      }
      //----------------------------------------------------------------------------
      template <typename T>
      SYNFORCEINLINE T aTan2( const register T fY, const register T fX );
      SYNFORCEINLINE float aTan2( const register float fY, const register float fX )
      {
          return ::atan2f( fY, fX );
      }
      SYNFORCEINLINE double aTan2( const register double fY, const register double fX )
      {
          return ::atan2( fY, fX );
      }
      //----------------------------------------------------------------------------
      template <typename T>
      SYNFORCEINLINE T cos( const register T fValue );
      SYNFORCEINLINE float cos( const register float fValue )
      {
          return ::cosf( fValue );
      }
      SYNFORCEINLINE double cos( const register double fValue )
      {
          return ::cos( fValue );
      }
      //----------------------------------------------------------------------------
      template <typename T>
      SYNFORCEINLINE T exp( const register T fValue );
      SYNFORCEINLINE float exp( const register float fValue )
      {
          return ::expf( fValue );
      }
      SYNFORCEINLINE double exp( const register double fValue )
      {
          return ::exp( fValue );
      }
      //----------------------------------------------------------------------------
      template <typename T>
      SYNFORCEINLINE T log( const register T fValue );
      SYNFORCEINLINE double log( const register double fValue )
      {
          return ::log( fValue );
      }
      SYNFORCEINLINE float log( const register float fValue )
      {
          return ::logf( fValue );
      }
      //----------------------------------------------------------------------------
      SYNFORCEINLINE double pow( const register double fBase, const register double fExponent)
      {
          return ::pow( fBase, fExponent );
      }
      SYNFORCEINLINE float pow( const register float fBase, const register float fExponent)
      {
          return ::powf( fBase, fExponent );
      }
      template <typename T>
      SYNFORCEINLINE T sin( const register T fValue );
      SYNFORCEINLINE double sin( const register double fValue )
      {
          return ::sin( fValue );
      }
      SYNFORCEINLINE float sin( const register float fValue )
      {
          return ::sinf( fValue );
      }
      template <typename T>
      SYNFORCEINLINE T tan( const register T fValue );
      SYNFORCEINLINE double tan( const register double fValue )
      {
          return ::tan( fValue );
      }
      SYNFORCEINLINE float tan( const register float fValue )
      {
          return ::tanf( fValue );
      }
      template <typename T>
      SYNFORCEINLINE T sqr( const register T fValue )
      {
          return fValue * fValue;
      }     
      SYNFORCEINLINE int abs( const register int iValue ) { return ::abs( iValue ); }
      SYNFORCEINLINE float abs( const register float iValue ) { return ::fabsf( iValue ); }
      SYNFORCEINLINE double abs( const register double iValue ) { return ::fabs( iValue ); }

      /** Is almost equal?
       * test for equality within some tolerance...
       * @PRE: tolerance must be >= 0
       */
      template <class T>
      SYNFORCEINLINE bool isEqual( const register T a, const register T b, const register T tolerance )
      {
         assert( tolerance >= (T)0 );
         return syn::Math::abs( a - b ) <= tolerance;
      }
      
/*
#ifdef WIN32
      // val is a 16-bit fixed-point value in 0x0 - 0xFFFF ([0 ; 1[)
      // Returns a 32-bit fixed-point value in 0x80000000 - 0xFFFFFFFF ([0.5 ; 1[)
      inline unsigned int fast_partial_exp2( int val )
      {
         unsigned int   result;

         __asm
         {
            mov eax, val
            shl eax, 15           ; eax = input [31/31 bits]
            or  eax, 080000000h   ; eax = input + 1  [32/31 bits]
            mul eax
            mov eax, edx          ; eax = (input + 1) ^ 2 [32/30 bits]
            mov edx, 2863311531   ; 2/3 [32/32 bits], rounded to +oo
            mul edx               ; eax = 2/3 (input + 1) ^ 2 [32/30 bits]
            add edx, 1431655766   ; + 4/3 [32/30 bits] + 1
            mov result, edx
         }
         

         return (result);
      }
#else
*/
double exp2Est( const double val );
float Log2Est( const float f );
//#endif


    /// input range [0..1], outputs range [0..1]
	  inline float smoothSplineNegOneToOne( const float val )
	  {
          float lerpval = val * 0.5f + 0.5f;
		  float a = lerp( lerpval, -1.0f, val );
		  float b = lerp( lerpval, val, 1.0f );
		  return lerp( lerpval, a, b ); // scale and bias lerpval from -1..1 to 0..1
	  }

    /// input range [0..1], outputs range [0..1]
	  	  inline float smoothSplineNegOneToOne_x2( const float val )
		  {
			  return smoothSplineNegOneToOne( smoothSplineNegOneToOne( val ) );
		  }


    /// smoothing routine from Cg/HLSL
    /// maps linear input to output that gradually fades in, goes linear, and gradually fades out (think like a spline curve, or like sledding from the top of a sin curve)
    /// input range [0..1], outputs range [0..1]
    SYNFORCEINLINE float smoothSpline( float x )
    {
        //const float mn = 0.0f; // min
        //const float mx = 1.0f; // max
        //float a = (x - mn) / (mx - mn);
        //return -2.0f*a*a*a + 3.0f*a*a;
      
        return -2.0f*x*x*x + 3.0f*x*x;

		// equivelent:
		//  float a = lerp( val, 0.0f, val );
		//  float b = lerp( val, val, 1.0f );
		//  return lerp( val, a, b );
	}

	SYNFORCEINLINE float smoothSpline_x2( float x )
	{
		return smoothSpline( smoothSpline( x ) );
	}

      /// maps linear 0..1 to log 0..1
      /// maps linear input to log output (fade in fast, smooth fade out)
      SYNFORCEINLINE float smoothLog( const float val )
      {
         static const float b = 0.005179f;
         static const float a = 1.0f - b;
         static const float c = 0.19f;
         static const float d = 1.0f;
         return d + c * logf( a * val + b );
      }

      /// maps linear 0..1 to log 0..1
      /// maps linear input to log output (fade in fast, smooth fade out)
      /// input is [0..1], output is also [0..1] but on a logarithmic curve as defined by Log2Est()
      SYNFORCEINLINE float smoothFastLog2( const float val )
      {
         static const float b = 0.005179f;
         static const float a = 1.0f - b;
         static const float c = 0.19f;
         static const float d = 1.0f;
         return d + c * Log2Est( a * val + b );
      }


    


      /// exponential smoothing routine (using expf())
      /// maps linear input to exponential output (fade in slow, then ramp up quick at the end)
      /// input is [0..1], output is also [0..1] but on an exponential curve as defined by expf()
      SYNFORCEINLINE float smoothExp( const float val )
      {
         return expf( 11.0f * val - 10.0f ) * 0.367918f;
      }

      

	  /// exponential smoothing routine (using expf())  (Transposed)
      /// maps linear input to exponential output (fade in fast, then ramp up slow at the end)
      /// input is [0..1], output is also [0..1] but on an exponential curve as defined by exp2Fast()
      SYNFORCEINLINE float smoothExpT( const float val )
      {
         return 1.0f - expf( 11.0f * (1.0f - val) - 10.0f ) * 0.367918f;
      }


    /// exponential smoothing routine
    /// maps linear input to exponential output (gradual fade in, then ramp up fast (slower fade in compared to smoothExp3))
    /// input range [0..1], outputs range [0..1]
    SYNFORCEINLINE float smoothExp_x2( float x )
    {
        return (x*x);
    }

    /// exponential smoothing routine
    /// maps linear input to exponential output (gradual fade in, then ramp up fast (faster fade in compared to smoothExp2))
    /// input range [0..1], outputs range [0..1]
    SYNFORCEINLINE float smoothExp_x3( float x )
    {
        return (x*x*x);
    }

    /// exponential smoothing routine
    /// maps linear input to exponential output (gradual fade in, then ramp up fast (faster fade in compared to smoothExp3))
    /// input range [0..1], outputs range [0..1]
    SYNFORCEINLINE float smoothExp_x6( float x )
    {
        float x6 = x*x;
        x6 *= x6*x6;
        return x6;
    }

	/// exponential smoothing routine (to any power)
    /// maps linear input to exponential output (gradual fade in, then ramp up fast)
    /// input range [0..1], outputs range [0..1]
    SYNFORCEINLINE float smoothExpPow( float x, float power )
    {
        return powf( x, power );
    }

	/// exponential smoothing routine (to any power)   (Transposed)
    /// maps linear input to exponential output (abrupt fade in, then ramp up gradually)
    /// input range [0..1], outputs range [0..1]
    SYNFORCEINLINE float smoothExpPowT( float x, float power )
    {
        return 1.0f - powf( 1.0f - x, power );
    }


	/// exponential smoothing routine (transposed)
    /// maps linear input to exponential output (abrupt fade in, smooth fade out (slower fade in compared to smoothExp3))
    /// input range [0..1], outputs range [0..1]
    SYNFORCEINLINE float smoothExpT_x2( float x )
    {
        x = 1.0f - x;
        return 1.0f - (x*x);
    }

    /// exponential smoothing routine (transposed)
    /// maps linear input to exponential output (abrupt fade in, then ramp up slow (faster fade in compared to smoothExp2))
    /// input range [0..1], outputs range [0..1]
    SYNFORCEINLINE float smoothExpT_x3( float x )
    {
        x = 1.0f - x;
        return 1.0f - (x*x*x);
    }

    /// exponential smoothing routine (transposed)
    /// maps linear input to exponential output (most abrupt fade in, then ramp up slow (faster fade in compared to smoothExp3))
    /// input range [0..1], outputs range [0..1]
    SYNFORCEINLINE float smoothExpT_x6( float x )
    {
        x = 1.0f - x;
        float x6 = x*x;
        x6 *= x6*x6;
        return 1.0f - x6;
    }

    /// halving (or some other fraction) smoothing routine (iterative result).  result is an exponential curve...
    /// filters an input value towards some end value, cutting it in [factor] each time. (usually an abrupt fade in, then ramp up slow)
    /// input range [0..1], outputs range [0..1]
    SYNFORCEINLINE float smoothGoal( float currentvalue, float goalvalue, float factor )
    {
        const float den = 1e-15f; // avoid denormal numbers, on intel these kinds of numbers suck cpu %
        const float dist_to_end = goalvalue - currentvalue;
        const float stride_this_frame = dist_to_end * factor + den;
        return currentvalue + stride_this_frame;
    }

      /** Linear approx. between 2 integer values of val. 
       *  Uses 32-bit integers. Not very efficient but fastest than exp()
       *  This code detects both big and little endian processors.
       *  @see musicdsp.org
       */
      inline double exp2Est( const double val ) 
      {
         // is the machine little endian?
         union
         {
            short   val;
            char    ch[sizeof( short )];
         } un;
         un.val = 256; // 0x10;
         // if un.ch[1] == 1 then we're little

         // return 2 to the power of val (exp base2)
         int    e;
         double ret;

         if (val >= 0)
         {
            e = int (val);
            ret = val - (e - 1);

            if (un.ch[1] == 1)
               ((*(1 + (int *) &ret)) &= ~(2047 << 20)) += (e + 1023) << 20;
            else
               ((*((int *) &ret)) &= ~(2047 << 20)) += (e + 1023) << 20;
         }
         else
         {
            e = int (val + 1023);
            ret = val - (e - 1024);

            if (un.ch[1] == 1)
               ((*(1 + (int *) &ret)) &= ~(2047 << 20)) += e << 20;
            else
               ((*((int *) &ret)) &= ~(2047 << 20)) += e << 20;
         }

         return ret;
      }

      /** This code uses IEEE 32-bit floating point representation 
       *  knowledge to quickly compute approximations to the Log2 of 
       *  a value. This function returns an under-estimate of the actual 
       *  value, although it might be sufficient for using in, say, a 
       *  dBV/FS level meter.
       *  @see musicdsp.org
       */
      SYNFORCEINLINE float Log2Est( const register float f )
      {
        assert( f > 0.0f );
        //assert( sizeof(f) == sizeof(int) );
        //assert( sizeof(f) == 4 );
        int i = (*(int *)&f);
        return (((i&0x7f800000)>>23)-0x7f)+(i&0x007fffff)/(float)0x800000;
      }
      SYNFORCEINLINE float Log2Est_2( register float val )
    {
       assert (val > 0.0f);
       int * const  exp_ptr = reinterpret_cast <int * const > (&val);
       int          x = *exp_ptr;
       const int    log_2 = ((x >> 23) & 255) - 128;
       x &= ~(255 << 23);
       x += 127 << 23;
       *exp_ptr = x;
       return (val + log_2);
    }

      /// log base 2
      template <typename T>
      SYNFORCEINLINE T Log2( const T f );
      SYNFORCEINLINE double Log2( const double f )
      {
        //static const double conversion = ::log10( 2.0 );
        //return log10( f ) / conversion;
        return ::log10( f ) * 3.321928;    // 3.321928 == 1.0f / ::log10( 2.0 )
      }
      SYNFORCEINLINE float Log2( const float f )
      {
        //static const float conversion = ::log10f( 2.0f );
        //return ::log10f( f ) / conversion;
        return ::log10( f ) * 3.321928f;   // 3.321928 == 1.0f / ::log10( 2.0 )
      }

      
/*
      ///  estamate pow() for doubles...   (not accurate enough for note2Freq...)
      double powEst_3(double a, double b)
      {
        int tmp = (*(1 + (int*)&a));
        int tmp2 = (int)(b * (tmp - 1072632447) + 1072632447);
        double p = 0.0;
        *(1 + (int*)&p) = tmp2;
        return p;
      }

      /// estimate pow() for floats...
      float powEst_2(float f,int n)
      {
          long *lp,l;
          lp=(long*)(&f);
          l=*lp;l-=0x3F800000l;l<<=(n-1);l+=0x3F800000l;
          *lp=l;
          return f;
      }

      /// estimate sqrt() for floats...
      float sqrtEst(float f,int n)
      {
          long *lp,l;
          lp=(long*)(&f);
          l=*lp;l-=0x3F800000l;l>>=(n-1);l+=0x3F800000l;
          *lp=l;
          return f;
      }
*/
      // FastPow http://blog.quenta.org/2012/09/0x5f3759df.html
      // raise x to the power of p  (x^p), when p is between [-1..1]
      float PowEst(float x, float p)
      {
         float xhalf = 0.5f * x;
         int i = *(int*)&x;         // evil floating point bit level hacking
         //﾿magic﾿ part of fast exponentiation for any exponent between -1 and 1
         i = 0x3f7a3bea - p * (0x3f7a3bea + i);  // what the fuck?
         x = *(float*)&i;

         //run single iteration of Newton﾿s method to improve approximation.
         x = x*(1.5f-(xhalf*x*x));
         return x;
      }

      // FastSqrt http://blog.quenta.org/2012/09/0x5f3759df.html
      float SqrtEst(float x)
      {
         float xhalf = 0.5f * x;
         int i = *(int*)&x;         // evil floating point bit level hacking
         i = 0x1fbd1df5 + (i >> 1);  // what the fuck?
         x = *(float*)&i;

         //run single iteration of Newton﾿s method to improve approximation.
         x = x*(1.5f-(xhalf*x*x));
         return x;
      }

      // FastCubeRoot http://blog.quenta.org/2012/09/0x5f3759df.html
      float CubeRootEst(float x)
      {
         float xhalf = 0.5f * x;
         int i = *(int*)&x;         // evil floating point bit level hacking
         i = (int) (0x2a517d3c + (0.333f * i));  // what the fuck?
         x = *(float*)&i;

         //run single iteration of Newton﾿s method to improve approximation.
         x = x*(1.5f-(xhalf*x*x));
         return x;
      }

      // Quake3 FastInvSqrt
      float SqrtREst(float x)
      {
         float xhalf = 0.5f * x;
         int i = *(int*)&x;         // evil floating point bit level hacking
         i = 0x5f3759df - (i >> 1);  // what the fuck?
         x = *(float*)&i;

         //run single iteration of Newton﾿s method to improve approximation.
         x = x*(1.5f-(xhalf*x*x));
         return x;
      }
/*
        // ~6 clocks on Pentium M vs. ~24 for single precision sqrtf   (11bits)
      SYNFORCEINLINE float sqrtEst_2( float x )
        {
            float z;
            _asm
            {
                rsqrtss xmm0, x
                rcpss    xmm0, xmm0
                movss    z, xmm0            // z ~= sqrt(x) to 0.038%
            }
            return z;
        }


      // ~19 clocks on Pentium M vs. ~24 for single precision sqrtf  (squareroot_sse_22bits)
    SYNFORCEINLINE float sqrtEst_3( float x )
    {
        static float half = 0.5f;
        float z;
        _asm
        {
            movss    xmm1, x            // x1: x
            rsqrtss xmm2, xmm1        // x2: ~1/sqrt(x) = 1/z
            rcpss    xmm0, xmm2        // x0: z == ~sqrt(x) to 0.05%
            
            movss    xmm4, xmm0        // x4: z
            movss    xmm3, half
            mulss    xmm4, xmm4        // x4: z*z
            mulss    xmm2, xmm3        // x2: 1 / 2z
            subss    xmm4, xmm1        // x4: z*z-x
            mulss    xmm4, xmm2        // x4: (z*z-x)/2z
            subss    xmm0, xmm4        // x0: z' to 0.000015%
            
            movss    z, xmm0          
        }
        return z;
    }

    // ~12 clocks on Pentium M vs. ~16 for single precision divide (reciprocal_sse_22bits)
    REALLY_INLINE float Reciprocal_2( float x )
    {
        float z;
        _asm
        {
            rcpss    xmm0, x            // x0: z ~= 1/x
            movss    xmm2, x            // x2: x
            movss    xmm1, xmm0        // x1: z ~= 1/x
            addss    xmm0, xmm0        // x0: 2z
            mulss    xmm1, xmm1        // x1: z^2
            mulss    xmm1, xmm2        // x1: xz^2
            subss    xmm0, xmm1        // x0: z' ~= 1/x to 0.000012%

            movss    z, xmm0          
        }
        return z;
    }
    */

      /*
      // Taylor series exp() approximations
      // from musicdsp
      // References : Posted by scoofy[AT]inf[DOT]elte[DOT]hu
      I needed a fast exp() approximation in the -3.14..3.14 range, so I made some approximations based on the tanh() 
      code posted in the archive by Fuzzpilz. Should be pretty straightforward, but someone may find this useful.

    The increasing numbers in the name of the function means increasing precision. Maximum error in the -1..1 range:
    expEst3: 0.05 (1.8%)
    expEst4: 0.01 (0.36%)
    expEst5: 0.0016152 (0.59%)
    expEst6: 0.0002263 (0.0083%)
    expEst7: 0.0000279 (0.001%)
    expEst8: 0.0000031 (0.00011%)
    expEst9: 0.0000003 (0.000011%)

    Maximum error in the -3.14..3.14 range:
    expEst3: 8.8742 (38.4%)
    expEst4: 4.8237 (20.8%)
    expEst5: 2.28 (9.8%)
    expEst6: 0.9488 (4.1%)
    expEst7: 0.3516 (1.5%)
    expEst8: 0.1172 (0.5%)
    expEst9: 0.0355 (0.15%)

      These were done using the Taylor series, for example I got fastexp4 by using:
    exp(x) = 1 + x + x^2/2 + x^3/6 + x^4/24 + ...
    = (24 + 24x + x^2*12 + x^3*4 + x^4) / 24
    (using Horner-scheme:)
    = (24 + x * (24 + x * (12 + x * (4 + x)))) * 0.041666666f
    */

    //  exp3 where "3" is the accuracy...   (3 is worst, 9 is highest)
    SYNFORCEINLINE float expEst3(const register float x) {
    return (6.0f+x*(6.0f+x*(3.0f+x)))*0.16666666f;
    }

    SYNFORCEINLINE float expEst4(const register float x) {
    return (24.0f+x*(24.0f+x*(12.0f+x*(4.0f+x))))*0.041666666f;
    }

    SYNFORCEINLINE float expEst5(const register float x) {
    return (120.0f+x*(120.0f+x*(60.0f+x*(20.0f+x*(5.0f+x)))))*0.0083333333f;
    }

    SYNFORCEINLINE float expEst6(const register float x) {
    return (720.0f+x*(720.0f+x*(360.0f+x*(120.0f+x*(30.0f+x*(6.0f+x))))))*0.0013888888f;
    }

    SYNFORCEINLINE float expEst7(const register float x) {
    return (5040.0f+x*(5040.0f+x*(2520.0f+x*(840.0f+x*(210.0f+x*(42.0f+x*(7.0f+x)))))))*0.00019841269f;
    }

    SYNFORCEINLINE float expEst8(const register float x) {
    return (40320.0f+x*(40320.0f+x*(20160.0f+x*(6720.0f+x*(1680.0f+x*(336.0f+x*(56.0f+x*(8.0f+x))))))))*2.4801587301e-5f;
    }

    SYNFORCEINLINE float expEst9(const register float x) {
    return (362880.0f+x*(362880.0f+x*(181440.0f+x*(60480.0f+x*(15120.0f+x*(3024.0f+x*(504.0f+x*(72.0f+x*(9.0f+x)))))))))*2.75573192e-6f;
    }
    // max error in the 0 .. 7.5 range: ~0.45%
    // NOTE: exp9Est() is cheaper than extEst() (but exp9Est() is not accurate above 3.14)
    SYNFORCEINLINE float expEstZeroToSeven(const register float x)
    {
      if (x < 2.5f)
          return 2.7182818f * expEst5( x - 1.0f );
      else if (x < 5.0f)            
          return 33.115452f * expEst5( x - 3.5f );
      else                          
          return 403.42879f * expEst5( x - 6.0f );
    }

    /// do e^x   (simulated exp(), accurate for a given range only)
    /// val is in the range -6.931472 to -0.693148    (or where val is 0..1 range in the equation ((11.0f * val - 10.0f) * log(2)) )
    /// @see smoothExp2Fast
    SYNFORCEINLINE float expEstNeg7ToNegHalf(const register float val)
    {
        if (-2.341456f <= val)
        {
            return expEst9( val );
        }
        // look for val == -2.341431 at this point...
        else
        {
            //return 2.0f * 0.000832f + expEst9( val + 2.341456f + 1.0f ) * 0.034350f;
            return          0.001664f + expEst9( val + 3.341456f        ) * 0.034350f;
        }
    }

    // does pow(2.0f, x)    (same as exp2)
    SYNFORCEINLINE float pow2(const register float x)
    {
        return exp( x * 0.6931472f );  // 0.6931472f == log(2)   should be faster than doing the more generic pow
        //return pow( 2.0f, x );
    }

    // does exp2( x )   (same as pow(2,x))
    SYNFORCEINLINE float exp2(const register float x)
    {
      return exp( x * 0.6931472f );  // 0.6931472f == log(2)
    }

    // does pow(2.0f, x)   (same as exp2Est)
    // max error in the 0-10.58 range: ~0.45%
    SYNFORCEINLINE float pow2Est(const register float x)
    {
      //const register float const log_two = 0.6931472f;
      return expEstZeroToSeven(x * 0.6931472f); // 0.6931472f == log(2)
      //return pow2( x );
    }


    /// exponential smoothing routine (pow(2, x) curve)
    /// maps linear input to 2^x exponential output (fade in slow, then ramp up quick at the end)
    /// input is [0..1], output is also [0..1] but on an exponential curve as defined by exp2Fast()
    /// NOTE: use this if you're looking for exponential mapping useful for types of things like:
    ///  - gain (amplitude) to db (decibles) conversion ( i.e. volume knob value (linear) to speaker output amount (exponential human eardrum perception))
    ///  - musical note (linear) to frequency calculation (exponential)
    SYNFORCEINLINE float smoothExp2Fast( const float val )
    {
        // 0.6931472f == log(2)
        return expEstNeg7ToNegHalf( (11.0f * val - 10.0f) * 0.6931472f )* 0.5f - 0.000000834f;  // 0.000000834f fudge...
        //return (float)exp2Est( 11.0f * val - 10.0f ) * 0.5f;
    }

    // very inaccurate, but very fast.
    SYNFORCEINLINE float smoothExp2FastFast( const float val )
    {
        float v = expEst5( val * 1.1f ) - 0.9998293f;
        float vv = v * v;
        return vv * vv * 0.06232165800f; // fudge
    }

    SYNFORCEINLINE float smoothExp2FastFastT( const float val )
    {
        return 1.0f - smoothExp2FastFast( 1.0f - val ); // invert input, and invert output...
    }

    /// reference version (expensive)   (see notes for smoothExp2Fast)
    SYNFORCEINLINE float smoothPow2( const float val )
    {
        return (float)pow2( 11.0f * val - 10.0f ) * 0.5f;
    }

    /// reference version (expensive)   (see notes for smoothExp2Fast)
    SYNFORCEINLINE float smoothExp2( const float val )
    {
        return (float)exp2( 11.0f * val - 10.0f ) * 0.5f;
    }


	/// exponential smoothing routine (using exp2Fast())  (Transposed)
    /// maps linear input to exponential output (fade in fast, then ramp up slow at the end)
    /// input is [0..1], output is also [0..1] but on an exponential curve as defined by exp2Fast()
    SYNFORCEINLINE float smoothExp2FastT( const float val )
    {
        return 1.0f - smoothExp2Fast( 1.0f - val ); // invert input, and invert output...
    }

      /// note value to control voltage (1V per octave)
      /// http://tomscarff.tripod.com/midi_analyser/midi_note_frequency.htm   (midi note to freq chart).
      SYNFORCEINLINE float cv2freq( const register float cv )
      {
         // do pow(2,x) to the cv...
          const register float cv_pow2 = syn::Math::pow2Est( cv ); //< est seems accurate enough (a little deviation on the highest octave), and is ~37x faster than pow2()  (on p4 2.26ghz  x86 family 15 model 2)
          //const register float cv_pow2 = syn::Math::pow2( cv );
          return cv_pow2 * 8.1757989156f;  // frequency of C0
      }

      /// control voltage to note (1V per octave)
      SYNFORCEINLINE float cv2note( const register float cv )
      {
         return cv * 12.0f;
      }


      /// note value to control voltage (1V per octave)
      SYNFORCEINLINE float note2cv( const register float note )
      {
         return note * 0.0833333333333f; // div by 12...
      }

      /** convert note to frequency.
       *  @param note is in midi note format (where middle C is 60)
       *  @post frequency is in herz
       */
      SYNFORCEINLINE float note2freq( const register float note )
      {
          return syn::Math::cv2freq( syn::Math::note2cv( note ) );
      }


      /// convert frequency to control voltage.
      SYNFORCEINLINE float freq2cv( const register float freq )
      {
         // reverse what note2Freq does...
         float c0freqInv = 1.0f / 8.1757989156f; // frequency of C0
         return syn::Math::Log2( freq * c0freqInv ); // Log2Est just doesn't work, i get repeating numbers in freq2Note()...
      }

      /// convert frequency to midi note number.
      SYNFORCEINLINE float freq2note( const register float freq )
      {
         return syn::Math::round( syn::Math::cv2note( syn::Math::freq2cv( freq ) ) );
      }




      //First the easy way which is usually good enough:
      //just use x squared or x cubed.

      //Second the more complicated way:
      /*

      These two functions reproduce a traditional professional
      mixer fader taper.

      VolumeToDB converts volume (0 to 100%) starting from the
      bottom of the fader into decibels. 
      DBtoVolume is the inverse.

      The taper is as follows from the top:
      The top of the fader is +10 dB
      100 to 52 : -5 dB per 12
      52 to 16 : -10 dB per 12
      16 to 4 : -20 dB per 12
      4 to 0 : fade to zero. (in these functions I go to -200dB
      which is effectively zero for up to 32 bit audio.)

      */

      inline float volumeToDB( float vol )
      {
              float db;

              vol = 100.0f - vol;

              if (vol <= 0.0f) {
                      db = 10.0f;
              } else if (vol < 48.0f) {
                      db = 10.0f - 5.0f/12.0f * vol;
              } else if (vol < 84.0f) {
                      db = -10.0f - 10.0f/12.0f * (vol - 48.0f);
              } else if (vol < 96.0f) {
                      db = -40.0f - 20.0f/12.0f * (vol - 84.0f);
              } else if (vol < 100.0f) {
                      db = -60.0f - 35.0f * (vol - 96.0f);
              } else db = -200.0f;
              return db;
      }

      inline float DBtoVolume( float db )
      {
              float vol;
              if (db >= 10.0f) {
                      vol = 0.0f;
              } else if (db > -10.0f) {
                      vol = -12.0f/5.0f * (db - 10.0f);
              } else if (db > -40.0f) {
                      vol = 48.0f - 12.0f/10.0f * (db + 10.0f);
              } else if (db > -60.0f) {
                      vol = 84.0f - 12.0f/20.0f * (db + 40.0f);
              } else if (db > -200.0f) {
                      vol = 96.0f - 1.0f/35.0f * (db + 60.0f);
              } else vol = 100.0f;

              vol = 100.0f - vol;

              return vol;
      }

      /// given 0..1 linear volume knob value, returns curved 0..1 value suited for db response.
      inline float unitVolumeToUnitDB( float zeroToOne )
      {
          return syn::Math::remap( syn::Math::volumeToDB( 100.0f * zeroToOne ), -200.0f, 10.0f, 0.0f, 1.0f );
      }
/*
//  http://www.stereopsis.com/Log2.html
typedef unsigned long uint32;
typedef long int32;

static inline int32 iLog2(uint32 x) {
	return iLog2((float)x);
}

// integer Log2 of a float
static inline int32 iLog2(float x)
{
	uint32 ix = (uint32&)x;
	uint32 exp = (ix >> 23) & 0xFF;
	int32 Log2 = int32(exp) - 127;

	return Log2;
}

static inline int32 iLog2_x86(uint32 x) 
{
	int32 retval;
	__asm {
		bsr eax, x
		mov retval, eax
	}
	return retval;
}

*/
   } // end of Math namespace

   //-------------------------------------------------------------------------------------------------------
   // Strings Conversion
   //-------------------------------------------------------------------------------------------------------
   //-------------------------------------------------------------------------------------------------------
   inline void float2string (float value, char *text)
   {
	   long c = 0, neg = 0;
	   char string[32];
	   char *s;
	   double v, integ, i10, mantissa, m10, ten = 10.;
      
	   v = (double)value;
	   if (v < 0)
	   {
		   neg = 1;
		   value = -value;
		   v = -v;
		   c++;
		   if (v > 9999999.)
		   {
			   strcpy (string, "Huge!");
			   return;
		   }
	   }
	   else if( v > 99999999.)
	   {
		   strcpy (string, "Huge!");
		   return;
	   }

	   s = string + 31;
	   *s-- = 0;
	   *s-- = '.';
	   c++;
      
	   integ = floor (v);
	   i10 = fmod (integ, ten);
	   *s-- = (char)((long)i10 + '0');
	   integ /= ten;
	   c++;
	   while (integ >= 1. && c < 8)
	   {
		   i10 = fmod (integ, ten);
		   *s-- = (char)((long)i10 + '0');
		   integ /= ten;
		   c++;
	   }
	   if (neg)
		   *s-- = '-';
	   strcpy (text, s + 1);
	   if (c >= 8)
		   return;

	   s = string + 31;
	   *s-- = 0;
	   mantissa = fmod (v, 1.);
	   mantissa *= pow (ten, (double)(8 - c));
	   while (c < 8)
	   {
		   if (mantissa <= 0)
			   *s-- = '0';
		   else
		   {
			   m10 = fmod (mantissa, ten);
			   *s-- = (char)((long)m10 + '0');
			   mantissa /= 10.;
		   }
		   c++;
	   }
	   strcat (text, s + 1);
   }

   inline void dB2string (float value, char *text)
   {
	   if (value <= 0)
	   #if MAC
		   strcpy (text, "-ﾰ");
	   #else
		   strcpy (text, "-oo");
	   #endif
	   else
		   float2string ((float)(20. * log10 (value)), text);
   }

   //-------------------------------------------------------------------------------------------------------
   inline void Hz2string (float sampleRate, float samples, char *text)
   {
	   if (!samples)
		   float2string (0, text);
	   else
		   float2string (sampleRate / samples, text);
   }

   //-------------------------------------------------------------------------------------------------------
   inline void ms2string (float sampleRate, float samples, char *text)
   {
	   float2string ((float)(samples * 1000. / sampleRate), text);
   }



   //-------------------------------------------------------------------------------------------------------
   inline void long2string (long value, char *text)
   {
	   char string[32];
      
	   if (value >= 100000000)
	   {
		   strcpy (text, "Huge!");
		   return;
	   }
	   sprintf (string, "%7d", (int)value);
	   string[8] = 0;
	   strcpy (text, (char *)string);
   }
   //-------------------------------------------------------------------------------------------------------
   //-------------------------------------------------------------------------------------------------------


    class PeakFollower
    {
    public:
       PeakFollower() : output( 0.0f ), halfLife( 0.05f ) {}
       float peakFollower( float input, float samprate )
       {
	      float scalar = powf( 0.5f, 1.0f/(halfLife * samprate));

	      if( input < 0.0 )
		     input = -input;  /* Absolute value. */

	      if ( input >= output )
	      {
		     /* When we hit a peak, ride the peak to the top. */
		     output = input;
	      }
	      else
	      {
		     /* Exponential decay of output when signal is low. */
		     output = output * scalar;
		     
             // avoid denormal numbers, and bad performance on intel due to it..
             static float flippy = 1.0000e-15f;
		     output += flippy;
		     flippy = -flippy;
	      }
	      return output;
       }
       float output;
       
       // halfLife = time in seconds for output to decay to half value after an impulse
       float halfLife;
    };

   class Plane
   {
      public:
      // if true, then isect_point = ray_o + ray_v * t
      bool intersect( float& t, vmVec3 ray_o, vmVec3 ray_v )
      {
         //t = -(n dot P + d)
         float denom = dot( n, ray_v );
         float eps = 0.00001f;
         if (fabsf( denom ) < eps)
         {
            printf( "intersect oops: true = %f < %f\n", denom, eps );
            printf( "intersect oops: %s = distance(ray_v=%f) < eps=%f\n",
                     distance(ray_v) < eps ? "true" : "false",
                     distance(ray_v), eps );
            assert( false && "todo: some testing, never been in here before?" );
            if (distance(ray_v) < eps)
               return true;
            else
               return false;
         }
         t = dot( n, n * d - ray_o ) / denom;
         return true;
      }
      float distance( vmVec3 p )
      {
         //|ax+by+cz-d| / sqrt(a^2 + b^2 + c^2)
         return (dot( p, n ) - d) / sqrtf( dot( p, p ) );
      }
      vmVec3 n; // plane normal
      float d; // distance along the normal to the origin
   };


   // winx and winy are in -1..1 range.
   inline vmVec3 window2world( float winx, float winy,
                           const vmMatrix4& proj2world )
   {
      vmVec3 v( winx, winy, 0.0f );
      v[1] *= -1.0f; // flip y

      vmVec4 line_near( v[0], v[1], 1.0f, 1.0f ); // far side of unit cube
      vmVec4 line_far( v[0], v[1], 0.0f, 1.0f );  // near side of unit cube
      line_near = proj2world * line_near;   // world near plane mouse pt
      line_near = line_near / line_near[3];
      line_far = proj2world * line_far;     // world far plane mouse pt
      line_far = line_far / line_far[3];
      vmVec3 ray_v = line_far.getXYZ() - line_near.getXYZ();
      ray_v = normalize( ray_v );           // world mouse vector

      // plane in world space, where the 2D stuff is
      Plane p;
      p.n = vmVec3( 0,0,-1 );
      p.d = 0.0f;

      // intersect mouse ray and Box2D Plane (at 0 depth)
      float t = 0;
      p.intersect( t, line_near.getXYZ(), ray_v );
      vmVec3 isect = line_near.getXYZ() + ray_v * t;

      float eps = 0.00001f;
#if _DEBUG
      if (!(fabsf( isect[2] ) < eps))
      {
         printf( "proj2world: \n" );
         print( proj2world );
         printf( "isect[2]:[%f] winx,winy:[%f %f] t:[%f] ray_v:[%f %f %f] line_near:[%f %f %f]\n", isect[2], winx, winy, t, ray_v[0], ray_v[1], ray_v[2], line_near.getXYZ()[0], line_near.getXYZ()[1], line_near.getXYZ()[2] );
      }
      assert( fabsf( isect[2] ) < eps && "matrix passed to window2world invalid (did you initialize it to vmMatrix4::identity()?)" );
#endif
      return vmVec3( isect[0], isect[1], 0.0f );
   }
   inline vmVec3 window2world( float winx, float winy,
                        float winwidth, float winheight,
                        const vmMatrix4& proj2world )
   {
      assert( winwidth != 0 && winheight != 0 && "winwidth or winheight was 0, not valid" );
      return window2world( (winx / winwidth) * 2.0 - 1.0f,
                           (winy / winheight) * 2.0 - 1.0f,
                           proj2world );
   }

} // end of syn namespace

#endif
