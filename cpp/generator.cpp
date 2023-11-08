#include "generator.h"
#include "generatorException.h"


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

void Generator::calculateSeed(Seed& outSeed, const SHA256Result& hashResult, const LeadingPatternBytes& leadingBytes) {
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

    uint8_t B0, B1;
    SHA256Result result;
    LeadingPatternBytes leading;

    this->seekNextBytesFromGenerator(&B0, 1);
    
    for(;;) {
        this->seekNextBytesFromGenerator(&B1, 1);

        if(pattern.bytes[0] == B0 && pattern.bytes[1] == B1) {
            EVP_DigestFinal(mdCtx, result.bytes, NULL);
            
            this->seekNextBytesFromGenerator(leading.bytes, 32);
            this->calculateSeed(seed, result, leading);
            EVP_MD_CTX_free(mdCtx);
            return;

        } else {
            EVP_DigestUpdate(mdCtx, &B0, 1);
            B0 = B1;
        }
    }


}
 