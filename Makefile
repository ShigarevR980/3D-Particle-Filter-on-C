CC = gcc
PYTHON = py

CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2 -Iinclude

TARGET = particle_filter.exe
TEST_TARGET = test_main.exe

SRC = src/main.c src/random_utils.c src/particle_filter.c src/csv_writer.c
TEST_SRC = tests/test_main.c src/random_utils.c src/particle_filter.c src/csv_writer.c

HEADERS = include/random_utils.h include/particle_filter.h include/csv_writer.h

all: $(TARGET)

$(TARGET): $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) -lm

run: $(TARGET)
	./$(TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_SRC) $(HEADERS)
	$(CC) $(CFLAGS) $(TEST_SRC) -o $(TEST_TARGET) -lm

plot:
	$(PYTHON) scripts/plot_results.py

clean:
	powershell -NoProfile -Command "Remove-Item -Path '$(TARGET)', '$(TEST_TARGET)', 'output.csv' -Force -ErrorAction SilentlyContinue; exit 0"

clean-plots:
	powershell -NoProfile -Command "Remove-Item -Recurse -Force -ErrorAction SilentlyContinue 'plots'; exit 0"

clean-all: clean clean-plots

.PHONY: all run test plot clean clean-plots clean-all