import sys
import json
from jsonschema import validate
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
    minPatterBytes = config["plots"]["minPatternBytes"]
    maxPatterBytes = config["plots"]["maxPatternBytes"]
    numSteps = config["plots"]["numSteps"]

    if config["implementation"]["cpp"] == True:
        datasets["cpp"] = {
            "iterations" : [],
            "time" : [],
            "patternBytes" : [],
        }

        currentStep = 0

        while currentStep < numSteps:
            citer = findCurrentIteration(minIterations,maxIterations,currentStep,numSteps)

            for nPatternBytes in range(minPatterBytes, maxPatterBytes+1):
                command = f"./../cpp/RBG PW CS {citer} --limit 1 --patternBytes {nPatternBytes}"
                with open('/dev/null', 'w') as devnull:
                    start_time = monotonic()
                    subprocess.run(
                        command,
                        shell=True,
                        stdout=devnull
                    )
                    end_time = monotonic()

                diff = (end_time - start_time)

                datasets["cpp"]["iterations"].append(citer)
                datasets["cpp"]["time"].append(diff)
                datasets["cpp"]["patternBytes"].append(nPatternBytes)

            currentStep += 1
            print(f"cpp {currentStep}/{numSteps} data generated")

    if config["implementation"]["go"] == True:
        datasets["go"] = {
            "iterations" : [],
            "time" : [],
            "patternBytes" : [],
        }

        currentStep = 0

        while currentStep < numSteps:
            for nPatternBytes in range(minPatterBytes, maxPatterBytes+1):
                citer = findCurrentIteration(minIterations,maxIterations,currentStep,numSteps)
                command = f"./../go/generator/generator PW CS {citer} --limit 1 --patternBytes {nPatternBytes}"
                with open('/dev/null', 'w') as devnull:
                    start_time = monotonic()
                    subprocess.run(
                        command,
                        shell=True,
                        stdout=devnull
                    )
                    end_time = monotonic()

                diff = (end_time - start_time)

                datasets["go"]["iterations"].append(citer)
                datasets["go"]["time"].append(diff)
                datasets["go"]["patternBytes"].append(nPatternBytes)

            currentStep += 1
            print(f"go {currentStep}/{numSteps} data generated")

    return datasets, minPatterBytes, maxPatterBytes



def generatePlotsFromDatasets(datasets, minPatternBytes, maxPatternBytes):
    fig, axs = plt.subplots(1, 2, figsize=(12, 6))  # Creating two subplots side by side

    for patternBytes in range(minPatternBytes, maxPatternBytes + 1):
        if datasets["cpp"] is not None:
            cppDataset = datasets["cpp"]
            axs[0].plot(
                [
                    cppDataset["iterations"][i]
                    for i in range(len(cppDataset["iterations"]))
                    if cppDataset["patternBytes"][i] == patternBytes
                ],
                [
                    cppDataset["time"][i]
                    for i in range(len(cppDataset["time"]))
                    if cppDataset["patternBytes"][i] == patternBytes
                ],
                label=f"{patternBytes} pattern bytes",
                marker="o",
            )

        if datasets["go"] is not None:
            goDataset = datasets["go"]
            axs[1].plot(
                [
                    goDataset["iterations"][i]
                    for i in range(len(goDataset["iterations"]))
                    if goDataset["patternBytes"][i] == patternBytes
                ],
                [
                    goDataset["time"][i]
                    for i in range(len(goDataset["time"]))
                    if goDataset["patternBytes"][i] == patternBytes
                ],
                label=f"{patternBytes} pattern bytes",
                marker="o",
            )

    axs[0].set_xlabel("Number of iterations")
    axs[0].set_ylabel("Time to Setup")
    axs[0].set_title("C++ Implementation")
    axs[0].legend()

    axs[1].set_xlabel("Number of iterations")
    axs[1].set_ylabel("Time to Setup")
    axs[1].set_title("Go Implementation")
    axs[1].legend()

    plt.tight_layout()
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
    
    dataset, minPatternBytes, maxPatternBytes = collectDataFromConfig(config)
    generatePlotsFromDatasets(
        dataset, minPatternBytes, maxPatternBytes
    )


if __name__ == "__main__":
    main(sys.argv)