#pragma once

#include <string>

/*
Usage:
Generator g = Generator(...);
g.setup();
g.nextBlock(block, 512);
...

*/

class Generator {

    public:

        Generator(std::string password, std::string confusionString, uint16_t iterationCount);

        // Runs setup algorithm
        void setup();

        void nextBlock(uint8_t* block, uint32_t blockLength);

    private:

        
};