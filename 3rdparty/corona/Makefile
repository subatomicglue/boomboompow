
OUTFILE= libcorona.a

SRCS= Convert.cpp           OpenBMP.cpp           OpenTGA.cpp \
		Corona.cpp            OpenGIF.cpp           SaveJPEG.cpp \
		Debug.cpp             OpenJPEG.cpp          SavePNG.cpp \
		DefaultFileSystem.cpp OpenPCX.cpp           SaveTGA.cpp \
		MemoryFile.cpp        OpenPNG.cpp \
		jpeg-6b/jcapimin.c jpeg-6b/jdmaster.c      libpng-1.2.1/pngrtran.c \
		jpeg-6b/jcapistd.c jpeg-6b/jdmerge.c       libpng-1.2.1/pngrutil.c \
		jpeg-6b/jccoefct.c jpeg-6b/jdphuff.c       libpng-1.2.1/pngset.c \
		jpeg-6b/jccolor.c  jpeg-6b/jdpostct.c      \
		jpeg-6b/jcdctmgr.c jpeg-6b/jdsample.c      libpng-1.2.1/pngtrans.c \
		jpeg-6b/jchuff.c   jpeg-6b/jdtrans.c       libpng-1.2.1/pngvcrd.c \
		jpeg-6b/jcinit.c   jpeg-6b/jerror.c        libpng-1.2.1/pngwio.c \
		jpeg-6b/jcmainct.c jpeg-6b/jfdctflt.c      libpng-1.2.1/pngwrite.c \
		jpeg-6b/jcmarker.c jpeg-6b/jfdctfst.c      libpng-1.2.1/pngwtran.c \
		jpeg-6b/jcmaster.c jpeg-6b/jfdctint.c      libpng-1.2.1/pngwutil.c \
		jpeg-6b/jcomapi.c  jpeg-6b/jidctflt.c      libungif-4.1.0/dgif_lib.c \
		jpeg-6b/jcparam.c  jpeg-6b/jidctfst.c      libungif-4.1.0/gif_err.c \
		jpeg-6b/jcphuff.c  jpeg-6b/jidctint.c      libungif-4.1.0/gifalloc.c \
		jpeg-6b/jcprepct.c jpeg-6b/jidctred.c      zlib-1.1.4/adler32.c \
		jpeg-6b/jcsample.c jpeg-6b/jmemmgr.c       zlib-1.1.4/compress.c \
		jpeg-6b/jctrans.c  jpeg-6b/jmemnobs.c      zlib-1.1.4/crc32.c \
		jpeg-6b/jdapimin.c jpeg-6b/jquant1.c       zlib-1.1.4/deflate.c \
		jpeg-6b/jdapistd.c jpeg-6b/jquant2.c       zlib-1.1.4/gzio.c \
		jpeg-6b/jdatadst.c jpeg-6b/jutils.c        zlib-1.1.4/infblock.c \
		jpeg-6b/jdatasrc.c libpng-1.2.1/png.c      zlib-1.1.4/infcodes.c \
		jpeg-6b/jdcoefct.c libpng-1.2.1/pngerror.c zlib-1.1.4/inffast.c \
		jpeg-6b/jdcolor.c  libpng-1.2.1/pnggccrd.c zlib-1.1.4/inflate.c \
		jpeg-6b/jddctmgr.c libpng-1.2.1/pngget.c   zlib-1.1.4/inftrees.c \
		jpeg-6b/jdhuff.c   libpng-1.2.1/pngmem.c   zlib-1.1.4/infutil.c \
		jpeg-6b/jdinput.c  libpng-1.2.1/pngpread.c zlib-1.1.4/trees.c \
		jpeg-6b/jdmainct.c libpng-1.2.1/pngread.c  zlib-1.1.4/uncompr.c \
		jpeg-6b/jdmarker.c libpng-1.2.1/pngrio.c   zlib-1.1.4/zutil.c

_suffix_list = c C CC cc cpp c++ cxx
makeobjs = $(filter %.o, $(foreach _suffix, $(_suffix_list), $(1:.$(_suffix)=.o)))
OBJS = $(call makeobjs, $(SRCS))

INCLUDES=-Ijpeg-6b -Ilibpng-1.2.1 -Ilibungif-4.1.0 -Izlib-1.1.4

