# Compiler
CXX := g++

# Project Directories
SRC_DIR   := src
TESTS_DIR := tests
TOOLS_DIR := tools
BUILD_DIR := build

# Explicitly list source files
SRC_FILES := $(SRC_DIR)/board.cpp \
             $(SRC_DIR)/evaluate.cpp \
             $(SRC_DIR)/fen.cpp \
             $(SRC_DIR)/movegen.cpp \
             $(SRC_DIR)/search.cpp \
             $(SRC_DIR)/uci.cpp \
             $(SRC_DIR)/zobrist.cpp

# Explicitly list header files (for dependency tracking)
HEADER_FILES := $(SRC_DIR)/bitboard.h \
                $(SRC_DIR)/board.h \
                $(SRC_DIR)/evaluate.h \
                $(SRC_DIR)/fen.h \
                $(SRC_DIR)/helpers.h \
                $(SRC_DIR)/magic.h \
                $(SRC_DIR)/move.h \
                $(SRC_DIR)/movegen.h \
                $(SRC_DIR)/search.h \
                $(SRC_DIR)/tt.h \
                $(SRC_DIR)/types.h \
                $(SRC_DIR)/uci.h \
                $(SRC_DIR)/zobrist.h

# Explicitly list test and tool files
TEST_FILES := $(TESTS_DIR)/tt_test.cpp \
              $(TESTS_DIR)/movegen_test.cpp \
              $(TESTS_DIR)/search_regression_test.cpp \
              $(TESTS_DIR)/uci_test.cpp \
              $(TESTS_DIR)/fen_parsing_and_printing.cpp \
              $(TESTS_DIR)/test_perft.cpp \
              $(TESTS_DIR)/moving_test.cpp

TOOL_FILES := $(TOOLS_DIR)/bench.cpp \
              $(TOOLS_DIR)/game_simulation.cpp \
              $(TOOLS_DIR)/magic_gen.cpp \
              $(TOOLS_DIR)/perft.cpp \
              $(TOOLS_DIR)/search_test.cpp \
              $(TOOLS_DIR)/uci_engine.cpp

# Base flags
CXX_BASE_FLAGS := -std=c++23 -I./src

# Google Test flags
GTEST_LIBS  := $(shell pkg-config --libs gtest gtest_main)
GTEST_FLAGS := $(shell pkg-config --cflags gtest)

# ----------------- Debug Configuration -----------------
DEBUG_ROOT    := $(BUILD_DIR)/debug
DEBUG_FLAGS   := $(CXX_BASE_FLAGS) -Wall -Wextra -Wpedantic -Wshadow -Wconversion \
                 -Wsign-conversion -Wcast-align -Wformat=2 -Wuninitialized \
                 -Wdouble-promotion -Wnull-dereference -Wold-style-cast -g -Og -DDEBUG
DEBUG_OBJS    := $(patsubst $(SRC_DIR)/%.cpp,$(DEBUG_ROOT)/objs/%.o,$(SRC_FILES))
DEBUG_TOOLS   := $(patsubst $(TOOLS_DIR)/%.cpp,$(DEBUG_ROOT)/%,$(TOOL_FILES))
DEBUG_TEST    := $(DEBUG_ROOT)/functionality_test

# ----------------- Release Configuration -----------------
RELEASE_ROOT  := $(BUILD_DIR)/release
RELEASE_FLAGS := $(CXX_BASE_FLAGS) -O3 -march=native -DNDEBUG -flto -fno-exceptions -fno-rtti
RELEASE_OBJS  := $(patsubst $(SRC_DIR)/%.cpp,$(RELEASE_ROOT)/objs/%.o,$(SRC_FILES))
RELEASE_TOOLS := $(patsubst $(TOOLS_DIR)/%.cpp,$(RELEASE_ROOT)/%,$(TOOL_FILES))

# ----------------- Default Target -----------------
all: debug release

debug: $(DEBUG_TOOLS) $(DEBUG_TEST)

release: $(RELEASE_TOOLS)

# ----------------- Debug Build Rules -----------------
$(DEBUG_ROOT)/objs/%.o: $(SRC_DIR)/%.cpp $(HEADER_FILES)
	@mkdir -p $(dir $@)
	$(CXX) $(DEBUG_FLAGS) -c $< -o $@

$(DEBUG_ROOT)/%: $(TOOLS_DIR)/%.cpp $(DEBUG_OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(DEBUG_FLAGS) $^ -o $@

$(DEBUG_TEST): $(TEST_FILES) $(DEBUG_OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(DEBUG_FLAGS) $(GTEST_FLAGS) $^ $(GTEST_LIBS) -o $@

# ----------------- Release Build Rules -----------------
$(RELEASE_ROOT)/objs/%.o: $(SRC_DIR)/%.cpp $(HEADER_FILES)
	@mkdir -p $(dir $@)
	$(CXX) $(RELEASE_FLAGS) -c $< -o $@

$(RELEASE_ROOT)/%: $(TOOLS_DIR)/%.cpp $(RELEASE_OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(RELEASE_FLAGS) $^ -o $@

# ----------------- Tests -----------------
test: $(DEBUG_TEST)
	@echo "Running unified functionality tests..."
	@./$(DEBUG_TEST)

# ----------------- Performance -----------------
PERF_ROOT  := $(BUILD_DIR)/perf
PERF_FLAGS := $(CXX_BASE_FLAGS) -O3 -march=native -g -DNDEBUG -flto
PERF_OBJS  := $(patsubst $(SRC_DIR)/%.cpp,$(PERF_ROOT)/objs/%.o,$(SRC_FILES))
PERF_TOOLS := $(patsubst $(TOOLS_DIR)/%.cpp,$(PERF_ROOT)/%,$(TOOL_FILES))

perf: $(PERF_TOOLS)

$(PERF_ROOT)/objs/%.o: $(SRC_DIR)/%.cpp $(HEADER_FILES)
	@mkdir -p $(dir $@)
	$(CXX) $(PERF_FLAGS) -c $< -o $@

$(PERF_ROOT)/%: $(TOOLS_DIR)/%.cpp $(PERF_OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(PERF_FLAGS) $^ -o $@

# ----------------- Clean -----------------
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all debug release perf clean test
