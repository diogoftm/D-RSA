#include <gtest/gtest.h>
#include <cstdlib>


double bitFlipPercentage(std::string& s1, std::string& s2) {

    if(s1.length() != s2.length()) {
        throw std::invalid_argument("s1.length() != s2.length()");
    }

    unsigned int length = s1.length();
    int count = 0;
    uint8_t b1,b2;
    for(unsigned int i=0;i<length;i++) {
        b1 = (uint8_t)s1.at(i);
        b2 = (uint8_t)s2.at(i);

        for(unsigned int j=0;j<8;j++) {

            uint8_t bit1 = (b1 >> j) & 0x01;
            uint8_t bit2 = (b2 >> j) & 0x01;

            if(bit1 != bit2)
                count++;

        }
    }

    return (100 * count) / (length * 8.0);
}

std::string getStdoutFromCommand(const char* command, unsigned int maxBytes) {

    FILE* pipe = popen(command, "r");
    if (!pipe) {
        throw std::runtime_error("Unable to open pipe from command " + std::string(command) + "\n");
    }

    char buffer[128];
    std::string result = "";
    size_t bytesRead = 0;

    while (bytesRead < maxBytes) {
        result += buffer;
        bytesRead += strlen(buffer);
    }

    int status = pclose(pipe);
    if (status == -1) {
        throw std::runtime_error("Error closing pipe\n");
    }

    return result;
}

TEST(RBG_Avalanche, Password) {
    const char* osCall_1 = "./RBG ABCDEF1 CS 50 --limit 100";
    const char* osCall_2 = "./RBG ABCDEF2 CS 51 --limit 100";

    std::string s1 = getStdoutFromCommand(osCall_1, 100);
    std::string s2 = getStdoutFromCommand(osCall_2, 100);

    
    EXPECT_EQ(s1.length(), s2.length());
    EXPECT_EQ(s1.length(), 100);

    double fp1 = bitFlipPercentage(s1,s2);
    ASSERT_NEAR(fp1, 50.0, 1.0);
}

/*TEST(RBG_Avalanche, ConfusionString) {
    const char* osCall_1 = "./RBG PW CS123 45 --limit 100";
    const char* osCall_2 = "./RBG PW CS113 45 --limit 100";

    std::string s1 = getStdoutFromCommand(osCall_1, 100);
    std::string s2 = getStdoutFromCommand(osCall_2, 100);

    EXPECT_EQ(s1.length(), s2.length());

    double fp1 = bitFlipPercentage(s1,s2);

    ASSERT_NEAR(fp1, 50, 1);
}

TEST(RBG_Avalanche, IterationCount) {
    const char* osCall_1 = "./RBG PW CS 60 --limit 100";
    const char* osCall_2 = "./RBG PW CS 61 --limit 100";

    std::string s1 = getStdoutFromCommand(osCall_1, 100);
    std::string s2 = getStdoutFromCommand(osCall_2, 100);

    EXPECT_EQ(s1.length(), s2.length());

    double fp1 = bitFlipPercentage(s1,s2);

    ASSERT_NEAR(fp1, 50, 1);
}

TEST(RBG_Determinist, Determinism) {
    const char* osCall = "./RBG PW CS 50 --limit 100";

    std::string s1 = getStdoutFromCommand(osCall, 100);
    std::string s2 = getStdoutFromCommand(osCall, 100);

    EXPECT_EQ(s1.length(), s2.length());

    EXPECT_EQ(s1.compare(s2), 0);
}
*/

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}