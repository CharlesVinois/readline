CC = cc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -O2
TARGET = build/readline
SOURCE = src/main.c

.PHONY: all run test clean

all: $(TARGET)

$(TARGET): $(SOURCE)
	mkdir -p build
	$(CC) $(CFLAGS) $(SOURCE) -o $(TARGET)

run: $(TARGET)
	./$(TARGET) "$(FILE)"

test: $(TARGET)
	sh tests/test_line_count.sh "$(TARGET)"

clean:
	rm -rf build
