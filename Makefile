CC = cc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -O2
TARGET = build/readline
DEBUG_TARGET = build/readline-debug
TEST_TARGET = build/test_count
SOURCES = src/main.c src/count.c
HEADERS = src/count.h
UNITY_DIR = vendor/unity
UNITY_SOURCE = $(UNITY_DIR)/src/unity.c
UNITY_FILES = $(wildcard $(UNITY_DIR)/src/*.c $(UNITY_DIR)/src/*.h)

.PHONY: all run test debug clean check-unity

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	mkdir -p build
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

$(DEBUG_TARGET): $(SOURCES) $(HEADERS)
	mkdir -p build
	$(CC) -std=c17 -Wall -Wextra -Wpedantic -O0 -g $(SOURCES) -o $(DEBUG_TARGET)

debug: $(DEBUG_TARGET)

# Signale comment initialiser Unity si le sous-module n'a pas été récupéré.
check-unity:
	test -f $(UNITY_SOURCE) || { \
		printf 'Unity is missing. Run: git submodule update --init --recursive\n'; \
		exit 1; \
	}

# Compile les tests unitaires avec Unity.
$(TEST_TARGET): tests/test_count.c src/count.c $(HEADERS) $(UNITY_FILES) check-unity
	mkdir -p build
	$(CC) $(CFLAGS) -Isrc -I$(UNITY_DIR)/src tests/test_count.c src/count.c $(UNITY_SOURCE) -o $(TEST_TARGET)

run: $(TARGET)
	./$(TARGET) $(ARGS)

# Exécute les tests unitaires Unity et les tests fonctionnels.
test: $(TARGET) $(TEST_TARGET)
	./$(TEST_TARGET)
	sh tests/test_line_count.sh "$(TARGET)"

clean:
	rm -rf build
