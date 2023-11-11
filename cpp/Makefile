# Compiler and compiler flags
CC = g++
CFLAGS = -Wall -g -Wno-deprecated-declarations

TARGETS = rsagen RBG

OPENSSL = -lssl -lcrypto

GTEST = -lgtest -lgtest_main

SRCS = generator.cpp rsagen.cpp RBG.cpp

OBJS = $(SRCS:.cpp=.o)

all: $(TARGETS)
	
rsagen: rsagen.o
	$(CC) $(CFLAGS) -o rsagen rsagen.o $(OPENSSL)

RBG: generator.o RBG.o
	$(CC) $(CFLAGS) -o RBG  generator.o RBG.o $(OPENSSL)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@



clean:
	rm -f $(OBJS) $(TARGETS)

.PHONY: all clean
