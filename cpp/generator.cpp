#include "generator.h"
#include "generatorException.h"
#include <openssl/evp.h>

Generator::Generator(GeneratorArgs args)
{
    this->args = args;
}

uint8_t const Generator::zerosArray[4096] = {0};

void Generator::setup()
{
    Seed bootstrapSeed, iterationSeed;
    Pattern pattern;

    findBootstrapSeed(this->args, bootstrapSeed);

    generatePattern(pattern, this->args.CS);

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

void Generator::generatePattern(Pattern &pattern, const std::string &const confusionString)
{

    unsigned int patternSize = PatternBytes;
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();

    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(ctx, confusionString.c_str(), confusionString.length());
    EVP_DigestFinal_ex(ctx, &pattern.bytes[0], &patternSize);

    EVP_MD_CTX_free(ctx);
}

void Generator::findBootstrapSeed(const GeneratorArgs &args, Seed &seed)
{
    if (PKCS5_PBKDF2_HMAC(
            args.PW.c_str(),
            args.PW.length(), (unsigned char *)this->args.CS.c_str(),
            this->args.CS.length(),
            args.IC,
            EVP_sha256(),
            sizeof(seed),
            seed.bytes) != 1)
    {
        throw GeneratorException("Error while calculating PBKDF2", GeneratorExceptionTypes::GENERATOR_SETUP_ERROR);
    }
}

void Generator::initializeGenerator(Seed &seed)
{
    if (this->cipher.ctx != NULL)
        EVP_CIPHER_CTX_free(this->cipher.ctx);

    this->cipher.ctx = EVP_CIPHER_CTX_new();

    if (EVP_EncryptInit(this->cipher.ctx, this->cipher.cipher, seed.bytes, NULL) != 1)
        throw GeneratorException("Error while initializing generator", GeneratorExceptionTypes::GENERATOR_SETUP_ERROR);
}

void Generator::findNextSeedByPattern(const Pattern &pattern, Seed &seed)
{
    uint8_t lbatch[4];
    uint8_t rbatch[4];

    do {
        
        this->seekNextBytesFromGenerator(lbatch, sizeof(lbatch));
        this->seekNextBytesFromGenerator(rbatch, sizeof(rbatch));

        //todo: finish this
    } while(true);
}
 