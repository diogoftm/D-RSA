# Compiler and compiler flags
CC = g++
CFLAGS = -Wall -g -Wno-deprecated-declarations

TARGETS = rsagen generator RBG

OPENSSL = -lssl -lcrypto

GTEST = -lgtest -lgtest_main

SRCS = rsagen.cpp generator.cpp RBG.cpp

OBJS = $(SRCS:.cpp=.o)


all: $(TARGETS)
	
rsagen: rsagen.o
	$(CC) $(CFLAGS) -o rsagen rsagen.o $(OPENSSL)

generator: generator.o
	$(CC) $(CFLAGS) -o generator generator.o $(OPENSSL)

RBG: RBG.o
	$(CC) $(CFLAGS) -o RBG RBG.o $(OPENSSL)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGETS)

.PHONY: all clean
