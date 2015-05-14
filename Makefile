CC = g++
CFLAGS = -c -g -O3 -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -Wno-deprecated -Wno-write-strings
INCL = -I./ -I/usr/X11R6/include
LIBS = -lX11 -lXt -L/usr/X11R6/lib -lSM -lICE
TARGET = see
all: $(TARGET) strip clean

FILES := $(wildcard *.cpp)
OBJS = $(FILES:.cpp=.o) gifsave.o

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LIBS)

%.o: %.cpp *.hpp gifsave.h
	$(CC) $(CFLAGS) $(INCL) $< -o $@

%.o: %.c gifsave.h
	$(CC) $(CFLAGS) $< -o $@

strip:
	strip $(TARGET)

clean:
	rm -rf $(OBJS) *~
