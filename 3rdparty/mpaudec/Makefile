
OUTFILE= libmpaudec.a

SRCS= bits.c  mpaudec.c

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
	$(CC) $(CFLAGS) -DHAVE_UNISTD_H -I. -c $< -o $@

clean:
	rm -f $(OBJS)

clobber:
	rm -f $(OBJS) $(OUTFILE)

