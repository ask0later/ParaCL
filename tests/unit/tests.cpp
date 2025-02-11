#include <gtest/gtest.h>
#include <vector>
#include <sstream>
#include <fstream>
#include <string>

constexpr size_t RIGHT_TEST_COUNT = 15;
constexpr size_t WRONG_TEST_COUNT = 7;

std::string Execute(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string result;
    
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        throw std::runtime_error("popen() failed");
    }

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    pclose(pipe);
    return result;
}

void RunTest(std::vector<int> &ans, std::vector<int> &res, size_t i, const std::string &test_name) {
    std::string program_file = "tests/end-to-end/" + test_name + "/" + std::to_string(i) + ".paracl";
    std::string answer_file = "tests/end-to-end/" + test_name + "/" + std::to_string(i) + ".ans";
    
    std::ifstream ans_stream(answer_file);
    if (!ans_stream.is_open()) {
        throw std::runtime_error("file is not open");
    }

    int value;
    while (ans_stream >> value) {
        ans.push_back(value);
    }
    ans_stream.close();

    std::string command = "./build/src/Interpretator " + program_file;
    std::string output;
    
    output = Execute(command);
    
    std::istringstream result_stream(output);
    while (result_stream >> value) {
        res.push_back(value);
    }
}

TEST(RightTest, EndtoEndParaCL) {
    for (size_t i = 1; i < RIGHT_TEST_COUNT; ++i) {
        std::string program_file = "tests/end-to-end/right/" + std::to_string(i) + ".paracl";
        std::string answer_file = "tests/end-to-end/right/" + std::to_string(i) + ".ans";
        
        std::ifstream ans_stream(answer_file);
        if (!ans_stream.is_open()) {
            throw std::runtime_error("file is not open");
        }

        std::vector<int> ans;
        int value;
        while (ans_stream >> value) {
            ans.push_back(value);
        }
        ans_stream.close();

        std::string command = "./build/src/Interpretator " + program_file;
        std::string output;
        
        try {
            output = Execute(command);
        } catch (std::exception &ex) {
            std::cout << ex.what() << std::endl;
            FAIL();
        }
        
        std::istringstream result_stream(output);
        std::vector<int> res;
        while (result_stream >> value) {
            res.push_back(value);
        }

        ASSERT_EQ(res.size(), ans.size());
        for (size_t j = 0; j < res.size(); ++j)
            ASSERT_EQ(res[j], ans[j]);
    }
}

TEST(WrongTest, EndtoEndParaCL) {
    for (size_t i = 1; i < WRONG_TEST_COUNT; ++i) {
        std::string program_file = "tests/end-to-end/wrong/" + std::to_string(i) + ".paracl";
        std::string answer_file = "tests/end-to-end/wrong/" + std::to_string(i) + ".ans";
        
        std::ifstream ans_stream(answer_file);
        ASSERT_TRUE(ans_stream.is_open());

        std::vector<std::string> ans;
        std::string line;
        while (std::getline(ans_stream, line)) {
            if (!line.empty())
                ans.push_back(line);
        }
        ans_stream.close();

        std::string command = "./build/src/Interpretator " + program_file;
        std::string output;
        try {
            output = Execute(command);
        } catch (const std::exception& ex) {
            std::cout << ex.what() << std::endl;
            FAIL();
        }

        std::istringstream result_stream(output);
        std::vector<std::string> res;
        while (std::getline(result_stream, line)) {
            if (!line.empty())
                res.push_back(line);
        }

        ASSERT_EQ(res.size(), ans.size());
        
        for (size_t j = 0; j < res.size(); ++j) {
            EXPECT_EQ(res[j], ans[j]);
        }
    }
}