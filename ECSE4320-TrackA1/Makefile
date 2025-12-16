CC ?= gcc
CFLAGS ?= -O2 -march=native -Wall -Wextra

BIN_DIR := results/raw
AFF_BIN := $(BIN_DIR)/affinity_bench
OVS_BIN := $(BIN_DIR)/oversub_bench
NUMA_BIN := $(BIN_DIR)/numa_bench
SMT_BIN := $(BIN_DIR)/smt_bench

all: $(AFF_BIN) $(OVS_BIN) $(NUMA_BIN) $(SMT_BIN)

$(AFF_BIN): src/affinity_bench.c
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $<

$(OVS_BIN): src/oversub_bench.c
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -pthread -o $@ $<

$(NUMA_BIN): src/numa_bench.c
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $<

$(SMT_BIN): src/smt_bench.c
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $<
	
clean:
	rm -rf $(AFF_BIN) $(OVS_BIN)

.PHONY: all clean