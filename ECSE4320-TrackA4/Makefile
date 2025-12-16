CXX = g++
CXXFLAGS = -O3 -std=c++20 -Wall -Wextra -pthread -march=native
INCLUDES = -Iinclude -Ibench

SRC = src/main.cpp \
      src/hashtable_coarse.cpp \
      src/hashtable_striped.cpp \
      src/hash.cpp

BIN = bin/a4

all: $(BIN)

$(BIN): $(SRC)
	mkdir -p bin
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@

clean:
	rm -rf bin
