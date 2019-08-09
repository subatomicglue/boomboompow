
#ifndef PARSER_H
#define PARSER_H

// parser:
namespace p
{

   inline bool IsCharEqual( char c, const char values[] )
   {
      int x = 0;
      while (values[x] != '\0')
      {
         if (c == values[x]) return true;
         ++x;
      }
      return false;
   }
   inline bool IsCharNumeric( char c )
   {
      return 48<=c && c<=57;
   }
   inline bool IsCharAlpha( char c )
   {
      return (65<=c && c<=90) || (97<=c && c<=122);
   }
   inline bool IsCharAlphaNumeric( char c )
   {
      return IsCharAlpha( c ) || IsCharNumeric( c );
   }
   inline void Eat( const char c[], int& pos, const std::string& text )
   {
      for (int x = pos; x < text.size() && IsCharEqual( text[x], c ); ++x)
      {
         ++pos;
      }
   }
   inline bool NextChar( const char c[], int& pos, const std::string& text )
   {
      Eat( " \t\n\r", pos, text ); // eat whitespace
      if (IsCharEqual( text[pos], c ))
      {
         ++pos;
         Eat( " \t\n\r", pos, text ); // eat whitespace
         return true;
      }
      return false;
   }
   // typename or varname
   inline std::string NextName( int& pos, const std::string& text, const char okDelimiters[] = "_-" )
   {
      Eat( " \t\n\r", pos, text ); // eat whitespace
      std::string retval;
      for (int x = pos;
           x < text.size() && (IsCharAlphaNumeric( text[x] ) ||
                               IsCharEqual( text[x], okDelimiters ));
           ++x)
      {
         retval += text[x];
         ++pos;
      }
      Eat( " \t\n\r", pos, text ); // eat whitespace
      return retval;
   }
   inline std::string NextFilename( int& pos, const std::string& text )
   {
      Eat( " \t\n\r", pos, text ); // eat whitespace
      // in quotes
      if (NextChar( "\"'", pos, text ))
      {
         std::string retval;
         while (!IsCharEqual( text[pos], "\"'" ))
         {
            retval += text[pos];
            ++pos;
         }
         NextChar( "\"'", pos, text );
         return retval;
      }

      // not in quotes...
      return NextName( pos, text, "_.-" );
   }
   inline bool NextBool( int& pos, const std::string& text )
   {
      std::string result = NextName( pos, text );
      return result == "true" || result == "1";
   }
   inline float NextFloat( int& pos, const std::string& text )
   {
      Eat( " \t\n\r", pos, text ); // eat whitespace
      std::string retval;
      for (int x = pos; x < text.size() && (IsCharNumeric( text[x] ) || IsCharEqual( text[x], ".-+" )); ++x)
      {
         retval += text[x];
         ++pos;
      }
      Eat( " \t\n\r", pos, text ); // eat whitespace
      return atof( retval.c_str() );
   }
   inline float NextInt( int& pos, const std::string& text )
   {
      Eat( " \t\n\r", pos, text ); // eat whitespace
      std::string retval;
      for (int x = pos;
           x < text.size() && (IsCharNumeric( text[x] ) ||
                               IsCharEqual( text[x], "-+" )); ++x)
      {
         retval += text[x];
         ++pos;
      }
      Eat( " \t\n\r", pos, text ); // eat whitespace
      return atoi( retval.c_str() );
   }

   class FileName : public std::string
   {
   public:
      FileName& operator=( const std::string& s ) { std::string::operator=(s); return *this; }
      FileName& operator=( const FileName& s ) { std::string::operator=(s); return *this; }
   };

   inline std::string ToString( bool val )
   {
      return std::string( val ? "true" : "false" );
   }
   inline std::string ToString( float val )
   {
      char buf[256]; sprintf( buf, "%.03f", val ); return std::string( buf );
   }
   inline std::string ToString( int val )
   {
      char buf[256]; sprintf( buf, "%d", val ); return std::string( buf );
   }
   inline const std::string& ToString( const std::string& str )
   {
      return str;
   }
   inline std::string ToString( const FileName& str )
   {
      return std::string( "\"" ) + str + "\"";
   }
} // parser

#endif

