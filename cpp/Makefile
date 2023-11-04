# Compiler and compiler flags
CC = g++
CFLAGS = -Wall -g -Wno-deprecated-declarations

TARGETS = encrypt decrypt test speed test_edes

OPENSSL = -lssl -lcrypto

GTEST = -lgtest -lgtest_main

SRCS = rsagen.cpp

OBJS = $(SRCS:.cpp=.o)


all: $(TARGETS)
	
rsagen: rsagen.o
	$(CC) $(CFLAGS) -o rsagen rsagen.o $(OPENSSL)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGETS)

# Phony targets
.PHONY: all clean
