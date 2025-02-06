#pragma once
#include <vector>
#include <string>

namespace err {

class ErrorHandler final {
    ErrorHandler() = default;
    ~ErrorHandler() = default;
public:
    static ErrorHandler &QueryErrorHandler() {
        static ErrorHandler error_handler{};
        return error_handler;
    }

    std::string GetFullErrorMessage(std::string error_name, std::string error_mes, size_t line) {
        return std::string(error_name + ": " + error_mes + ", at line #" + std::to_string(line) + ":\n" + strings_[line - 1]);
    }

    std::vector<std::string> &GetStrings() {
        return strings_;
    }

private:
    std::vector<std::string> strings_;
};
} // namespace err