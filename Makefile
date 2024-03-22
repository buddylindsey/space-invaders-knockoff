CC=gcc
CFLAGS=-Wall $(shell sdl2-config --cflags)
LIBS=$(shell sdl2-config --libs)

TARGET=space_invaders
SRC=main.c

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)

clean:
	rm -f $(TARGET)

