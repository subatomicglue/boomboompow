
APPNAME=BoomBoomPow
# when srcs come from ../ need to use nested .build/dir/ for each new level
BUILDDIR=.build/
EXEEXT=.exe
OBJEXT=.o
OUTFILE=$(APPNAME)$(EXEEXT)

# if wrong path given here, you'll get "No rule to make target `.depend'"
SRCS= main.cpp ${3RDPARTY}md5/md5.c k-grafix/BuiltInText.cpp k-audio/WavAudioIStream.cpp k-grafix/triangulate.cpp

#		nvwa-0.9/nvwa/bool_array.cpp \
		nvwa-0.9/nvwa/debug_new.cpp  \
		nvwa-0.9/nvwa/mem_pool_base.cpp  \
		nvwa-0.9/nvwa/static_mem_pool.cpp

all: $(OUTFILE) showinfo.exe

3RDPARTY=3rdparty/
DEFINES=-DUSE_BSD -DUSE_SDL -DEXCLUDE_GLEW
INCLUDES=-I. -I${3RDPARTY} -Ik-core -Ik-grafix -Ik-audio -Ik-game \
		-I${3RDPARTY}libsndfile \
		-I${3RDPARTY}mpaudec \
		-I${3RDPARTY}flac-1.1.2/src/libFLAC \
		-I${3RDPARTY}flac-1.1.2/src/flac \
		-I${3RDPARTY}libogg-1.3.0/include \
		-I${3RDPARTY}libvorbis-1.3.3/include \
		-I${3RDPARTY}libsamplerate-0.1.8/src \
		-I${3RDPARTY}vectormath/scalar/cpp \
		-I${3RDPARTY}eastl/include \
		-I${3RDPARTY}pixmi \
		-I${3RDPARTY}nvwa-0.9
INCLUDES += -I${3RDPARTY}SDL2-2.0.1/installed/include/SDL2

_suffix_list = c C CC cc cpp c++ cxx
makeobjs = $(addprefix $(BUILDDIR), $(filter %${OBJEXT}, $(foreach _suffix, $(_suffix_list), $(1:.$(_suffix)=${OBJEXT}))))
OBJS = $(call makeobjs, $(SRCS))
DEPS := $(OBJS:${OBJEXT}=.d)

UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
CC=gcc
CXX=g++
MD=gccmakedep
CFLAGS= -O2 -mno-thumb
#CFLAGS= -g -ggdb -mno-thumb -DDEBUG -D_DEBUG
LIBS = -lasound
MAKEDEPENDINCLUDES=-I/usr/include -I/usr/include/c++/4.2.1/ -I/usr/X11/include -I/usr/include/c++/4.2.1/tr1 -I/opt/X11/include
endif
ifeq ($(UNAME), Darwin)
CC=llvm-gcc
CXX=llvm-g++
MD=makedepend
DEFINES+=-DDEBUG -D_DEBUG
#CFLAGS= -O3 -g3 -arch x86_64 -mno-thumb
CFLAGS= -arch x86_64 -g -ggdb -Wno-deprecated-register
GR_LIBS= -framework OpenGL -L/usr/X11/lib -L/usr/X11R6/lib -L/opt/X11/lib -lXrandr -lXxf86vm -lXi -lXext -lX11 -lm
#INCLUDES+=-I${3RDPARTY}glew-1.10.0/include
SDL_FRAMEWORKS = -framework CoreAudio \
				 -framework CoreServices \
				 -framework AudioToolbox \
				 -framework AudioUnit \
				 -framework Carbon \
				 -framework ForceFeedback \
				 -framework IOKit \
				 -framework Cocoa \
				 -liconv
LIBS = $(SDL_FRAMEWORKS) $(GR_LIBS)
MAKEDEPENDINCLUDES=-I/usr/include -I/usr/include/c++/4.2.1/ -I/usr/X11/include -I/usr/include/c++/4.2.1/tr1
endif
CXXFLAGS= -std=c++0x $(CFLAGS)

BUILD_THESE_LIBS = ${3RDPARTY}libsndfile/libsndfile.a \
		${3RDPARTY}mpaudec/libmpaudec.a  \
		${3RDPARTY}flac-1.1.2/libflac.a \
		${3RDPARTY}libsamplerate-0.1.8/src/.libs/libsamplerate.a \
		${3RDPARTY}libvorbis-1.3.3/libvorbis.a \
		${3RDPARTY}libogg-1.3.0/libogg.a \
		${3RDPARTY}eastl/libeastl.a \
		${3RDPARTY}corona/libcorona.a
