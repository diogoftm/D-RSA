#pragma once

#include <string>
#include <bitset>
#include <openssl/evp.h>
#include <vector>

#define PatternBytes 2
#define ZEROS_ARRAY_SIZE 4096

struct GeneratorArgs
{
    std::string PW;
    std::string CS;
    uint16_t IC;
    int patternBytes;
};

class Generator
{

public:
    Generator(GeneratorArgs args);

    // Runs setup algorithm
    void setup();

    void nextBlock(uint8_t *block, int blockLength);

protected:
    struct Pattern
    {
        std::vector<uint8_t> bytes;
        int size;
    };

    struct Cipher
    {
        const EVP_CIPHER *cipher = EVP_chacha20();
        EVP_CIPHER_CTX *ctx = NULL;
    };

    struct _32Bytes
    {
        uint8_t bytes[32];
    };

    using Seed = _32Bytes;
    using SHA256Result = _32Bytes;
    using LeadingPatternBytes = _32Bytes;

    void findBootstrapSeed(const GeneratorArgs &args, Seed &seed);
    void initializeGenerator(Seed &seed);
    void findNextSeedByPattern(const Pattern &pattern, Seed &seed);
    void seekNextBytesFromGenerator(uint8_t *out, int nbytes);

    static void generatePattern(Pattern &pattern, const std::string confusionString);
    static int getArgon2MemoryUsageByIC(int IC);
    static int getArgon2IterationsByIC(int IC);
    static void setArgon2Salt(unsigned char* salt, const char* CS, int IC);

    static void calculateSeed(Seed &outSeed, const SHA256Result &result, const LeadingPatternBytes &leadingBytes);
    GeneratorArgs args;
    Cipher cipher;
    bool setupDone = false;
    static const uint8_t zerosArray[ZEROS_ARRAY_SIZE];
};