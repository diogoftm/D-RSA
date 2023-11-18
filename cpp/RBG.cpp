#include <iostream>
#include <stdexcept>
#include "generator.h"
#include <optional>
#include <cstring>
#include <string>

#define MIN(A, B) A > B ? B : A

const static char *usage = "usage: ./RBG password confusionString iterationCount [--limit nbytes] [--patternBytes nbytes]";

struct OptionalArguments
{
    std::optional<int> limit;
    std::optional<int> patternBytes;
};


void produceDataUntilLimit(Generator &generator, std::optional<int> limit)
{
    uint8_t block[1024];
    int bytesWritten = 0, bytesToWrite = 0, missingBytes = 0;

    while (!limit.has_value() || bytesWritten < limit.value())
    {
        generator.nextBlock(block, sizeof(block));

        if (!limit.has_value())
        {
            bytesToWrite = sizeof(block);
        }
        else
        {
            missingBytes = limit.value() - bytesWritten;
            bytesToWrite = MIN((int)sizeof(block), missingBytes);
        }

        fwrite(block, sizeof(uint8_t), bytesToWrite, stdout);
        bytesWritten += bytesToWrite;
    }
}


void parseArgs(int argc, char *argv[], GeneratorArgs &args, OptionalArguments &optionalArgs)
{
    if (argc < 4)
        throw std::invalid_argument("Not enough arguments");

    args.PW = argv[1];
    args.CS = argv[2];

    if (std::stoi(argv[3]) < 1)
        throw std::invalid_argument("Invalid IC value");

    args.IC = std::stoi(argv[3]);

    for (int i = 4; i < argc; i++)
    {
        if (strcmp(argv[i], "--limit") == 0)
        {
            if (i + 1 >= argc)
                throw std::invalid_argument("Missing argument for --limit");


            if (std::stoi(argv[i + 1]) < 0)
                throw std::invalid_argument("Invalid limit value");
            optionalArgs.limit = std::stoi(argv[i + 1]);
            i++;
        }
        else if (strcmp(argv[i], "--patternBytes") == 0)
        {
            if (i + 1 >= argc)
                throw std::invalid_argument("Missing argument for --patternBytes");

            if (std::stoi(argv[i + 1]) < 1)
                throw std::invalid_argument("Invalid patternBytes value");
            
            optionalArgs.patternBytes = std::stoi(argv[i + 1]);
            i++;
        }
        else
        {
            throw std::invalid_argument("Invalid argument");
        }
    }

    if(optionalArgs.patternBytes.has_value()) {
        args.patternBytes = optionalArgs.patternBytes.value();
    } else {
        args.patternBytes = PatternBytes;
    }
}


int main(int argc, char *argv[])
{

    GeneratorArgs args;
    OptionalArguments optionalArgs;

    try
    {
        parseArgs(argc, argv, args, optionalArgs);
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