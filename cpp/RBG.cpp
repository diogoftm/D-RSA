#include <iostream>
#include <stdexcept>
#include "generator.h"


const static char* usage = "usage: ./RBG password confusionString iterationCount";

void getGeneratorArgsFromArgs(GeneratorArgs &args, int argc, char *argv[])
{
    if(argc != 4)
        throw std::invalid_argument("Expected three arguments");

    std::string PW = std::string(argv[1]);
    std::string CS = std::string(argv[2]);
    int IC = std::stoi(argv[3]);

    args.PW = PW;
    args.CS = CS;
    args.IC = IC;
}



int main(int argc, char *argv[])
{

    GeneratorArgs args;
    uint8_t block[1024];
    
    try {
        getGeneratorArgsFromArgs(args, argc, argv);
    } catch(std::exception& exception) {

        std::cerr << exception.what() << "\n" << usage << "\n";
        return EXIT_FAILURE;
    }

    Generator generator = Generator(args);
    generator.setup();

    while(true) {
        generator.nextBlock(block, sizeof(block));
        fwrite(block, sizeof(uint8_t), sizeof(block), stdout);
    }

}