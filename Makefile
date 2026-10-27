CC = cc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -O2
TARGET = build/readline
SOURCE = src/main.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCE)
	mkdir -p build
	$(CC) $(CFLAGS) $(SOURCE) -o $(TARGET)

clean:
	rm -rf build
