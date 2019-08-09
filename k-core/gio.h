

#ifndef GIO_BASE_FILES
#define GIO_BASE_FILES

#include <iostream>
#include <algorithm>
#include <fstream>
#include <string>
#include <list>
#include <cstdarg>
#include <vector>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <assert.h>
#ifdef WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>

namespace gio
{
   // return the current working directory
   inline std::string cwd()
   {
      char path[1024];
#ifdef WIN32
      if (!_getcwd( path, sizeof( path ) ))
#else
      if (!getcwd( path, sizeof( path ) ))
#endif
      {
         return "";
      }
      return path;
   }

   // set the current working directory
   inline void chdir( const char* dir )
   {
      char path[1024];
#ifdef WIN32
      ::_chdir( dir );
#else
      ::chdir( dir );
#endif
   }

   // allows you to do if (c == Separator()) to check if a path separator like \ or /
   struct Separator
   {
      // scan string to determine separator type used.
      char find( const std::string& s )
      {
         for (size_t x=0;x<s.size();++x)
            if (s[x] == '/' || s[x] == '\\')
               return s[x];
#ifdef WIN32
         return '\\';
#else
         // handle "c:" case
         if (s.size() > 1 && s[1] == ':') return '\\'; else return '/';
#endif
      }
   };
   inline bool operator==( char c, Separator s ) { return c == '/' || c == '\\'; }
   inline bool operator!=( char c, Separator s ) { return c != '/' && c != '\\'; }

   // return native filesystem OS separator
#ifdef WIN32
   char NativeSeparator() { return '\\'; }
#else
   char NativeSeparator() { return '/'; }
#endif

   // check if path is reletive or absolute
   inline bool isAbsolutePath( const std::string& path )
   {
      Separator sep;
      return
         ((3 <= path.size() &&
           (('a' <= path[0] && path[0] <= 'z') ||
            ('A' <= path[0] && path[0] <= 'Z')) &&
             path[1] == ':' &&
             path[2] == sep) ||                                      // windows 'c:\'
          (1 <= path.size() && path[0] == sep) ||                    // win/unix '/' or '\'
          (2 <= path.size() && path[0] == '\\' && path[1] == '\\')); // UNC network share '\\'
   }

   /*
   * Given any Unix or Windows or UNC path for a file, simplify it and make it absolute.
   * For example,
   * path = "/home/", => "/home"
   * path = "/a/./b/../../c/", => "/c"
   * path = "./debug", => "/home/kevin/src/proj/debug"
   * Corner Cases:
   * Did you consider the case where path = "/../"?
   * In this case, you should return "/".
   * Another corner case is the path might contain multiple slashes '/' together, such as "/home//foo/".
   * In this case, you should ignore redundant slashes and return "/home/foo".
   */
   inline std::string simplifyPath( std::string path )
   {
      std::list<std::string> s; //stack
      char c;
      unsigned int pos = 0;
      std::string prefix;
      Separator sep;

      // pick a separator to use so that the path is unified with only one kind
      std::string separator = "/";
      separator[0] = sep.find( path );

      // relative path "dir" ".file" ".."
      if (!isAbsolutePath( path ))
      {
         path = cwd() + separator + path;
      }

      // special case: path prefix
      // windows "c:\" absolute path
      if ((('a' <= path[0] && path[0] <= 'z') ||
           ('A' <= path[0] && path[0] <= 'Z')) &&
            path[1] == ':' &&
            path[2] == sep)
      {
         prefix = "c:";
         prefix[0] = path[0];
         pos += 3;
      }
      // windows UNC network share "\\"
      else if (path[0] == '\\' && path[1] == '\\')
      {
         prefix = "\\";
         pos += 2;
      }
      // unix/windows absolute path "/"
      else if (path[0] == sep)
      {
         pos += 1;
      }

      // sanity check, first char of path needs to be one of the following
      if (path[0] == sep || path[0] == '.'
            || ('a' <= path[0] && path[0] <= 'z')
            || ('A' <= path[0] && path[0] <= 'Z')
         )
      {
         // great!
      }
      else
      {
         return path;
      }
      while (pos < path.size())
      {
         c = path[pos];
         // separator /
         if (c == sep)
         {
            while (pos < path.size() && path[pos] == c)
            {
               pos++;
            }
         }
         // .. removes previous dir
         // .\0  just ignore
         // ./  just ignore
         else if (c == '.' &&
                  (
                   (pos + 1) >= path.size() ||
                   ((pos + 1) < path.size() &&
                    (path[pos + 1] == '.' || path[pos + 1] == sep))
                  )
                 )
         {
            if ((pos + 1) < path.size() && path[pos + 1] == '.')
            {
               // ..
               if (s.size() > 0)
               {
                  s.pop_back();
               }
               pos += 2;
            }
            else
            {
               // .
               pos++;
            }
         }
         else
         {
            // letter, or a dot followed by !dot && !sep
            unsigned int i = pos;
            while (i < path.size() && path[i] != sep)
            {
               i++;
            }

            std::string name = path.substr( pos, i - pos );
            if (name.size())
            {
               s.push_back( name );
            }
            pos = i;
         }
      }

      if (s.size() == 0)
      {
         if (prefix.size())
            return prefix;
         else
            return separator;
      }
      std::string simplified = "";
      while (s.size() > 0)
      {
         std::string name = s.back();
         s.pop_back();
         simplified = separator + name + simplified;
      }
      return prefix + simplified;
   }

