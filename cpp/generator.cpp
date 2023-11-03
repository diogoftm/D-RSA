#include "generator.h"
#include <openssl/evp.h>

Generator::Generator(GeneratorArgs args) {
    //PKCS5_PBKDF2_HMAC()
    this->args = args;
}

void Generator::setup() {

    Seed bootstrapSeed, iterationSeed;
    findBootstrapSeed(this->args, bootstrapSeed);
    initializeGenerator(bootstrapSeed);

    for(uint32_t i=0;i<this->args.IC;i++) {
        findNextSeed(iterationSeed);
        initializeGenerator(iterationSeed);
    }
}

void Generator::nextBlock(uint8_t* block, uint32_t blockLength) {

}

void Generator::findBootstrapSeed(const GeneratorArgs& args, Seed& seed) {

}

void Generator::initializeGenerator(Seed& seed) {

}

void Generator::findNextSeed(Seed& seed) {
    
}