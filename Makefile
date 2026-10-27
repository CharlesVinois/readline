CC = cc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -O2
TARGET = build/readline
LIBRARY = build/libreadline_stats.a
DEBUG_TARGET = build/readline-debug
TEST_TARGET = build/test_count
EXAMPLE_TARGET = build/count_file
COVERAGE_DIR = build/coverage
COVERAGE_CFLAGS = $(CFLAGS) -O0 -g --coverage -fprofile-abs-path
GCOV ?= gcov
PREFIX ?= /usr/local
DESTDIR ?=
LIBRARY_OBJECT = build/count.o
APP_OBJECT = build/main.o
HEADERS = include/readline_stats.h
UNITY_DIR = vendor/unity
UNITY_SOURCE = $(UNITY_DIR)/src/unity.c
UNITY_FILES = $(wildcard $(UNITY_DIR)/src/*.c $(UNITY_DIR)/src/*.h)
COVERAGE_APP_OBJECTS = $(COVERAGE_DIR)/main.o $(COVERAGE_DIR)/count.o
COVERAGE_TEST_OBJECTS = $(COVERAGE_DIR)/test_count.o $(COVERAGE_DIR)/count.o $(COVERAGE_DIR)/unity.o

.PHONY: all library run test debug example coverage install clean check-unity

all: $(LIBRARY) $(TARGET)

library: $(LIBRARY)

$(LIBRARY_OBJECT): src/count.c $(HEADERS)
	mkdir -p build
	$(CC) $(CFLAGS) -Iinclude -c $< -o $@

$(LIBRARY): $(LIBRARY_OBJECT)
	$(AR) rcs $@ $^

$(APP_OBJECT): src/main.c $(HEADERS)
	mkdir -p build
	$(CC) $(CFLAGS) -Iinclude -c $< -o $@

$(TARGET): $(APP_OBJECT) $(LIBRARY)
	$(CC) $(CFLAGS) $(APP_OBJECT) $(LIBRARY) -o $@

$(DEBUG_TARGET): src/main.c src/count.c $(HEADERS)
	mkdir -p build
	$(CC) -std=c17 -Wall -Wextra -Wpedantic -O0 -g -Iinclude src/main.c src/count.c -o $(DEBUG_TARGET)

debug: $(DEBUG_TARGET)

# Signale comment initialiser Unity si le sous-module n'a pas été récupéré.
check-unity:
	test -f $(UNITY_SOURCE) || { \
		printf 'Unity is missing. Run: git submodule update --init --recursive\n'; \
		exit 1; \
	}

# Compile les tests unitaires avec Unity et la bibliothèque publique.
$(TEST_TARGET): tests/test_count.c $(LIBRARY) $(HEADERS) $(UNITY_FILES) check-unity
	mkdir -p build
	$(CC) $(CFLAGS) -Iinclude -I$(UNITY_DIR)/src tests/test_count.c $(LIBRARY) $(UNITY_SOURCE) -o $(TEST_TARGET)

# Compile un client d'exemple contre la bibliothèque statique.
$(EXAMPLE_TARGET): examples/count_file.c $(LIBRARY) $(HEADERS)
	mkdir -p build
	$(CC) $(CFLAGS) -Iinclude $< $(LIBRARY) -o $@

example: $(EXAMPLE_TARGET)

install: $(LIBRARY)
	install -d $(DESTDIR)$(PREFIX)/lib $(DESTDIR)$(PREFIX)/include
	install -m 644 $(LIBRARY) $(DESTDIR)$(PREFIX)/lib/libreadline_stats.a
	install -m 644 $(HEADERS) $(DESTDIR)$(PREFIX)/include/readline_stats.h

# Construit l'exécutable et les tests avec les compteurs gcov.
$(COVERAGE_DIR)/main.o: src/main.c $(HEADERS)
	mkdir -p $(COVERAGE_DIR)
	$(CC) $(COVERAGE_CFLAGS) -Iinclude -c $< -o $@

$(COVERAGE_DIR)/count.o: src/count.c $(HEADERS)
	mkdir -p $(COVERAGE_DIR)
	$(CC) $(COVERAGE_CFLAGS) -Iinclude -c $< -o $@

$(COVERAGE_DIR)/test_count.o: tests/test_count.c $(HEADERS) $(UNITY_FILES) check-unity
	mkdir -p $(COVERAGE_DIR)
	$(CC) $(COVERAGE_CFLAGS) -Iinclude -I$(UNITY_DIR)/src -c $< -o $@

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
test: $(LIBRARY) $(TARGET) $(TEST_TARGET) $(EXAMPLE_TARGET)
	./$(TEST_TARGET)
	sh tests/test_line_count.sh "$(TARGET)"
	sh tests/test_library_example.sh "$(EXAMPLE_TARGET)"

# Exécute les tests instrumentés et produit les rapports gcov de src/.
coverage: test $(COVERAGE_DIR)/readline $(COVERAGE_DIR)/test_count
	rm -f $(COVERAGE_DIR)/*.gcda
	./$(COVERAGE_DIR)/test_count
	sh tests/test_line_count.sh "$(COVERAGE_DIR)/readline"
	cd $(COVERAGE_DIR) && $(GCOV) -b -c -o . ../../src/main.c ../../src/count.c

clean:
	rm -rf build
