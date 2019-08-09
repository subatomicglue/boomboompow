
UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
LIBS = -lasound
endif
ifeq ($(UNAME), Darwin)
LIBS = -framework CoreAudio -framework CoreServices -framework AudioToolbox -framework AudioUnit -framework Carbon
endif

SRCS=   patest_read_record.c \
patest_ringmix.c \
patest_buffer.c           patest_sine8.c \
patest_callbackstop.c     patest_sine_channelmaps.c \
patest_clip.c             patest_sine_formats.c \
patest_converters.c       patest_sine_srate.c \
patest_dither.c           patest_sine_time.c \
patest_start_stop.c \
patest_hang.c             patest_stop.c \
patest_in_overflow.c      patest_stop_playout.c \
     patest_suggested_vs_streaminfo_latency.c \
patest_latency.c          patest_sync.c \
patest_leftright.c        patest_timing.c \
patest_longsine.c         patest_toomanysines.c \
patest_many.c             patest_two_rates.c \
patest_maxsines.c         patest_underflow.c \
patest_mono.c             patest_wire.c \
patest_multi_sine.c       \
patest_out_underflow.c    \
patest_prime.c            patest_write_stop.c

OBJS=$(SRCS:.c=.o)
EXES=$(SRCS:.c=.exe)

%.o: %.c
	gcc  -I ../src/common -I ../include  -c $< -o $@

%.exe: %.o
	gcc -I../src/common -I../include $< -o $@ ../lib/.libs/libportaudio.a -pthread $(LIBS)

all: $(EXES)
	-echo done

clean:
	rm $(EXES) $(OBJS)

