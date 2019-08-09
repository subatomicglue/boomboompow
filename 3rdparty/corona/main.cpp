
#include "corona.h"
#include <string>

int main()
{
   std::string s = "filename.tga";
   corona::Image* image = corona::OpenImage( s.c_str(), corona::FF_AUTODETECT, corona::PF_R8G8B8A8 );

   image->getPixels();
   return 0;
}