   inline void simplifyPathTest()
   {
      assert( isAbsolutePath( "c:\\" ) == true );
      assert( isAbsolutePath( "t:/" ) == true );
      assert( isAbsolutePath( "/" ) == true );
      assert( isAbsolutePath( "\\" ) == true );
      assert( isAbsolutePath( "\\\\terra\\" ) == true );
      assert( isAbsolutePath( "." ) == false );
      assert( isAbsolutePath( ".." ) == false );
      assert( isAbsolutePath( "./" ) == false );
      assert( isAbsolutePath( "Debug" ) == false );
      assert( isAbsolutePath( ".svn\\" ) == false );
      assert( simplifyPath( "/." ) == "/" );
      assert( simplifyPath( "/../" ) == "/" );
      assert( simplifyPath( "/../." ) == "/" );
      assert( simplifyPath( "/.././" ) == "/" );
      assert( simplifyPath( "/../../..///../.././///" ) == "/" );
      assert( simplifyPath( "/home/kevin" ) == "/home/kevin" );
      assert( simplifyPath( "/home/kevin/" ) == "/home/kevin" );
      assert( simplifyPath( "/home/kevin/.." ) == "/home" );
      assert( simplifyPath( "/home/kevin/../" ) == "/home" );
      assert( simplifyPath( "/home/kevin/../dave/./bok/hi///x/../../../././/" ) == "/home/dave" );
      assert( simplifyPath( "." ) == cwd() );
      assert( simplifyPath( "./" ) == cwd() );
      assert( simplifyPath( "./.." ) == simplifyPath( cwd() + "/.." ) );
      assert( simplifyPath( "./../" ) == simplifyPath( cwd() + "/.." ) );
      assert( simplifyPath( "./../." ) == simplifyPath( cwd() + "/.." ) );
      assert( simplifyPath( "./.././" ) == simplifyPath( cwd() + "/.." ) );
      assert( simplifyPath( ".." ) == simplifyPath( cwd() + "/.." ) );
      assert( simplifyPath( ".conf" ) == cwd() + "/.conf" );
      assert( simplifyPath( "Debug" ) == cwd() + "/Debug" );
      assert( simplifyPath( "Debug/" ) == cwd() + "/Debug" );
      assert( simplifyPath( "/.bok" ) == "/.bok" );
      assert( simplifyPath( "/.bok/" ) == "/.bok" );
      assert( simplifyPath( "/.bok././bah/../" ) == "/.bok." );
      assert( simplifyPath( "./src/data/include/../lib-win32" ) == cwd() + "/src/data/lib-win32" );
      assert( simplifyPath( "/home/kevin/.dotfile" ) == "/home/kevin/.dotfile" );
      assert( simplifyPath( "/home/kevin/.dotfile/" ) == "/home/kevin/.dotfile" );
      assert( simplifyPath( "/home/kevin/.dotfile/.." ) == "/home/kevin" );
      assert( simplifyPath( "/home/kevin/.dotfile/." ) == "/home/kevin/.dotfile" );
      assert( simplifyPath( "c:\\home\\kevin\\.dotfile\\." ) == "c:\\home\\kevin\\.dotfile" );
      assert( simplifyPath( "c:\\home\\kevin\\.dotfile\\..\\..\\vickie\\.conf\\\\\\..\\." ) == "c:\\home\\vickie" );
      assert( simplifyPath( "c:\\home\\kevin\\.dotfile\\." ) == "c:\\home\\kevin\\.dotfile" );
      assert( simplifyPath( "c:/home/kevin/.dotfile/." ) == "c:/home/kevin/.dotfile" );
      assert( simplifyPath( "\\\\terra\\" ) == "\\\\terra" );
      assert( simplifyPath( "\\\\terra\\.\\hai\\.." ) == "\\\\terra" );
      assert( simplifyPath( "\\\\terra/./hai" ) == "\\\\terra\\hai" );
      assert( simplifyPath( "\\\\terra/./hai\\.." ) == "\\\\terra" );
      assert( simplifyPath( "\\\\terra\\video\\tv" ) == "\\\\terra\\video\\tv" );
   }

