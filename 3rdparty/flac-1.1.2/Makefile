OUTFILE= libflac.a

SRCS= src/flac/analyze.c \
src/flac/decode.c \
src/flac/encode.c \
src/flac/local_string_utils.c \
src/flac/main.c \
src/flac/utils.c \
src/flac/vorbiscomment.c \
src/libFLAC/bitbuffer.c \
src/libFLAC/bitmath.c \
src/libFLAC/cpu.c \
src/libFLAC/crc.c \
src/libFLAC/file_decoder.c \
src/libFLAC/file_encoder.c \
src/libFLAC/fixed.c \
src/libFLAC/float.c \
src/libFLAC/format.c \
src/libFLAC/lpc.c \
src/libFLAC/md5.c \
src/libFLAC/memory.c \
src/libFLAC/metadata_iterators.c \
src/libFLAC/metadata_object.c \
src/libFLAC/seekable_stream_decoder.c \
src/libFLAC/seekable_stream_encoder.c \
src/libFLAC/stream_decoder.c \
src/libFLAC/stream_encoder.c \
src/libFLAC/stream_encoder_framing.c


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
	$(CC) ${CFLAGS} -DVERSION="\"KevinCompiledThis\"" -DHAVE_UNISTD_H -I. -Isrc -Isrc/flac -Isrc/libFLAC/include -Iinclude -c $< -o $@

clean:
	rm -f $(OBJS)

clobber:
	rm -f $(OBJS) $(OUTFILE)


