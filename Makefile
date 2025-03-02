# Compiler and flags
CC = gcc
FASTER = -O3
CFLAGS = -Wall -Wextra -std=c99 -g #$(FASTER)
LDFLAGS = -lm

# Directories
SRC_DIR = Definitions
DECL_DIR = Declarations
BUILD_DIR = build
BIN_ = target
BIN_DIR = $(BIN_)/release
TEST_DIR = tests

# Targets
EXEC = $(BIN_DIR)/spreadsheet
TEST_EXEC = $(BIN_DIR)/test_suite
REPORT = report.pdf

# Source files
MAIN_SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/frontend.c $(SRC_DIR)/backend.c $(SRC_DIR)/dS.c $(SRC_DIR)/parser.c
TEST_SRCS = $(wildcard $(TEST_DIR)/*.c)

# Objects
MAIN_OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(MAIN_SRCS))
TEST_OBJS = $(patsubst $(TEST_DIR)/%.c,$(BUILD_DIR)/%.o,$(TEST_SRCS))

.PHONY: all test report clean directories valgrind

# Build the main program
all: directories $(EXEC)

directories:
	@mkdir -p $(BUILD_DIR) $(BIN_DIR)

$(EXEC): $(MAIN_OBJS)
	@$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(DECL_DIR)/ds.h $(DECL_DIR)/parser.h
	@$(CC) $(CFLAGS) -c $< -o $@ -g

# Run tests
test: directories $(TEST_EXEC)
	@$(TEST_EXEC)

$(TEST_EXEC): $(filter-out $(BUILD_DIR)/main.o, $(MAIN_OBJS)) $(TEST_OBJS)
	@$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Run program with valgrind
valgrind: $(EXEC)
	@valgrind --leak-check=full --track-origins=yes $(EXEC)

# Generate the report
report:
	@pdflatex -interaction=nonstopmode report.tex
	@pdflatex -interaction=nonstopmode report.tex  # Run twice to resolve references
	@rm -f *.aux *.log *.out *.toc

# Clean all generated files
clean:
	@rm -rf $(BUILD_DIR) $(BIN_DIR) $(BIN_) $(TEST_EXEC)
	@rm -f *.aux *.log *.out *.toc $(REPORT)