#		pixmi/libpixmi.a
BUILD_THESE_LIBS += ${3RDPARTY}SDL2-2.0.1/installed/lib/libSDL2.a \
							${3RDPARTY}SDL2-2.0.1/installed/lib/libSDL2main.a

###############################
# audio interface:
INCLUDES += -DUSE_SDLAUDIO

#INCLUDES += -DUSE_PORTAUDIO -Ilibportaudio/include
#BUILD_THESE_LIBS += libportaudio/lib/.libs/libportaudio.a
###############################

${3RDPARTY}pixmi/libpixmi.a:
	cd ${3RDPARTY}pixmi && $(MAKE)

${3RDPARTY}eastl/libeastl.a:
	cd ${3RDPARTY}eastl && $(MAKE)

${3RDPARTY}SDL2-2.0.1/Makefile: ${3RDPARTY}SDL2-2.0.1/Makefile.in
	cd ${3RDPARTY}SDL2-2.0.1 && ./config-kevin

${3RDPARTY}SDL2-2.0.1/installed/lib/libSDL2.a: ${3RDPARTY}SDL2-2.0.1/Makefile
	cd ${3RDPARTY}SDL2-2.0.1/ && $(MAKE) && $(MAKE) install

${3RDPARTY}glew-1.10.0/lib/libglew.a:
	cd ${3RDPARTY}glew-1.10.0 && $(MAKE)

${3RDPARTY}corona/libcorona.a:
	cd ${3RDPARTY}corona && $(MAKE)

${3RDPARTY}libsamplerate-0.1.8/Makefile:
	cd ${3RDPARTY}libsamplerate-0.1.8/ && ./configure-kevin

${3RDPARTY}libsamplerate-0.1.8/src/.libs/libsamplerate.a: ${3RDPARTY}libsamplerate-0.1.8/Makefile
	cd ${3RDPARTY}libsamplerate-0.1.8 && $(MAKE) CC=$(CC) CXX=$(CXX) CFLAGS="${CFLAGS}" CXXFLAGS="$(CXXFLAGS)"

${3RDPARTY}libportaudio/Makefile:
	cd ${3RDPARTY}libportaudio/ && ./configure-kevin

${3RDPARTY}libportaudio/bindings/cpp/Makefile:
	cd ${3RDPARTY}libportaudio/bindings/cpp && ./configure

${3RDPARTY}libportaudio/lib/.libs/libportaudio.a: ${3RDPARTY}libportaudio/Makefile
	cd ${3RDPARTY}libportaudio && $(MAKE)

${3RDPARTY}flac-1.1.2/libflac.a:
	cd ${3RDPARTY}flac-1.1.2 && $(MAKE) CC=$(CC) CXX=$(CXX) CFLAGS="${CFLAGS}" CXXFLAGS="$(CXXFLAGS)"

${3RDPARTY}libsndfile/libsndfile.a:
	cd ${3RDPARTY}libsndfile && $(MAKE) CC=$(CC) CXX=$(CXX) CFLAGS="${CFLAGS}" CXXFLAGS="$(CXXFLAGS)"

${3RDPARTY}mpaudec/libmpaudec.a:
	cd ${3RDPARTY}mpaudec && $(MAKE) CC=$(CC) CXX=$(CXX) CFLAGS="${CFLAGS}" CXXFLAGS="$(CXXFLAGS)"

${3RDPARTY}libogg-1.3.0/libogg.a:
	cd ${3RDPARTY}libogg-1.3.0 && $(MAKE) CC=$(CC) CXX=$(CXX) CFLAGS="${CFLAGS}" CXXFLAGS="$(CXXFLAGS)"

${3RDPARTY}libvorbis-1.3.3/libvorbis.a:
	cd ${3RDPARTY}libvorbis-1.3.3 && $(MAKE) CC=$(CC) CXX=$(CXX) CFLAGS="${CFLAGS}" CXXFLAGS="$(CXXFLAGS)"

$(OUTFILE): $(DEPS) Makefile $(BUILD_THESE_LIBS)  $(BUILD_THESE_GR_LIBS) $(OBJS)
	@echo "-=[Creating $@]=-"
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(OUTFILE) $(BUILD_THESE_LIBS) -lm $(LIBS)
	$(MAKE) appbundle

