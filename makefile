CC=gcc
CFLAGS=-Wall -Wextra -O2
SOURCES=$(wildcard *.c)
TARGET=program

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)
