#include <iostream>
#include <stdexcept>
#include "generator.h"
#include <optional>
#include <string>

#define MIN(A,B) A > B ? B : A

const static char *usage = "usage: ./RBG password confusionString iterationCount [--limit nbytes]";

struct OptionalArguments
{
    std::optional<int> limit;
};

void getGeneratorArgsFromArgs(GeneratorArgs &args, int argc, char *argv[])
{
    if (argc != 4 && argc != 6)
        throw std::invalid_argument("Invalid number of arguments");

    std::string PW = std::string(argv[1]);
    std::string CS = std::string(argv[2]);

    int IC;
    try
    {
        IC = std::stoi(argv[3]);
    }
    catch (std::exception &)
    {
        throw std::invalid_argument("Unable to convert iterationCount to an integer");
    }

    if (IC <= 0)
        throw std::invalid_argument("iterationCount should be >= 1");

    args.PW = PW;
    args.CS = CS;
    args.IC = IC;
}

void getOptionalArguments(OptionalArguments &args, int argc, char *argv[])
{
    if (argc == 4)
    {
        args.limit = std::nullopt;
        return;
    }

    std::string limitString = "--limit";

    if(limitString.compare(argv[4])) {
        throw std::invalid_argument("Unrecognized argument '" + std::string(argv[4]) + "'");
    }   

    int limit;

    try
    {
        limit = std::stoi(argv[5]);
    }
    catch (std::exception &)
    {
        throw std::invalid_argument("Unable to convert limit flag to an integer");
    }

    if (limit <= 0)
        throw std::invalid_argument("limit flag should be >= 1");

    args.limit = std::optional<int>(limit);
}

void produceDataUntilLimit(Generator &generator, std::optional<int> limit)
{
    uint8_t block[1024];
    int bytesWritten = 0, bytesToWrite, missingBytes;

    while (!limit.has_value() || bytesToWrite < limit.value())
    {
        generator.nextBlock(block, sizeof(block));

        if(!limit.has_value()) {
            bytesToWrite = sizeof(block);
        } else {
            missingBytes = limit.value() - bytesWritten;
            bytesToWrite = MIN((int)sizeof(block), missingBytes);
        }

        fwrite(block, sizeof(uint8_t), bytesToWrite, stdout);
        bytesWritten += bytesToWrite;
    }
}

int main(int argc, char *argv[])
{

    GeneratorArgs args;
    OptionalArguments optionalArgs;

    try
    {

        getGeneratorArgsFromArgs(args, argc, argv);
        getOptionalArguments(optionalArgs, argc, argv);
    }
    catch (std::exception &exception)
    {

        std::cerr << exception.what() << "\n"
                  << usage << "\n";
        return EXIT_FAILURE;
    }

    Generator generator = Generator(args);
    generator.setup();

    produceDataUntilLimit(generator, optionalArgs.limit);
    return EXIT_SUCCESS;
}