showinfo.exe: $(DEPS) Makefile $(BUILD_THESE_LIBS) $(BUILDDIR)showinfo.o
	@echo "-=[Creating $@]=-"
	$(CXX) $(CXXFLAGS) $(BUILDDIR)showinfo.o -o showinfo.exe $(BUILD_THESE_LIBS) -lm $(LIBS)



APPBUNDLE=$(APPNAME).app
APPBUNDLECONTENTS=$(APPBUNDLE)/Contents
APPBUNDLEEXE=$(APPBUNDLECONTENTS)/MacOS
APPBUNDLERESOURCES=$(APPBUNDLECONTENTS)/Resources
APPBUNDLEICON=$(APPBUNDLECONTENTS)/Resources
appbundle: macosx/$(APPNAME).icns
	@echo "-=[Creating $(APPBUNDLE)]=-"
	rm -rf $(APPBUNDLE)
	mkdir $(APPBUNDLE)
	mkdir $(APPBUNDLE)/Contents
	mkdir $(APPBUNDLE)/Contents/MacOS
	mkdir $(APPBUNDLE)/Contents/Resources
	cp macosx/Info.plist $(APPBUNDLECONTENTS)/
	cp macosx/PkgInfo $(APPBUNDLECONTENTS)/
	cp macosx/$(APPNAME).icns $(APPBUNDLEICON)/
	cp $(OUTFILE) $(APPBUNDLEEXE)/$(APPNAME)
	cp -r kid $(APPBUNDLERESOURCES)/
	cp -r kid2 $(APPBUNDLERESOURCES)/
	cp -r art $(APPBUNDLERESOURCES)/
	cp -r pro $(APPBUNDLERESOURCES)/
	cp -r testwav $(APPBUNDLERESOURCES)/

macosx/$(APPNAME).icns: macosx/$(APPNAME)Icon.png
	@echo "-=[Creating $@]=-"
	rm -rf macosx/$(APPNAME).iconset
	mkdir macosx/$(APPNAME).iconset
	sips -z 16 16     macosx/$(APPNAME)Icon.png --out macosx/$(APPNAME).iconset/icon_16x16.png
	sips -z 32 32     macosx/$(APPNAME)Icon.png --out macosx/$(APPNAME).iconset/icon_16x16@2x.png
	sips -z 32 32     macosx/$(APPNAME)Icon.png --out macosx/$(APPNAME).iconset/icon_32x32.png
	sips -z 64 64     macosx/$(APPNAME)Icon.png --out macosx/$(APPNAME).iconset/icon_32x32@2x.png
	sips -z 128 128   macosx/$(APPNAME)Icon.png --out macosx/$(APPNAME).iconset/icon_128x128.png
	sips -z 256 256   macosx/$(APPNAME)Icon.png --out macosx/$(APPNAME).iconset/icon_128x128@2x.png
	sips -z 256 256   macosx/$(APPNAME)Icon.png --out macosx/$(APPNAME).iconset/icon_256x256.png
	sips -z 512 512   macosx/$(APPNAME)Icon.png --out macosx/$(APPNAME).iconset/icon_256x256@2x.png
	sips -z 512 512   macosx/$(APPNAME)Icon.png --out macosx/$(APPNAME).iconset/icon_512x512.png
	cp macosx/$(APPNAME)Icon.png macosx/$(APPNAME).iconset/icon_512x512@2x.png
	iconutil -c icns -o macosx/$(APPNAME).icns macosx/$(APPNAME).iconset
	rm -r macosx/$(APPNAME).iconset

clean:
	rm -f $(OBJS) $(BUILDDIR)/showinfo.o
	rm -f $(OUTFILE)
	rm -rf $(APPNAME).app
	rm -f showinfo.exe

cleandepend:
	rm -f .depend
	rm -f $(DEPS)

