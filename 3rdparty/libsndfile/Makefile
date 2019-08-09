
OUTFILE= libsndfile.a

SRCS= G72x/g721.c     GSM610/gsm_decode.c   GSM610/lpc.c \
G72x/g723_16.c  GSM610/add.c         GSM610/gsm_destroy.c  GSM610/preprocess.c \
G72x/g723_24.c  GSM610/code.c        GSM610/gsm_encode.c   GSM610/rpe.c \
G72x/g723_40.c  GSM610/decode.c      GSM610/gsm_option.c   GSM610/short_term.c \
G72x/g72x.c     GSM610/gsm_create.c  GSM610/long_term.c    GSM610/table.c \
aiff.c     double64.c   interleave.c  ogg.c  sndfile.c          voc.c \
alaw.c     dwd.c        ircam.c       paf.c  strings.c          vox_adpcm.c \
au.c       dwvw.c       macbinary3.c  pcm.c  svx.c              w64.c \
au_g72x.c  file_io.c    macos.c       pvf.c  wav.c \
avr.c      float32.c    mat4.c        raw.c  wav_w64.c \
command.c  gsm610.c     mat5.c        rx2.c  wve.c \
common.c   htk.c        ms_adpcm.c    sd2.c  txw.c              xi.c \
dither.c   ima_adpcm.c  nist.c        sds.c  ulaw.c

OBJS=$(SRCS:.c=.o)

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
CFLAGS= -O3 -arch x86_64 -mno-thumb
#CFLAGS= -arch x86_64 -g -ggdb -mno-thumb
endif
CXXFLAGS=$(CFLAGS)

$(OUTFILE): $(OBJS)
	ar -cvq $(OUTFILE)  $(OBJS)

.c.o:
	$(CC) $(CFLAGS) -DUSE_BSD -U__MACH__ -U__APPLE__ -Umacintosh -U__MWERKS__ -DHAVE_UNISTD_H -I. -c $< -o $@

clean:
	rm -f $(OBJS)

clobber:
	rm -f $(OBJS) $(OUTFILE)

