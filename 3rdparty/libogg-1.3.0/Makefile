
OUTFILE= libogg.a

SRCS= src/bitwise.c  src/framing.c

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
	$(CC) $(CFLAGS) -DHAVE_UNISTD_H -I. -Iinclude -c $< -o $@

clean:
	rm -f $(OBJS)

clobber:
	rm -f $(OBJS) $(OUTFILE)

