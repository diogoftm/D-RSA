#include "generator.h"
#include "generatorException.h"
#include <stdio.h>
#include <argon2.h>
#include <cstring>
#include <sodium.h>
#include <math.h>
#include <algorithm>
#include <iostream>

Generator::Generator(GeneratorArgs args)
{
    this->args = args;

    if(sodium_init() < 0)
        throw GeneratorException("Unable to initialize sodium", GeneratorExceptionTypes::GENERATOR_SETUP_ERROR);

}

uint8_t const Generator::zerosArray[4096] = {0};

void Generator::setup()
{
    Seed bootstrapSeed, iterationSeed;
    Pattern pattern;
    pattern.size = this->args.patternBytes;

    findBootstrapSeed(this->args, bootstrapSeed);

    Generator::generatePattern(pattern, this->args.CS);

    initializeGenerator(bootstrapSeed);

    for (uint32_t i = 0; i < this->args.IC; i++)
    {
        findNextSeedByPattern(pattern, iterationSeed);
        initializeGenerator(iterationSeed);
    }

    setupDone = true;
}

void Generator::nextBlock(uint8_t *block, int blockLength)
{
    if (!setupDone)
        throw GeneratorException("Could not call Generator::nextBlock without calling Generator::setup()", GeneratorExceptionTypes::GENERATOR_RUNTIME_ERROR);

    this->seekNextBytesFromGenerator(block, blockLength);
}

void Generator::seekNextBytesFromGenerator(uint8_t *out, int blockLength)
{
    int outLen;
    int encryptedBytes = 0;

    do
    {
        EVP_EncryptUpdate(this->cipher.ctx, &out[0] + encryptedBytes, &outLen, zerosArray, blockLength % ZEROS_ARRAY_SIZE);
        encryptedBytes += outLen;
        blockLength -= ZEROS_ARRAY_SIZE;

    } while (encryptedBytes < blockLength);
}

void Generator::generatePattern(Pattern &pattern, std::string confusionString)
{
    unsigned int patternSize = pattern.size;
    uint8_t BUFF[32];
    int strsize = confusionString.length();
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    pattern.bytes = std::vector<uint8_t>();
    if (ctx != nullptr)
    {
        if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) == 1 &&
            EVP_DigestUpdate(ctx, confusionString.c_str(), strsize) == 1 &&
            EVP_DigestFinal_ex(ctx, BUFF, &patternSize) == 1)
        {
            for(int i=0;i<pattern.size;i++) {
                pattern.bytes.push_back(BUFF[i]);
            }

            
        }
        else
        {
            throw GeneratorException("Error in OpenSSL functions", GeneratorExceptionTypes::GENERATOR_SETUP_ERROR);
        }

        EVP_MD_CTX_free(ctx);
    }
    else
    {
        throw GeneratorException("Error creating EVP_MD_CTX", GeneratorExceptionTypes::GENERATOR_SETUP_ERROR);
    }
}

void Generator::findBootstrapSeed(const GeneratorArgs &args, Seed &seed)
{

    int memoryUsage = getArgon2MemoryUsageByIC(args.IC);
    int iterations = getArgon2IterationsByIC(args.IC);

    unsigned char salt[crypto_pwhash_argon2i_SALTBYTES];

    setArgon2Salt(salt, args.CS.c_str(), args.IC);

    const char* PW = args.PW.c_str();
    const int PW_Len = strlen(PW);

    int status = argon2_hash(
        iterations, memoryUsage, 1,
        PW, PW_Len,
        salt, 16,
        seed.bytes, sizeof(seed.bytes),
        nullptr, 0,
        Argon2_i, ARGON2_VERSION_NUMBER);

    if (status != 0) {
        throw GeneratorException("Error while calculating Argon2 Bootstrap Seed", GeneratorExceptionTypes::GENERATOR_SETUP_ERROR);
    }
}

void Generator::setArgon2Salt(unsigned char* salt, const char* CS, int IC) {

    auto ctx = EVP_MD_CTX_new();

    uint8_t digest[32];

    EVP_DigestInit(ctx, EVP_sha256());
    EVP_DigestUpdate(ctx,CS, strlen(CS));
    EVP_DigestUpdate(ctx, &IC, 4);

    unsigned int mdLen;
    EVP_DigestFinal(ctx,digest, &mdLen);
    EVP_MD_CTX_free(ctx);

    memcpy(salt, digest, crypto_pwhash_argon2i_SALTBYTES);
}

void Generator::initializeGenerator(Seed &seed)
{
    if (this->cipher.ctx != NULL)
        EVP_CIPHER_CTX_free(this->cipher.ctx);

    this->cipher.ctx = EVP_CIPHER_CTX_new();

    if (EVP_EncryptInit(this->cipher.ctx, this->cipher.cipher, seed.bytes, NULL) != 1)
        throw GeneratorException("Error while initializing generator", GeneratorExceptionTypes::GENERATOR_SETUP_ERROR);
}

void Generator::calculateSeed(Seed &outSeed, const SHA256Result &hashResult, const LeadingPatternBytes &leadingBytes)
{
    auto ctx = EVP_MD_CTX_new();

    EVP_DigestInit(ctx, EVP_sha256());
    EVP_DigestUpdate(ctx, hashResult.bytes, sizeof(hashResult.bytes));
    EVP_DigestUpdate(ctx, leadingBytes.bytes, sizeof(leadingBytes.bytes));

    unsigned int mdLen;
    EVP_DigestFinal(ctx, &outSeed.bytes[0], &mdLen);
    EVP_MD_CTX_free(ctx);
}

void Generator::findNextSeedByPattern(const Pattern &pattern, Seed &seed)
{


    auto mdCtx = EVP_MD_CTX_new();
    EVP_DigestInit(mdCtx, EVP_sha256());

    uint8_t B;
    SHA256Result result;
    LeadingPatternBytes leading;


    std::vector<uint8_t> currPattern = std::vector<uint8_t>();
    uint8_t* block = new uint8_t[pattern.size];
    seekNextBytesFromGenerator(block, pattern.size);
    
    for(int i=0;i<pattern.size;i++) {
        currPattern.push_back(block[i]);
    }

    delete[] block;


    for (;;)
    {

        if(currPattern.size() == (long unsigned int)(pattern.size + 1))
            currPattern.erase(currPattern.begin());


        if (currPattern.size() == (long unsigned int)this->args.patternBytes && std::equal(currPattern.begin(), currPattern.end(),  pattern.bytes.begin(), pattern.bytes.end()))
        {
            EVP_DigestFinal(mdCtx, result.bytes, NULL);

            this->seekNextBytesFromGenerator(leading.bytes, 32);
            Generator::calculateSeed(seed, result, leading);
            EVP_MD_CTX_free(mdCtx);
            return;
        }
        else
        {
            EVP_DigestUpdate(mdCtx, &currPattern.data()[0], 1);
            this->seekNextBytesFromGenerator(&B, 1);
            currPattern.push_back(B);
        }
    }
}


int Generator::getArgon2MemoryUsageByIC(int IC) {
    return 1024*1024;
    
}

int Generator::getArgon2IterationsByIC(int IC) {

    static int minIterations = crypto_pwhash_argon2i_OPSLIMIT_MIN;
    static int maxIterations = crypto_pwhash_argon2i_OPSLIMIT_MODERATE;

    int usedIterations = minIterations + (int)((maxIterations - (double)minIterations) * (log10(IC) / 4.0));

    if(minIterations <= usedIterations && usedIterations <= maxIterations)
        return usedIterations;

    if(usedIterations < minIterations)
        return minIterations;
    
    return maxIterations;
}