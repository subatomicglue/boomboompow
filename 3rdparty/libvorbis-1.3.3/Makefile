OUTFILE= libvorbis.a

SRCS= lib/lookup.c      lib/vorbisfile.c \
lib/lpc.c         lib/window.c lib/lsp.c         \
lib/mapping0.c    lib/mdct.c        \
lib/analysis.c    lib/psy.c         \
lib/barkmel.c          \
lib/bitrate.c     lib/registry.c    \
lib/block.c       lib/res0.c        \
lib/codebook.c    lib/sharedbook.c  \
lib/envelope.c    lib/smallft.c     \
lib/floor0.c      lib/synthesis.c   \
lib/floor1.c      lib/tone.c        \
lib/info.c        lib/vorbisenc.c

#lib/psytune.c

SRCVQ= vq/vqgen.c vq/metrics.c vq/bookutil.c vq/distribution.c vq/huffbuild.c vq/latticebuild.c vq/latticetune.c

OBJS=$(SRCS:.c=.o)

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
CFLAGS= -O3 -arch x86_64 -mno-thumb
#CFLAGS= -arch x86_64 -g -ggdb -mno-thumb
endif
CXXFLAGS=$(CFLAGS)

all: $(OUTFILE)

$(OUTFILE): $(OBJS)
	ar -cvq $(OUTFILE)  $(OBJS)

.c.o:
	$(CC) $(CFLAGS) -DHAVE_UNISTD_H -Ilib -I../libogg-1.3.0/include -I. -Iinclude -c $< -o $@

clean:
	rm -f $(OBJS)

clobber:
	rm -f $(OBJS) $(OUTFILE)

