CC=gcc
CFLAGS=-c -O3
LDFLAGS=-lpng16 -lcrypto
LDLIBS=-I/opt/local/include/libpng16 -I/opt/local/include -L/opt/local/lib
EXECUTABLE=stegano
SOURCES=stegano.c AES_util.c png_util.c Error.c
OBJECTS=$(SOURCES:.c=.o)
VPATH=src

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(LDLIBS) -o $(EXECUTABLE) $(OBJECTS)

.c.o:
	$(CC) $(CFLAGS) $(LDLIBS) $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
