CC = cc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -O2
TARGET = build/readline
DEBUG_TARGET = build/readline-debug
SOURCE = src/main.c

.PHONY: all run test debug clean

all: $(TARGET)

$(TARGET): $(SOURCE)
	mkdir -p build
	$(CC) $(CFLAGS) $(SOURCE) -o $(TARGET)

$(DEBUG_TARGET): $(SOURCE)
	mkdir -p build
	$(CC) -std=c17 -Wall -Wextra -Wpedantic -O0 -g $(SOURCE) -o $(DEBUG_TARGET)

debug: $(DEBUG_TARGET)

run: $(TARGET)
	./$(TARGET) "$(FILE)"

test: $(TARGET)
	sh tests/test_line_count.sh "$(TARGET)"

clean:
	rm -rf build