clobber:
	$(MAKE) cleandepend
	$(MAKE) clean
	rm -rf $(BUILDDIR)
	-cd ${3RDPARTY}corona && $(MAKE) clobber
	-cd ${3RDPARTY}pixmi && $(MAKE) clobber
	-cd ${3RDPARTY}eastl && $(MAKE) clobber
	-cd ${3RDPARTY}flac-1.1.2 && $(MAKE) clobber
	-cd ${3RDPARTY}libsndfile && $(MAKE) clobber
	-cd ${3RDPARTY}mpaudec && $(MAKE) clobber
	-cd ${3RDPARTY}SDL2-2.0.1 && $(MAKE) uninstall; $(MAKE) clean; $(MAKE) distclean
	-cd ${3RDPARTY}libogg-1.3.0 && $(MAKE) clobber
	-cd ${3RDPARTY}libvorbis-1.3.3 && $(MAKE) clobber
	-cd ${3RDPARTY}libportaudio && $(MAKE) clean && rm -f Makefile && cd test && $(MAKE) clean
	-cd ${3RDPARTY}libportaudio/bindings/cpp && $(MAKE) clean && rm -f Makefile
	-cd ${3RDPARTY}libsamplerate-0.1.8 && $(MAKE) clean && rm -f Makefile
	-cd ${3RDPARTY}glew-1.10.0 && $(MAKE) clean
	-rm -rf iOS/Build
	-rm ${3RDPARTY}nvwa-0.9/nvwa/*.o
	-rm ${3RDPARTY}md5/*.o



#############################
# Generic rules
#############################

# the | means that the prerequisites that follow (in this case $(BUILDDIR)) are order-only prerequisites. This means that if any $(objects) must be built then obj must be built first, but if obj is out of date (or doesn't exist), that does not force $(objects) to be built.
${OBJS}: | $(BUILDDIR)
${DEPS}: | $(BUILDDIR)

$(BUILDDIR):
	@echo "\n-=[Making the $(BUILDDIR) dir]=-\n"
	-@mkdir -p $@

# %.cpp needs to be first for $< to resolve to it
$(BUILDDIR)%${OBJEXT}: %.cpp
	@echo "-=[Creating $@]=-"
	-@mkdir -p `dirname $@`
	$(CXX) $(DEFINES) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
$(BUILDDIR)%${OBJEXT}: %.c
	@echo "-=[Creating $@]=-"
	-@mkdir -p `dirname $@`
	$(CC) $(DEFINES) $(CFLAGS) $(INCLUDES) -c $< -o $@

# make this target, to generate .d files.
depend: .depend $(DEPS)
	@echo "ok!"

# to use one file for depends, put ".depend" as a dependency of your output file target
# then add -include .depend to the makefile
.depend: $(SRCS) Makefile | $(BUILDDIR)
	@rm -f .depend && touch .depend
	@echo "-=[Creating Dependencies]=-\n   $(SRCS)\n"
	$(MD) -p$(BUILDDIR) -f.depend -- $(DEFINES) $(INCLUDES) $(MAKEDEPENDINCLUDES) -- $(SRCS)
	@rm -f .depend.bak
	@echo "\n-=[" `wc -l < .depend` "lines of Makefile code generated inside '.depend'! Woohoo....]=-\n"

# to use individual depend files, put $(DEPS) as a dependency of your output file target
# then add -include $(DEPS) to the makefile
$(BUILDDIR)%.d: %.cpp
	@echo "-=[writing $@]=-"
	@mkdir -p `dirname $@`
#	rm -f $@ && touch $@
#	$(MD) -p$(BUILDDIR) -f$@ -- $(DEFINES) $(INCLUDES) $(MAKEDEPENDINCLUDES) -- $<
	g++ -MF"$@" -MG -MM -MP -MT"$@" -MT"$(BUILDDIR)$(<:.cpp=${OBJEXT})" "$<" $(DEFINES) $(INCLUDES) $(MAKEDEPENDINCLUDES)
#	@rm -f $@.bak
	@echo "\n-=[" `wc -l < $@` "lines of Makefile code generated inside '"$@"'! Woohoo....]=-\n"
$(BUILDDIR)%.d: %.c
	@echo "-=[writing $@]=-"
	@mkdir -p `dirname $@`
#	@rm -f $@ && touch $@
#	$(MD) -p$(BUILDDIR) -f$@ -- $(DEFINES) $(INCLUDES) $(MAKEDEPENDINCLUDES) -- $<
	gcc -MF"$@" -MG -MM -MP -MT"$@" -MT"$(BUILDDIR)$(<:.c=${OBJEXT})" "$<" $(DEFINES) $(INCLUDES) $(MAKEDEPENDINCLUDES)
#	@rm -f $@.bak
	@echo "\n-=[" `wc -l < $@` "lines of Makefile code generated inside '"$@"'! Woohoo....]=-\n"

ifneq ($(MAKECMDGOALS),cleandepend)
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),clobber)
#-include .depend
-include $(DEPS)
endif
endif
endif

