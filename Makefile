CC = cc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -O2
TARGET = build/readline
LIBRARY = build/libreadline_stats.a
DEBUG_TARGET = build/readline-debug
UNIT_TEST_TARGET = build/test_count
EXAMPLE_TARGET = build/count_file
FAULT_TARGET = build/readline-faults
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
FAULT_INJECTION_SOURCE = tests/functional/fault_injection.c
COVERAGE_UNIT_OBJECTS = $(COVERAGE_DIR)/test_count.o $(COVERAGE_DIR)/count.o $(COVERAGE_DIR)/unity.o

.PHONY: all library run test test-unit test-functional test-integration debug example coverage install clean check-unity

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

$(FAULT_TARGET): src/main.c src/count.c $(FAULT_INJECTION_SOURCE) $(HEADERS)
	mkdir -p build
	$(CC) $(CFLAGS) -Iinclude src/main.c src/count.c $(FAULT_INJECTION_SOURCE) \
		-Wl,--wrap=fclose -Wl,--wrap=readline_count_stream -o $@

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
$(UNIT_TEST_TARGET): tests/unit/test_count.c $(LIBRARY) $(HEADERS) $(UNITY_FILES) check-unity
	mkdir -p build
	$(CC) $(CFLAGS) -Iinclude -I$(UNITY_DIR)/src $< $(LIBRARY) $(UNITY_SOURCE) -o $@

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

$(COVERAGE_DIR)/test_count.o: tests/unit/test_count.c $(HEADERS) $(UNITY_FILES) check-unity
	mkdir -p $(COVERAGE_DIR)
	$(CC) $(COVERAGE_CFLAGS) -Iinclude -I$(UNITY_DIR)/src -c $< -o $@

$(COVERAGE_DIR)/unity.o: $(UNITY_SOURCE) $(UNITY_FILES) check-unity
	mkdir -p $(COVERAGE_DIR)
	$(CC) $(COVERAGE_CFLAGS) -I$(UNITY_DIR)/src -c $(UNITY_SOURCE) -o $@

$(COVERAGE_DIR)/fault_injection.o: $(FAULT_INJECTION_SOURCE) $(HEADERS)
	mkdir -p $(COVERAGE_DIR)
	$(CC) $(COVERAGE_CFLAGS) -Iinclude -c $< -o $@

$(COVERAGE_DIR)/readline: $(COVERAGE_APP_OBJECTS) $(COVERAGE_DIR)/fault_injection.o
	$(CC) --coverage $^ -Wl,--wrap=fclose -Wl,--wrap=readline_count_stream -o $@

$(COVERAGE_DIR)/test_count: $(COVERAGE_UNIT_OBJECTS)
	$(CC) --coverage $^ -o $@

$(COVERAGE_DIR)/count_file: examples/count_file.c $(COVERAGE_DIR)/count.o $(HEADERS)
	$(CC) $(COVERAGE_CFLAGS) -Iinclude $< $(COVERAGE_DIR)/count.o -o $@

run: $(TARGET)
	./$(TARGET) $(ARGS)

# Chaque cible exécute une seule catégorie de tests.
test-unit: $(UNIT_TEST_TARGET)
	./$(UNIT_TEST_TARGET)

test-functional: $(TARGET) $(FAULT_TARGET)
	sh tests/functional/test_cli.sh "$(TARGET)"
	sh tests/functional/test_cli_faults.sh "$(FAULT_TARGET)"

test-integration: $(EXAMPLE_TARGET)
	sh tests/integration/test_library_example.sh "$(EXAMPLE_TARGET)"

test: test-unit test-functional test-integration

# Exécute chaque catégorie sur les binaires instrumentés et produit les rapports.
coverage: $(COVERAGE_DIR)/readline $(COVERAGE_DIR)/test_count $(COVERAGE_DIR)/count_file
	rm -f $(COVERAGE_DIR)/*.gcda
	./$(COVERAGE_DIR)/test_count
	sh tests/functional/test_cli.sh "$(COVERAGE_DIR)/readline"
	sh tests/functional/test_cli_faults.sh "$(COVERAGE_DIR)/readline"
	sh tests/integration/test_library_example.sh "$(COVERAGE_DIR)/count_file"
	cd $(COVERAGE_DIR) && $(GCOV) -b -c -o . ../../src/main.c ../../src/count.c \
		| tee gcov.log
	test "$$(grep -c '^Lines executed:100.00%' $(COVERAGE_DIR)/gcov.log)" -eq 3
	test "$$(grep -c '^Branches executed:100.00%' $(COVERAGE_DIR)/gcov.log)" -eq 2
	test "$$(grep -c '^Taken at least once:100.00%' $(COVERAGE_DIR)/gcov.log)" -eq 2

clean:
	rm -rf build
