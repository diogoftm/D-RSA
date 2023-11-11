import matplotlib
import sys
import json
from jsonschema import validate, ValidationError
from dataclasses import dataclass
import matplotlib.pyplot as plt
from time import monotonic
import subprocess


def printUsage():
    print(
        "usage: python3 randgen.py confPath"
    )

def getConfigByArgs(argv):

    config_data = None

    with open(argv[1], 'r') as config_file:
        config_data = json.load(config_file)

    if not config_data:
        raise "Unable to read data from config file"
    
    expectedSchema = {
        "type": "object",
        "properties": {
            "plots": {
                "type": "object",
                "properties": {
                    "minIterations": {"type": "integer", "minimum": 1},
                    "maxIterations": {"type": "integer", "minimum": 1},
                    "numSteps": {"type": "integer"}
                },
                "required": ["minIterations", "maxIterations", "numSteps"]
            },
            "implementation": {
                "type": "object",
                "properties": {
                    "cpp": {"type": "boolean"},
                    "go": {"type": "boolean"}
                },
                "required": ["cpp", "go"]
            }
        },
        "required": ["plots", "implementation"]
    }

    validate(instance=config_data, schema=expectedSchema)

    return config_data





def collectDataFromConfig(config):

    datasets = {"cpp": None, "go": None}

    findCurrentIteration = lambda minI, maxI, currS, numS: (
        int(min(config["plots"]["maxIterations"],
            max(config["plots"]["minIterations"],
                minI + (maxI - minI) * currS / numS)
        ))
    )

    minIterations = config["plots"]["minIterations"]
    maxIterations = config["plots"]["maxIterations"]
    numSteps = config["plots"]["numSteps"]

    currentStep = 0

    if config["implementation"]["cpp"] == True:
        datasets["cpp"] = {
            "iterations" : [],
            "time" : [],
        }
        citer = findCurrentIteration(minIterations,maxIterations,currentStep,numSteps)
        command = f"./cpp/RBG PW CS {citer} --limit 1"

        start_time = monotonic()

        subprocess.run(
            command,
            shell=True
        )

        end_time = monotonic()

        diff = (end_time - start_time)

        datasets["cpp"]["iterations"].append(citer)
        datasets["cpp"]["time"].append(diff)

        currentStep += 1


    return datasets



def generatePlotsFromDatasets(datasets):

    plt.plot()

    if datasets["cpp"] is not None:
        cppDataset = datasets["cpp"]
        plt.plot(cppDataset["iterations"], cppDataset["time"], label="cpp implementation", marker='o')

    if datasets["go"] is not None:
        goDataset = datasets["go"]
        plt.plot(goDataset["iterations"], goDataset["time"], label="go implementation", marker='o')


    plt.xlabel("Number of iterations")
    plt.ylabel("Time to Setup")
    plt.title("Time it takes to setup algorithm")

    plt.legend()
    plt.show()



def main(argv):
    if len(argv) != 2:
        printUsage()
        exit(1)

    config = None
    try:
        config = getConfigByArgs(argv)
    except Exception as e:
        print("Unable to parse config file")
        print(e)
    
    generatePlotsFromDatasets(
        collectDataFromConfig(config)
    )


if __name__ == "__main__":
    main(sys.argv)