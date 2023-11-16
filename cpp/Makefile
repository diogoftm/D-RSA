# Compiler and compiler flags
CC = g++

GIT_FLAG := 

CFLAGS = -Wall -g -Wno-deprecated-declarations $(GIT_FLAG)

TARGETS = rsagen RBG test_RBG test_generator

OPENSSL = -lssl -lcrypto -lsodium -largon2

GTEST = -lgtest -lgtest_main

SRCS = generator.cpp rsagen.cpp RBG.cpp test_RBG.cpp test_generator.cpp

OBJS = $(SRCS:.cpp=.o)

all: $(TARGETS)
	
rsagen: rsagen.o
	$(CC) $(CFLAGS) -o rsagen rsagen.o $(OPENSSL)

RBG: generator.o RBG.o
	$(CC) $(CFLAGS) -o RBG  generator.o RBG.o $(OPENSSL)

test_RBG: test_RBG.o
	$(CC) $(CFLAGS) -o test_RBG test_RBG.o $(GTEST)

test_generator: test_generator.o
	$(CC) $(CFLAGS) -o test_generator test_generator.o generator.o $(GTEST) $(OPENSSL)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@



clean:
	rm -f $(OBJS) $(TARGETS)

.PHONY: all clean
