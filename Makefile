CC = cc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -O2
TARGET = build/readline
DEBUG_TARGET = build/readline-debug
TEST_TARGET = build/test_count
COVERAGE_DIR = build/coverage
COVERAGE_CFLAGS = $(CFLAGS) -O0 -g --coverage -fprofile-abs-path
GCOV ?= gcov
SOURCES = src/main.c src/count.c
HEADERS = src/count.h
UNITY_DIR = vendor/unity
UNITY_SOURCE = $(UNITY_DIR)/src/unity.c
UNITY_FILES = $(wildcard $(UNITY_DIR)/src/*.c $(UNITY_DIR)/src/*.h)
COVERAGE_APP_OBJECTS = $(COVERAGE_DIR)/main.o $(COVERAGE_DIR)/count.o
COVERAGE_TEST_OBJECTS = $(COVERAGE_DIR)/test_count.o $(COVERAGE_DIR)/count.o $(COVERAGE_DIR)/unity.o

.PHONY: all run test debug coverage clean check-unity

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

# Construit l'exécutable et les tests avec les compteurs gcov.
$(COVERAGE_DIR)/main.o: src/main.c $(HEADERS)
	mkdir -p $(COVERAGE_DIR)
	$(CC) $(COVERAGE_CFLAGS) -Isrc -c $< -o $@

$(COVERAGE_DIR)/count.o: src/count.c $(HEADERS)
	mkdir -p $(COVERAGE_DIR)
	$(CC) $(COVERAGE_CFLAGS) -Isrc -c $< -o $@

$(COVERAGE_DIR)/test_count.o: tests/test_count.c $(HEADERS) $(UNITY_FILES) check-unity
	mkdir -p $(COVERAGE_DIR)
	$(CC) $(COVERAGE_CFLAGS) -Isrc -I$(UNITY_DIR)/src -c $< -o $@

$(COVERAGE_DIR)/unity.o: $(UNITY_SOURCE) $(UNITY_FILES) check-unity
	mkdir -p $(COVERAGE_DIR)
	$(CC) $(COVERAGE_CFLAGS) -I$(UNITY_DIR)/src -c $(UNITY_SOURCE) -o $@

$(COVERAGE_DIR)/readline: $(COVERAGE_APP_OBJECTS)
	$(CC) --coverage $^ -o $@

$(COVERAGE_DIR)/test_count: $(COVERAGE_TEST_OBJECTS)
	$(CC) --coverage $^ -o $@

run: $(TARGET)
	./$(TARGET) $(ARGS)

# Exécute les tests unitaires Unity et les tests fonctionnels.
test: $(TARGET) $(TEST_TARGET)
	./$(TEST_TARGET)
	sh tests/test_line_count.sh "$(TARGET)"

# Exécute les tests instrumentés et produit les rapports gcov de src/.
coverage: $(COVERAGE_DIR)/readline $(COVERAGE_DIR)/test_count
	rm -f $(COVERAGE_DIR)/*.gcda
	./$(COVERAGE_DIR)/test_count
	sh tests/test_line_count.sh "$(COVERAGE_DIR)/readline"
	cd $(COVERAGE_DIR) && $(GCOV) -b -c -o . ../../src/main.c ../../src/count.c

clean:
	rm -rf build