all: $(OUTFILE)

UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
CC=gcc
CXX=g++
CFLAGS= -O2 -mno-thumb
#CFLAGS= -g -ggdb -mno-thumb
endif
ifeq ($(UNAME), Darwin)
CC=llvm-gcc
CXX=llvm-g++
CFLAGS= -O3 -arch x86_64
#CFLAGS= -arch x86_64 -g -ggdb
MAKEDEPENDINCLUDES=-I/usr/include -I/usr/include/c++/4.2.1/ -I/usr/X11/include -I/usr/include/c++/4.2.1/tr1
endif
CXXFLAGS=-std=c++0x $(CFLAGS)

$(OUTFILE): Makefile $(OBJS)
	ar -cvq $(OUTFILE)  $(OBJS)

# %.cpp needs to be first for $< to resolve to it
%.o: %.cpp
	$(CXX) $(DEFINES) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
%.o: %.c
	$(CC) $(DEFINES) $(CFLAGS) $(INCLUDES) -c $< -o $@

depend Makefile: $(SRCS)
	makedepend -- $(INCLUDES) $(MAKEDEPENDINCLUDES) -- $(SRCS)

clean:
	rm -f $(OBJS) $(OUTFILE)

clobber:
	rm -f $(OBJS) $(OUTFILE)


# DO NOT DELETE

