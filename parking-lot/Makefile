CXX ?= g++
CXXFLAGS ?= -O2 -g
CXXFLAGS += -std=c++17 -Wall -Wextra -Wpedantic -Wshadow
CPPFLAGS += -Iinclude

APP := parking_lot
SRC := $(wildcard src/*.cpp)
CORE_SRC := $(filter-out src/main.cpp,$(SRC))
TEST_BIN := tests/test_core

all: $(APP)

$(APP): $(SRC)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(SRC) -o $@

$(TEST_BIN): $(CORE_SRC) tests/test_core.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(CORE_SRC) tests/test_core.cpp -o $@

test: $(TEST_BIN)
	./$(TEST_BIN)

run: $(APP)
	./$(APP)

clean:
	rm -f $(APP) $(TEST_BIN)

.PHONY: all test run clean
