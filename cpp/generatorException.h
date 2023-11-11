#pragma once

#include <stdexcept>

enum GeneratorExceptionTypes {
    GENERATOR_SETUP_ERROR,
    GENERATOR_RUNTIME_ERROR
};

std::string generatorExceptionTypesRepr(GeneratorExceptionTypes type) {
    switch(type) {
        case GENERATOR_SETUP_ERROR: return "Generator Setup Error";
        case GENERATOR_RUNTIME_ERROR: return "Generator Runtime Error";
    }

    return "";
}

class GeneratorException : public std::exception {
    public:
        GeneratorException(const std::string& message, GeneratorExceptionTypes type) : message(message) {}

        const char* what() const noexcept override {
            return ("(" + generatorExceptionTypesRepr(this->type) + ")\n" + message).c_str(); 
        }

    private:
        std::string message;
        GeneratorExceptionTypes type;
};