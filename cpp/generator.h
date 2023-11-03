#pragma once

#include <string>



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

        void nextBlock(uint8_t* block, uint32_t blockLength);

    private:

        void findBootstrapSeed(const GeneratorArgs& args, Seed& seed);
        void initializeGenerator(Seed& seed);
        void findNextSeed(Seed& seed);

        GeneratorArgs args;
};