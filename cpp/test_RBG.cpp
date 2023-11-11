#include <gtest/gtest.h>
#include <cstdlib>

double bitFlipPercentage(const std::vector<uint8_t> &s1, const std::vector<uint8_t> &s2)
{
    if (s1.size() != s2.size())
    {
        throw std::invalid_argument("s1.size() != s2.size()");
    }

    size_t length = s1.size();
    int count = 0;

    for (size_t i = 0; i < length; i++)
    {
        uint8_t b1 = s1[i];
        uint8_t b2 = s2[i];

        for (unsigned int j = 0; j < 8; j++)
        {
            uint8_t bit1 = (b1 >> j) & 0x01;
            uint8_t bit2 = (b2 >> j) & 0x01;

            if (bit1 != bit2)
                count++;
        }
    }

    return (100 * count) / (length * 8.0);
}

std::vector<uint8_t> getStdoutBytesFromCommand(const char *command, unsigned int maxBytes)
{
    FILE *pipe = popen(command, "r");
    if (!pipe)
    {
        throw std::runtime_error("Unable to open pipe from command " + std::string(command) + "\n");
    }

    const size_t bufferSize = 128;
    char buffer[bufferSize];
    std::vector<uint8_t> result;
    size_t bytesRead = 0;

    while (bytesRead < maxBytes)
    {
        size_t toRead = std::min(maxBytes - bytesRead, bufferSize);

        size_t bytesReadThisIteration = fread(buffer, 1, toRead, pipe);

        if (bytesReadThisIteration == 0)
        {
            break;
        }

        result.insert(result.end(), buffer, buffer + bytesReadThisIteration);

        bytesRead += bytesReadThisIteration;
    }

    int status = pclose(pipe);
    if (status == -1)
    {
        throw std::runtime_error("Error closing pipe\n");
    }

    if (bytesRead < maxBytes)
    {
        throw std::runtime_error("Read less bytes than specified in maxBytes\n");
    }

    return result;
}

bool checkCommand(const char *command, int expectedExitCode)
{
    if (freopen("/dev/null", "w", stdout) == nullptr)
    {
        throw std::runtime_error("Failed to redirect stdout to /dev/null.");
    }

    if (freopen("/dev/null", "w", stderr) == nullptr)
    {
        throw std::runtime_error("Failed to redirect stderr to /dev/null.");
    }

    int returnValue = std::system(command);

    if (freopen("/dev/tty", "w", stdout) == nullptr)
    {
        throw std::runtime_error("Failed to restore stdout.");
    }

    if (freopen("/dev/tty", "w", stderr) == nullptr)
    {
        throw std::runtime_error("Failed to restore stderr.");
    }

    if (WIFEXITED(returnValue))
    {
        int exitCode = WEXITSTATUS(returnValue);
        return exitCode == expectedExitCode;
    }
    else
    {
        throw std::runtime_error("Command did not exit normally.");
    }
}

void runWithTimeout()
{
}

TEST(RBG_Avalanche, Password)
{
    const char *osCall_1 = "./RBG ABCDEF12 CS 50 --limit 1024";
    const char *osCall_2 = "./RBG ABCDEF13 CS 50 --limit 1024";

    std::vector<uint8_t> s1Bytes = getStdoutBytesFromCommand(osCall_1, 1024);
    std::vector<uint8_t> s2Bytes = getStdoutBytesFromCommand(osCall_2, 1024);

    ASSERT_EQ(s1Bytes.size(), s2Bytes.size());
    ASSERT_EQ(s2Bytes.size(), 1024);

    double fp1 = bitFlipPercentage(s1Bytes, s2Bytes);
    ASSERT_NEAR(fp1, 50.0, 1.0);
}

TEST(RBG_Avalanche, ConfusionString)
{
    const char *osCall_1 = "./RBG PW CS12 50 --limit 1024";
    const char *osCall_2 = "./RBG PW CS13 50 --limit 1024";

    std::vector<uint8_t> s1Bytes = getStdoutBytesFromCommand(osCall_1, 1024);
    std::vector<uint8_t> s2Bytes = getStdoutBytesFromCommand(osCall_2, 1024);

    ASSERT_EQ(s1Bytes.size(), s2Bytes.size());
    ASSERT_EQ(s2Bytes.size(), 1024);

    double fp1 = bitFlipPercentage(s1Bytes, s2Bytes);
    ASSERT_NEAR(fp1, 50.0, 1.0);
}

TEST(RBG_Avalanche, IterationCount)
{
    const char *osCall_1 = "./RBG PWW CS 50 --limit 1024";
    const char *osCall_2 = "./RBG PWW CS 51 --limit 1024";

    std::vector<uint8_t> s1Bytes = getStdoutBytesFromCommand(osCall_1, 1024);
    std::vector<uint8_t> s2Bytes = getStdoutBytesFromCommand(osCall_2, 1024);

    ASSERT_EQ(s1Bytes.size(), s2Bytes.size());
    ASSERT_EQ(s2Bytes.size(), 1024);

    double fp1 = bitFlipPercentage(s1Bytes, s2Bytes);
    ASSERT_NEAR(fp1, 50.0, 1.0);
}

TEST(RBG_Determinism, EqualArguments)
{
    const char *osCall = "./RBG PWW CS 50 --limit 1024";

    std::vector<uint8_t> s1Bytes = getStdoutBytesFromCommand(osCall, 1024);
    std::vector<uint8_t> s2Bytes = getStdoutBytesFromCommand(osCall, 1024);

    ASSERT_TRUE(s1Bytes == s2Bytes);
}

TEST(RBG_ExitCodes, InvalidArguments)
{
    std::vector<const char *> badCommands = {
        "./RBG",
        "./RBG PW CS",
        "./RBG PW CS X",
        "./RBG PW CS -1",
        "./RBG PW CS 5 --limit",
        "./RBG PW CS 5 --limit -1",
        "./RBG PW CS 5 --limit X",
        "./RBG PW CS 5 --limit 1 2 3",
    };

    for (const char *command : badCommands)
    {
        ASSERT_TRUE(checkCommand(command, EXIT_FAILURE));
    }
}

TEST(RBG_ExitCodes, GoodArguments)
{
    std::vector<const char *> goodCommands = {
        "./RBG PW CS 50 --limit 150",
        "./RBG MWEQM CS 50 --limit 9025",
        "./RBG 3J029J3091J4302 JD0329N4F34GF8427GF8427G4Q2G8952G09 101 --limit 1"};

    for (const char *command : goodCommands)
    {
        ASSERT_TRUE(checkCommand(command, EXIT_SUCCESS));
    }
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}