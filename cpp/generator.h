#pragma once

#include <string>
#include <bitset>

#define PatternBytes 2
#define ZEROS_ARRAY_SIZE 4096

union Pattern {
    uint8_t bytes[PatternBytes];
    uint16_t hword[PatternBytes / 2];
};

struct Cipher {
    const EVP_CIPHER* cipher = EVP_chacha20();
    EVP_CIPHER_CTX* ctx = NULL;
};

struct Seed {
    uint8_t bytes[32];
};


struct GeneratorArgs {
    std::string PW;
    std::string CS;
    uint16_t IC;
};


class Generator {

    public:

        Generator(GeneratorArgs args);

        // Runs setup algorithm
        void setup();

        void nextBlock(uint8_t* block, int blockLength);

    private:

        void findBootstrapSeed(const GeneratorArgs& args, Seed& seed);
        void initializeGenerator(Seed& seed);
        void findNextSeedByPattern(const Pattern& pattern, Seed& seed);
        void seekNextBytesFromGenerator(uint8_t* out, int nbytes);
        
        static void generatePattern(Pattern& pattern, const std::string& const confusionString);

        GeneratorArgs args;
        Cipher cipher;
        bool setupDone = false;
        static const uint8_t zerosArray[ZEROS_ARRAY_SIZE];
};