Convert.o: corona.h Debug.h SimpleImage.h Types.h Utility.h
OpenBMP.o: corona.h SimpleImage.h Types.h Utility.h
OpenTGA.o: Debug.h Open.h corona.h SimpleImage.h Types.h Utility.h
Corona.o: corona.h MemoryFile.h Types.h Utility.h Open.h Save.h SimpleImage.h
OpenGIF.o: libungif-4.1.0/gif_lib.h Debug.h Open.h corona.h SimpleImage.h
OpenGIF.o: Types.h Utility.h
SaveJPEG.o: jpeg-6b/jpeglib.h jpeg-6b/jconfig.h jpeg-6b/jmorecfg.h
SaveJPEG.o: jpeg-6b/jerror.h Debug.h Save.h corona.h Utility.h Types.h
Debug.o: Debug.h
OpenJPEG.o: jpeg-6b/jpeglib.h jpeg-6b/jconfig.h jpeg-6b/jmorecfg.h Open.h
OpenJPEG.o: corona.h SimpleImage.h Types.h Utility.h
SavePNG.o: libpng-1.2.1/png.h zlib-1.1.4/zlib.h zlib-1.1.4/zconf.h
SavePNG.o: libpng-1.2.1/pngconf.h Debug.h Save.h corona.h Types.h
DefaultFileSystem.o: Utility.h corona.h Types.h
OpenPCX.o: Debug.h Open.h corona.h SimpleImage.h Types.h Utility.h
SaveTGA.o: libpng-1.2.1/png.h zlib-1.1.4/zlib.h zlib-1.1.4/zconf.h
SaveTGA.o: libpng-1.2.1/pngconf.h Debug.h Save.h corona.h Utility.h Types.h
MemoryFile.o: MemoryFile.h corona.h Types.h Utility.h
OpenPNG.o: libpng-1.2.1/png.h zlib-1.1.4/zlib.h zlib-1.1.4/zconf.h
OpenPNG.o: libpng-1.2.1/pngconf.h Debug.h Open.h corona.h SimpleImage.h
OpenPNG.o: Types.h Utility.h
jpeg-6b/jcapimin.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jcapimin.o: jpeg-6b/jmorecfg.h
jpeg-6b/jdmaster.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jdmaster.o: jpeg-6b/jmorecfg.h
libpng-1.2.1/pngrtran.o: libpng-1.2.1/png.h zlib-1.1.4/zlib.h
libpng-1.2.1/pngrtran.o: zlib-1.1.4/zconf.h libpng-1.2.1/pngconf.h
jpeg-6b/jcapistd.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jcapistd.o: jpeg-6b/jmorecfg.h
jpeg-6b/jdmerge.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jdmerge.o: jpeg-6b/jmorecfg.h
libpng-1.2.1/pngrutil.o: libpng-1.2.1/png.h zlib-1.1.4/zlib.h
libpng-1.2.1/pngrutil.o: zlib-1.1.4/zconf.h libpng-1.2.1/pngconf.h
jpeg-6b/jccoefct.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jccoefct.o: jpeg-6b/jmorecfg.h
jpeg-6b/jdphuff.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jdphuff.o: jpeg-6b/jmorecfg.h jpeg-6b/jdhuff.h
libpng-1.2.1/pngset.o: libpng-1.2.1/png.h zlib-1.1.4/zlib.h
libpng-1.2.1/pngset.o: zlib-1.1.4/zconf.h libpng-1.2.1/pngconf.h
jpeg-6b/jccolor.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jccolor.o: jpeg-6b/jmorecfg.h
jpeg-6b/jdpostct.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jdpostct.o: jpeg-6b/jmorecfg.h
jpeg-6b/jcdctmgr.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jcdctmgr.o: jpeg-6b/jmorecfg.h jpeg-6b/jdct.h
jpeg-6b/jdsample.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jdsample.o: jpeg-6b/jmorecfg.h
libpng-1.2.1/pngtrans.o: libpng-1.2.1/png.h zlib-1.1.4/zlib.h
libpng-1.2.1/pngtrans.o: zlib-1.1.4/zconf.h libpng-1.2.1/pngconf.h
jpeg-6b/jchuff.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jchuff.o: jpeg-6b/jmorecfg.h jpeg-6b/jchuff.h
jpeg-6b/jdtrans.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jdtrans.o: jpeg-6b/jmorecfg.h
libpng-1.2.1/pngvcrd.o: libpng-1.2.1/png.h zlib-1.1.4/zlib.h
libpng-1.2.1/pngvcrd.o: zlib-1.1.4/zconf.h libpng-1.2.1/pngconf.h
jpeg-6b/jcinit.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jcinit.o: jpeg-6b/jmorecfg.h
jpeg-6b/jerror.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jerror.o: jpeg-6b/jmorecfg.h jpeg-6b/jversion.h jpeg-6b/jerror.h
libpng-1.2.1/pngwio.o: libpng-1.2.1/png.h zlib-1.1.4/zlib.h
libpng-1.2.1/pngwio.o: zlib-1.1.4/zconf.h libpng-1.2.1/pngconf.h
jpeg-6b/jcmainct.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jcmainct.o: jpeg-6b/jmorecfg.h
jpeg-6b/jfdctflt.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jfdctflt.o: jpeg-6b/jmorecfg.h jpeg-6b/jdct.h
libpng-1.2.1/pngwrite.o: libpng-1.2.1/png.h zlib-1.1.4/zlib.h
libpng-1.2.1/pngwrite.o: zlib-1.1.4/zconf.h libpng-1.2.1/pngconf.h
jpeg-6b/jcmarker.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jcmarker.o: jpeg-6b/jmorecfg.h
jpeg-6b/jfdctfst.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jfdctfst.o: jpeg-6b/jmorecfg.h jpeg-6b/jdct.h
libpng-1.2.1/pngwtran.o: libpng-1.2.1/png.h zlib-1.1.4/zlib.h
libpng-1.2.1/pngwtran.o: zlib-1.1.4/zconf.h libpng-1.2.1/pngconf.h
jpeg-6b/jcmaster.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jcmaster.o: jpeg-6b/jmorecfg.h
jpeg-6b/jfdctint.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jfdctint.o: jpeg-6b/jmorecfg.h jpeg-6b/jdct.h
libpng-1.2.1/pngwutil.o: libpng-1.2.1/png.h zlib-1.1.4/zlib.h
libpng-1.2.1/pngwutil.o: zlib-1.1.4/zconf.h libpng-1.2.1/pngconf.h
jpeg-6b/jcomapi.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jcomapi.o: jpeg-6b/jmorecfg.h
jpeg-6b/jidctflt.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jidctflt.o: jpeg-6b/jmorecfg.h jpeg-6b/jdct.h
libungif-4.1.0/dgif_lib.o: libungif-4.1.0/gif_lib.h
libungif-4.1.0/dgif_lib.o: libungif-4.1.0/gif_lib_private.h
jpeg-6b/jcparam.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jcparam.o: jpeg-6b/jmorecfg.h
jpeg-6b/jidctfst.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jidctfst.o: jpeg-6b/jmorecfg.h jpeg-6b/jdct.h
libungif-4.1.0/gif_err.o: libungif-4.1.0/gif_lib.h
jpeg-6b/jcphuff.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jcphuff.o: jpeg-6b/jmorecfg.h jpeg-6b/jchuff.h
jpeg-6b/jidctint.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jidctint.o: jpeg-6b/jmorecfg.h jpeg-6b/jdct.h
libungif-4.1.0/gifalloc.o: libungif-4.1.0/gif_lib.h
jpeg-6b/jcprepct.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jcprepct.o: jpeg-6b/jmorecfg.h
jpeg-6b/jidctred.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jidctred.o: jpeg-6b/jmorecfg.h jpeg-6b/jdct.h
zlib-1.1.4/adler32.o: zlib-1.1.4/zlib.h zlib-1.1.4/zconf.h
jpeg-6b/jcsample.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jcsample.o: jpeg-6b/jmorecfg.h
jpeg-6b/jmemmgr.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jmemmgr.o: jpeg-6b/jmorecfg.h jpeg-6b/jmemsys.h
zlib-1.1.4/compress.o: zlib-1.1.4/zlib.h zlib-1.1.4/zconf.h
jpeg-6b/jctrans.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jctrans.o: jpeg-6b/jmorecfg.h
jpeg-6b/jmemnobs.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jmemnobs.o: jpeg-6b/jmorecfg.h jpeg-6b/jmemsys.h
zlib-1.1.4/crc32.o: zlib-1.1.4/zlib.h zlib-1.1.4/zconf.h
jpeg-6b/jdapimin.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jdapimin.o: jpeg-6b/jmorecfg.h
jpeg-6b/jquant1.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jquant1.o: jpeg-6b/jmorecfg.h
zlib-1.1.4/deflate.o: zlib-1.1.4/deflate.h zlib-1.1.4/zutil.h
zlib-1.1.4/deflate.o: zlib-1.1.4/zlib.h zlib-1.1.4/zconf.h
jpeg-6b/jdapistd.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jdapistd.o: jpeg-6b/jmorecfg.h
jpeg-6b/jquant2.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jquant2.o: jpeg-6b/jmorecfg.h
zlib-1.1.4/gzio.o: zlib-1.1.4/zutil.h zlib-1.1.4/zlib.h zlib-1.1.4/zconf.h
jpeg-6b/jdatadst.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jdatadst.o: jpeg-6b/jmorecfg.h jpeg-6b/jerror.h
jpeg-6b/jutils.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jutils.o: jpeg-6b/jmorecfg.h
zlib-1.1.4/infblock.o: zlib-1.1.4/zutil.h zlib-1.1.4/zlib.h
zlib-1.1.4/infblock.o: zlib-1.1.4/zconf.h zlib-1.1.4/infblock.h
zlib-1.1.4/infblock.o: zlib-1.1.4/inftrees.h zlib-1.1.4/infcodes.h
zlib-1.1.4/infblock.o: zlib-1.1.4/infutil.h
jpeg-6b/jdatasrc.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jdatasrc.o: jpeg-6b/jmorecfg.h jpeg-6b/jerror.h
libpng-1.2.1/png.o: libpng-1.2.1/png.h zlib-1.1.4/zlib.h zlib-1.1.4/zconf.h
libpng-1.2.1/png.o: libpng-1.2.1/pngconf.h
zlib-1.1.4/infcodes.o: zlib-1.1.4/zutil.h zlib-1.1.4/zlib.h
zlib-1.1.4/infcodes.o: zlib-1.1.4/zconf.h zlib-1.1.4/inftrees.h
zlib-1.1.4/infcodes.o: zlib-1.1.4/infblock.h zlib-1.1.4/infcodes.h
zlib-1.1.4/infcodes.o: zlib-1.1.4/infutil.h zlib-1.1.4/inffast.h
jpeg-6b/jdcoefct.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jdcoefct.o: jpeg-6b/jmorecfg.h
libpng-1.2.1/pngerror.o: libpng-1.2.1/png.h zlib-1.1.4/zlib.h
libpng-1.2.1/pngerror.o: zlib-1.1.4/zconf.h libpng-1.2.1/pngconf.h
zlib-1.1.4/inffast.o: zlib-1.1.4/zutil.h zlib-1.1.4/zlib.h zlib-1.1.4/zconf.h
zlib-1.1.4/inffast.o: zlib-1.1.4/inftrees.h zlib-1.1.4/infblock.h
zlib-1.1.4/inffast.o: zlib-1.1.4/infcodes.h zlib-1.1.4/infutil.h
zlib-1.1.4/inffast.o: zlib-1.1.4/inffast.h
jpeg-6b/jdcolor.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jdcolor.o: jpeg-6b/jmorecfg.h
libpng-1.2.1/pnggccrd.o: libpng-1.2.1/png.h zlib-1.1.4/zlib.h
libpng-1.2.1/pnggccrd.o: zlib-1.1.4/zconf.h libpng-1.2.1/pngconf.h
zlib-1.1.4/inflate.o: zlib-1.1.4/zutil.h zlib-1.1.4/zlib.h zlib-1.1.4/zconf.h
zlib-1.1.4/inflate.o: zlib-1.1.4/infblock.h
jpeg-6b/jddctmgr.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jddctmgr.o: jpeg-6b/jmorecfg.h jpeg-6b/jdct.h
libpng-1.2.1/pngget.o: libpng-1.2.1/png.h zlib-1.1.4/zlib.h
libpng-1.2.1/pngget.o: zlib-1.1.4/zconf.h libpng-1.2.1/pngconf.h
zlib-1.1.4/inftrees.o: zlib-1.1.4/zutil.h zlib-1.1.4/zlib.h
zlib-1.1.4/inftrees.o: zlib-1.1.4/zconf.h zlib-1.1.4/inftrees.h
zlib-1.1.4/inftrees.o: zlib-1.1.4/inffixed.h
jpeg-6b/jdhuff.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jdhuff.o: jpeg-6b/jmorecfg.h jpeg-6b/jdhuff.h
libpng-1.2.1/pngmem.o: libpng-1.2.1/png.h zlib-1.1.4/zlib.h
libpng-1.2.1/pngmem.o: zlib-1.1.4/zconf.h libpng-1.2.1/pngconf.h
zlib-1.1.4/infutil.o: zlib-1.1.4/zutil.h zlib-1.1.4/zlib.h zlib-1.1.4/zconf.h
zlib-1.1.4/infutil.o: zlib-1.1.4/infblock.h zlib-1.1.4/inftrees.h
zlib-1.1.4/infutil.o: zlib-1.1.4/infcodes.h zlib-1.1.4/infutil.h
jpeg-6b/jdinput.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jdinput.o: jpeg-6b/jmorecfg.h
libpng-1.2.1/pngpread.o: libpng-1.2.1/png.h zlib-1.1.4/zlib.h
libpng-1.2.1/pngpread.o: zlib-1.1.4/zconf.h libpng-1.2.1/pngconf.h
zlib-1.1.4/trees.o: zlib-1.1.4/deflate.h zlib-1.1.4/zutil.h zlib-1.1.4/zlib.h
zlib-1.1.4/trees.o: zlib-1.1.4/zconf.h zlib-1.1.4/trees.h
jpeg-6b/jdmainct.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jdmainct.o: jpeg-6b/jmorecfg.h
libpng-1.2.1/pngread.o: libpng-1.2.1/png.h zlib-1.1.4/zlib.h
libpng-1.2.1/pngread.o: zlib-1.1.4/zconf.h libpng-1.2.1/pngconf.h
zlib-1.1.4/uncompr.o: zlib-1.1.4/zlib.h zlib-1.1.4/zconf.h
jpeg-6b/jdmarker.o: jpeg-6b/jinclude.h jpeg-6b/jconfig.h jpeg-6b/jpeglib.h
jpeg-6b/jdmarker.o: jpeg-6b/jmorecfg.h
libpng-1.2.1/pngrio.o: libpng-1.2.1/png.h zlib-1.1.4/zlib.h
libpng-1.2.1/pngrio.o: zlib-1.1.4/zconf.h libpng-1.2.1/pngconf.h
zlib-1.1.4/zutil.o: zlib-1.1.4/zutil.h zlib-1.1.4/zlib.h zlib-1.1.4/zconf.h
