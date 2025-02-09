#pragma once
#include <vector>
#include <string>
#include "location.hpp"

namespace err {

class ErrorHandler final {
    ErrorHandler() = default;
    ~ErrorHandler() = default;
public:
    static ErrorHandler &QueryErrorHandler() {
        static ErrorHandler error_handler{};
        return error_handler;
    }

    std::string GetFullErrorMessage(std::string error_name, \
                                    std::string error_mes, \
                                    const yy::Location &loc) const { 
        auto mes = error_name + ": " + error_mes + ", at line #" + std::to_string(loc.GetStartLine()) + ":\n"; 
        mes += (loc.GetCodeLine(loc.GetStartLine() - 1) + "\n");
        mes += std::string(loc.GetStartColumn() - 1, ' ');
        mes += std::string(loc.GetEndColumn() - loc.GetStartColumn(), '^');

        return mes;
    }
};
} // namespace err