   bool isDir( const char* path )
   {
      DIR* d = opendir( path );
      if (d)
         closedir( d );
      return d != NULL;
   }

   bool isFile( const char* path )
   {
      return !isDir( path );
   }

   inline std::string join( char c, std::vector<std::string> strs )
   {
      std::string result;
      std::string cc = "/";
      cc[0] = c;
      for (unsigned int x = 0; x < strs.size(); ++x)
      {
         if (0 < result.size() && 1 <= x && result[result.size()-1] != c )
         {
            result += cc;
         }
         result += strs[x];
      }
      return result;
   }
   // need to NULL terminate the ... list
   inline std::string join( char c, ... )
   {
      const int max_strs = 20;
      char* strs_c[max_strs];
      std::vector<std::string> strs;
      va_list arg_ptr;
      va_start( arg_ptr, c );
      char* s = va_arg( arg_ptr, char* );
      int count = 0;
      while (s)
      {
         strs_c[count] = s;
         s = va_arg( arg_ptr, char* );
         ++count;
         if (max_strs < count)
         {
            assert( count < max_strs && "gio::join(...):  hmmmmm, did you null terminate?" );
         }
      }
      assert( count < max_strs && "gio::join(...):  forget to supply NULL?  more than max_strs arguments given?" );
      for (int x = 0; x < count; ++x)
      {
         strs.push_back( strs_c[x] );
      }
      va_end( arg_ptr );
      return join( c, strs );
   }

   inline std::string tolower( const std::string& t )
   {
      std::string s = t;
      std::transform(s.begin(), s.end(), s.begin(), ::tolower);

      /*
      // use the C++ locale support for this. You gain genericity
      std::transform(l.begin(), l.end(), l.begin(), std::bind2nd(std::ptr_fun(&std::tolower<char>), std::locale("")));
      std::transform(r.begin(), r.end(), r.begin(), std::bind2nd(std::ptr_fun(&std::tolower<char>), std::locale("")));
      // std::transform(l.begin(), l.end(), l.begin(), ::tolower);
      // std::transform(r.begin(), r.end(), r.begin(), ::tolower);
      */
      return s;
   }

   inline size_t computeFileSize( const char* filename )
   {
      FILE* f = ::fopen( filename, "rb" );
      if (f)
      {
         ::fseek( f, 0, SEEK_END );
         const int size = ::ftell( f );
         ::fclose( f );
         return size;
      }
      return 0;
   }

   // safest read whole file into a buffer, use ComputeFileSize before calling
   // adds \0 onto end of file, so bufsize needs to be +1
   inline char* readFile( char* buf, size_t bufsize, const char* filename, bool status = true )
   {
      //if (status) printf( "reading: %s... ", filename );
      FILE* f = ::fopen( filename, "r" );
      if (f)
      {
         ::fseek( f, 0, SEEK_END );
         const size_t filesize = ::ftell( f );
         ::fseek( f, 0, SEEK_SET );
         assert( (filesize+1) <= bufsize && "ReadFromFile: buffer too small" );
         const int num_read = ::fread( buf, 1, filesize, f );
         buf[filesize] = '\0';
         assert( num_read == filesize );
         ::fclose( f );
         //if (status) printf( "[%d bytes read]\n", num_read );
         return buf;
      }
      if (status) printf( "[fail]\n" );
      if (status) printf( "reading: %s... ", filename );
      if (1 <= bufsize) buf[0] = '\0'; else buf = NULL;
      return buf;
   }

   // load text file into string
   // WARNING: returns memory you have to delete!
   inline char* new_FromFile( const char* filename )
   {
      size_t size = computeFileSize( filename );
      if (0 < size)
      {
         char* bytes = new char[size+1];
         return readFile( bytes, size+1, filename );
      }
      return NULL;
   }

   bool writeFile( const char* filename, const char* buf, size_t size )
   {
      FILE* f = ::fopen( filename, "wb" );
      if (f)
      {
         ::fwrite( buf, 1, size, f );
         ::fclose( f );
         return true;
      }
      return false;
   }

   inline bool fileExists( const char* filename )
   {
      FILE* f = ::fopen( filename, "rb" );
      if (f)
      {
         ::fclose( f );
         return true;
      }
      return false;
   }

   // case sensative path, insensative filename, return actual filename found
   inline bool fileExistsi(std::string& filename,
                           const char *path, const char *ifilename)
   {
      DIR *dir; struct dirent *entry;
      const int path_size = 1024;
      char uifilename[path_size+1], ud_name[path_size+1];
      assert( strlen( ifilename ) <= path_size );

      if ((dir = opendir(path)) == NULL) {return false;}
      while ( (entry = readdir(dir)) )
      {
         for (size_t x = 0; x < strlen( ifilename )+1; ++x)
            uifilename[x] = toupper( ifilename[x] );
         for (size_t x = 0; x < strlen( entry->d_name )+1; ++x)
            ud_name[x] = toupper( entry->d_name[x] );
         if(strncmp( uifilename, ud_name, path_size ) == 0)
         {
            filename = entry->d_name;
            closedir(dir);
            return true;
         }
      }
      closedir(dir);
      return false;
   }

