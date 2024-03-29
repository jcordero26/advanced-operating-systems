# the compiler: gcc for C program, define as g++ for C++
CC=gcc

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS=-I. -g -Wall
LFLAGS=-lrt -lpthread -lm
GTKLIB=`pkg-config --cflags --libs gtk+-3.0`


BIN_DIR=bin
LIB_BACKEND=backend/build_tmp/lib/libbackend.a
LIB_GUI=frontend/build_tmp/lib/libgui.a
PRODUCER_APP=$(BIN_DIR)/producer
TERMINATOR_APP=$(BIN_DIR)/terminator
CONSUMER_APP=$(BIN_DIR)/consumer
CREATOR_APP=$(BIN_DIR)/creator

build: backend frontend $(CREATOR_APP) $(TERMINATOR_APP) $(CONSUMER_APP) $(PRODUCER_APP)

.PHONY: backend frontend
backend:
	$(MAKE) -C $@

frontend:
	$(MAKE) -C $@

$(CREATOR_APP): $(BIN_DIR)
	$(CC) $(CFLAGS) -DCREATOR_APP -o $@ main.c $(LIB_BACKEND) $(LIB_GUI) $(LFLAGS) $(GTKLIB)

$(PRODUCER_APP): $(BIN_DIR)
	$(CC) $(CFLAGS) -DPRODUCER_APP -o $@ main.c $(LIB_BACKEND) $(LFLAGS)

$(CONSUMER_APP): $(BIN_DIR)
	$(CC) $(CFLAGS) -DCONSUMER_APP -o $@ main.c $(LIB_BACKEND) $(LFLAGS)

$(TERMINATOR_APP): $(BIN_DIR)
	$(CC) $(CFLAGS) -DTERMINATOR_APP -o $@ main.c $(LIB_BACKEND) $(LFLAGS)

$(BIN_DIR):
	mkdir -p $@

clean:
	rm -rf bin/
	cd backend && make clean
	cd frontend && make clean
