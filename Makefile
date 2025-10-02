# Compiler
CXX := g++

# Directories
SRC_DIR := src
TEST_DIR := test
BUILD_DIR := build

# Source files
SRC := $(wildcard $(SRC_DIR)/*.cpp)
TEST_SRC := $(wildcard $(TEST_DIR)/*.cpp)

# ---------------- Base flags ----------------
CXX_BASE_FLAGS := -std=c++20 -I./src

# ---------------- Debug build ----------------
DEBUG_DIR := $(BUILD_DIR)/debug
DEBUG_FLAGS := $(CXX_BASE_FLAGS)
DEBUG_FLAGS += -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Wcast-align -Wformat=2 -g -O0

DEBUG_OBJ := $(patsubst $(SRC_DIR)/%.cpp,$(DEBUG_DIR)/%.o,$(SRC))
DEBUG_BIN := $(patsubst $(TEST_DIR)/%.cpp,$(DEBUG_DIR)/%,$(TEST_SRC))

# ---------------- Release build ----------------
RELEASE_DIR := $(BUILD_DIR)/release
RELEASE_FLAGS := $(CXX_BASE_FLAGS)
RELEASE_FLAGS += -O3 -march=native -DNDEBUG

RELEASE_OBJ := $(patsubst $(SRC_DIR)/%.cpp,$(RELEASE_DIR)/%.o,$(SRC))
RELEASE_BIN := $(patsubst $(TEST_DIR)/%.cpp,$(RELEASE_DIR)/%,$(TEST_SRC))

# ---------------- Default target ----------------
all: debug release

# ---------------- Debug build ----------------
debug: $(DEBUG_DIR) $(DEBUG_BIN)

$(DEBUG_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(DEBUG_FLAGS) -c $< -o $@

$(DEBUG_DIR)/%: $(TEST_DIR)/%.cpp $(DEBUG_OBJ)
	$(CXX) $(DEBUG_FLAGS) $^ -o $@

$(DEBUG_DIR):
	mkdir -p $(DEBUG_DIR)

# ---------------- Release build ----------------
release: $(RELEASE_DIR) $(RELEASE_BIN)

$(RELEASE_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(RELEASE_FLAGS) -c $< -o $@

$(RELEASE_DIR)/%: $(TEST_DIR)/%.cpp $(RELEASE_OBJ)
	$(CXX) $(RELEASE_FLAGS) $^ -o $@

$(RELEASE_DIR):
	mkdir -p $(RELEASE_DIR)

# ---------------- Clean ----------------
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all debug release clean