   inline void tokenize(std::vector<std::string>& tokens, const std::string& str, const std::string& delimiters /*= " "*/)
   {
      // Skip delimiters at beginning.
      std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
      // Find first "non-delimiter".
      std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

      while (std::string::npos != pos || std::string::npos != lastPos)
      {
         // Found a token, add it to the vector.
         tokens.push_back(str.substr(lastPos, pos - lastPos));
         // Skip delimiters.  Note the "not_of"
         lastPos = str.find_first_not_of(delimiters, pos);
         // Find next "non-delimiter"
         pos = str.find_first_of(delimiters, lastPos);
      }
   }

   inline std::vector<std::string> tokenize(const std::string& str, const std::string& delimiters /*= " "*/)
   {
      std::vector<std::string> tokens;
      tokenize( tokens, str, delimiters );
      return tokens;
   }

    // make everything in the path.  (don't specify the filename, just the path - use justPath() to filter)
    inline bool mkpath( const char* filename )
    {
      bool firstslash = filename[0] == '\\' || filename[0] == '/';
      std::vector<std::string> tokens;
      std::vector<std::string> dirs;
      tokenize( tokens, filename, "\\/" );

      dirs.resize( tokens.size() );
      for (unsigned int x = 0; x < tokens.size(); ++x)
      {
         for (unsigned int y = 0; y <= x; ++y)
         {
             if(firstslash && y==0)
                 dirs[x] += "/";

             dirs[x] += tokens[y];
             if (y < (x)) dirs[x] += "/";
         }
      }

      for (unsigned int x = 0; x < dirs.size(); ++x)
      {
// windows wanted to call them _stat and _mkdir... 
#ifdef WIN32 // and !defined(gcc) also?
#	define stat _stat
#	define mkdir(a,b) _mkdir(a);
#endif
         struct stat st;
         bool folderExists = (stat(dirs[x].c_str(), &st) == 0);
         if (!folderExists && (dirs[x][dirs[x].size() - 1] != ':')) // ignore c:, etc...
         {
            mkdir( dirs[x].c_str(), 0777 );
         }
#ifdef WIN32 // and !defined(gcc)?
#	undef stat
#	undef mkdir
#endif
      }
      return true;
    }

   // returns the last token in the a string
   inline char* strtokLast( char* string, const char* after )
   {
      // see what the extension is.
      char* ext = ::strtok( string, after );
      char* temp = ext;

      while (temp != NULL)
      {
         temp = ::strtok( NULL, after );
         if (temp != NULL)
            ext = temp;
      }

      return ext;
   }

   // returns the last token in the string, or returns string if there is only one token.
   // doesn't modify the source string
   inline const char* const getLastStringToken( const char* const string, char delimiter )
   {
      for (int x = ::strlen(string) - 1; x > 0; --x)
      {
         if (string[x] == delimiter)
         {
            return &string[x + 1];
         }
      }

      return string;
   }

   // returns just the .xxx part of a filename.
   // if there are no .'s in the name, then it returns a pointer to '\0' (the end)
   inline const char* justExt( const char* filename )
   {
      // see if there are no .'s in the name
      if (::strchr( filename, '.' ) == NULL)
         return &filename[ ::strlen(filename) - 1 ];

      // return the extension after the last .
      return getLastStringToken( filename, '.' );
   }

   std::string justPath( const char* const filepath )
   {
      int size = strlen( filepath );
      for (int x = (size - 1); 0 <= x; --x)
      {
         if (filepath[x] == '\\' || filepath[x] == '/')
         {
            std::string path = filepath;
            path.resize( x+1 );
            return path;
         }
      }
      return filepath;
   }

   std::string justFilename( const char* const filepath )
   {
      int size = strlen( filepath );
      for (int x = size-1; 0 <= x; --x)
      {
         if ((filepath[x] == '\\' || filepath[x] == '/') && x < size)
         {
            return &filepath[x+1];
         }
      }
      return filepath;
   }

   // fix the slashes.
   std::string fixupPath( const std::string& path )
    {
    #ifdef WIN32
       const char* slash = "\\";
    #else
       const char* slash = "/";
    #endif
       bool addFrontSlash = (path[0] == '\\' || path[0] == '/');
       std::string ret = gio::join( slash[0], gio::tokenize( path, "\\/" ) );
       if (addFrontSlash)
       {
          ret = std::string( slash ) + ret;
       }
       return ret;
    }
} // end namespace

#endif

