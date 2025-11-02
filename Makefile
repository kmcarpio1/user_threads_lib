default: all

# Dossiers
SRC_DIR := ./src
TST_DIR := ./tst
OBJ_DIR := ./dist

TSTGRP_DIR := $(TST_DIR)/graphs
TSTFRG_DIR := $(TST_DIR)/tst_forge
TSTOBJ_DIR := $(OBJ_DIR)/tst_frg

TSTGRP_DATA_DIR := $(TSTGRP_DIR)/data
TSTGRP_RESULTS_DIR := $(TSTGRP_DIR)/results

BIN_DIR := ./install/bin
LIB_DIR := ./install/lib

# Options de compilation
CC = gcc
CFLAGS = -Wall -Wextra -fvisibility=hidden -fPIC -O2 -I src 

# Fichiers sources
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
TSTFRG_FILES := \
    ./tst/tst_forge/01-main.c \
    ./tst/tst_forge/02-switch.c \
    ./tst/tst_forge/03-equity.c \
    ./tst/tst_forge/11-join.c \
    ./tst/tst_forge/12-join-main.c \
    ./tst/tst_forge/21-create-many.c \
    ./tst/tst_forge/22-create-many-recursive.c \
    ./tst/tst_forge/23-create-many-once.c \
    ./tst/tst_forge/31-switch-many.c \
    ./tst/tst_forge/32-switch-many-join.c \
    ./tst/tst_forge/33-switch-many-cascade.c \
    ./tst/tst_forge/51-fibonacci.c \
    ./tst/tst_forge/61-mutex.c \
    ./tst/tst_forge/62-mutex.c \
    ./tst/tst_forge/63-mutex-equity.c \
    ./tst/tst_forge/64-mutex-join.c \
	./tst/tst_forge/81-deadlock.c \
	./tst/tst_forge/91-stack-overflow.c \
	./tst/tst_forge/92-signal.c
	

# Fichiers cibles
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/src/%.o,$(SRC_FILES))
LIB_FILES := $(OBJ_DIR)/libthread.so $(OBJ_DIR)/libthread.a

BINDIST_FILES := $(patsubst $(TSTFRG_DIR)/%.c,$(OBJ_DIR)/tst_frg/%,$(TSTFRG_FILES))
PTHREAD_BINDIST_FILES := $(patsubst $(TSTFRG_DIR)/%.c,$(OBJ_DIR)/tst_frg/%-pthread,$(TSTFRG_FILES))

BIN_FILES := $(patsubst $(TSTFRG_DIR)/%.c,$(BIN_DIR)/%,$(TSTFRG_FILES))
PTHREAD_BIN_FILES := $(patsubst $(TSTFRG_DIR)/%.c,$(BIN_DIR)/%-pthread,$(TSTFRG_FILES))

TST_DATA_FILES := $(wildcard $(TSTGRP_DATA_DIR))
TST_RESULTS_DIR := $(wildcard $(TSTGRP_RESULTS_DIR))

# Make all
all: $(LIB_FILES) $(BINDIST_FILES)

# Make install
install: all
	cp $(OBJ_DIR)/libthread.so $(LIB_DIR)/libthread.so 
	cp $(OBJ_DIR)/libthread.a $(LIB_DIR)/libthread.a
	cp $(OBJ_DIR)/tst_frg/* $(BIN_DIR)

# Cibles de compilation
$(OBJ_DIR)/src/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/tst_frg/%: $(TSTFRG_DIR)/%.c $(OBJ_DIR)/libthread.a
	$(CC) $(CFLAGS) -o $@ $< $(OBJ_DIR)/libthread.a

$(OBJ_DIR)/tst_frg/%-pthread: $(TSTFRG_DIR)/%.c $(OBJ_DIR)/libthread.a
	$(CC) $(CFLAGS) -DUSE_PTHREAD $< -o $@ $(OBJ_DIR)/libthread.a

$(OBJ_DIR)/libthread.so: $(OBJ_FILES)
	$(CC) $(CFLAGS) -shared -o $@ $^

$(OBJ_DIR)/libthread.a: $(OBJ_FILES)
	ar rcs $@ $^

$(BIN_FILES): $(BINDIST_FILES)
$(PTHREAD_BIN_FILES): $(PTHREAD_BINDIST_FILES)

# Make clean
clean:
	find $(LIB_DIR) $(BIN_DIR) $(OBJ_DIR) $(TST_DATA_FILES) $(TST_RESULTS_DIR) -type f -not -name '.gitkeep' -delete

# Make check
check: install
	@echo "BIN_FILES: $(BIN_FILES)"
	@for file in $(BIN_FILES) ; do \
		echo Lance test $$file ; \
		./$$file 24 100 ; \
		echo ; \
	done

# Make valgrind
valgrind: install
	@echo "BIN_FILES: $(BIN_FILES)"
	@for file in $(BIN_FILES) ; do \
		echo Valgrind test $$file ; \
		valgrind $$file --leak-check=full --show-reachable=yes --track-origins=yes ; \
		echo ; \
	done

# Make pthread
pthreads: $(PTHREAD_BIN_FILES)

# Make gdb
gdb: CFLAGS += -g
gdb: all

# Make symbols
symbols: $(LIB_FILES)
	nm -gD $(LIB_DIR)/libthread.so
	nm -gC $(LIB_DIR)/libthread.a

# Make data from benchmark tests (SUBRULE OF MAKE GRAPHS)
data_test: install pthreads
	for executable in $(BINDIST_FILES) $(PTHREAD_BINDIST_FILES); do \
		echo "Generating test $$executable"; \
		python3 ${TSTGRP_DIR}/data.py $$executable; \
	done

# Make graphs from data (SUBRULE OF MAKE GRAPHS)
graph_from_data: data_test
	@for file in $(BINDIST_FILES) ; do \
		echo "Graphing test $$file"; \
		python3 $(TSTGRP_DIR)/graphs.py $$file; \
	done

# Make graphs
graphs : graph_from_data


# Special targets
.PHONY: all clean check valgrind pthreads graphs install
