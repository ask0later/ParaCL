#pragma once
#include <fstream>
#include "location.hh"

namespace yy {
    class Location final : public location {
    public:
        Location(std::string &file_name) : location() {
            std::ifstream file(file_name);
            if (!file.is_open()) {
                throw std::invalid_argument("Can't open file");
            }

            std::string line;
            while (std::getline(file, line))
                code_lines_.push_back(line);
        
            file.close();
        }

        int GetStartLine() const {
            return begin.line;
        }

        int GetStartColumn() const {
            return begin.column;
        }

        int GetEndLine() const {
            return end.line;
        }

        int GetEndColumn() const {
            return end.column;
        }
        
        const std::string &GetCodeLine(size_t line_num) const {
            return code_lines_[line_num];
        }
    private:
        inline static std::vector<std::string> code_lines_{};
    }; // class Location
};