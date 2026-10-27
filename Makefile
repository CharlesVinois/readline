CC = cc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -O2
TARGET = build/readline
DEBUG_TARGET = build/readline-debug
SOURCES = src/main.c src/count.c
HEADERS = src/count.h

.PHONY: all run test debug clean

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	mkdir -p build
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

$(DEBUG_TARGET): $(SOURCES) $(HEADERS)
	mkdir -p build
	$(CC) -std=c17 -Wall -Wextra -Wpedantic -O0 -g $(SOURCES) -o $(DEBUG_TARGET)

debug: $(DEBUG_TARGET)

run: $(TARGET)
	./$(TARGET) $(ARGS)

test: $(TARGET)
	sh tests/test_line_count.sh "$(TARGET)"

clean:
	rm -rf build
