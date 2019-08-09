#include <iostream>
#include "pixmi/SgiImporter.h"
#include "pixmi/TgaImporter.h"
#include "pixmi/PcxImporter.h"
#include "pixmi/BmpImporter.h"
#include "pixmi/PngImporter.h"
#include "pixmi/CFileIO.h"


//////////////////////////////
// Output image statistics.
//////////////////////////////
inline void outputImageProperties(const pixmi::Image &image)
{
    std::cout<<"Image Properties:\n\n";
    std::cout<<"Name            - "<<image.name()<<"\n";
    std::cout<<"Color Depth     - "<<image.bpp()<<"-bit color\n";
    std::cout<<"Size            - "<<image.width()<<" x "<<image.height()<<"\n";
    std::cout<<"Channels        - "<<image.channels()<<" color channel(s).\n";
    std::cout<<"BPC             - "<<image.bpc()<<" byte(s) per channel\n";
    std::cout<<"Memory Alignent - "<<image.rowAlignment()<<"-byte boundary addressing\n"<<std::flush;
}

int main(int argc, char* argv[])
{
if (argc < 3)
{ 
   std::cout<<"Not enough args: iproperties ext filename.ext\n"<<std::flush;
   exit(1);
   return 0;
}

pixmi::Image image;

std::string ext = argv[1];
std::string name = argv[2];

bool result = false;

if (ext == "sgi")
{
pixmi::SgiImporter i;
result = i.load( name.c_str(), image );
}
else if (ext == "tga")
{
pixmi::TgaImporter i;
result = i.load( name.c_str(), image );
}
else if (ext == "pcx")
{
pixmi::PcxImporter i;
result = i.load( name.c_str(), image );
}
else if (ext == "bmp")
{
pixmi::BmpImporter i;
result = i.load( name.c_str(), image );
}
else if (ext == "png")
{
pixmi::PngImporter i;
result = i.load( name.c_str(), image );
}

if (result == false)
{
   std::cout<<"iproperties: Couldn't load image\n"<<std::flush;
   exit(1);
}
outputImageProperties( image );

}
