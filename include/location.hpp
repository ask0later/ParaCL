#pragma once
#include <fstream>

namespace yy {
    struct Location final {
        struct Position final {
            int line = 1;
            int column = 1;
        }; // struct Position
    public:
        Location() {}
        Location(std::string &file_name) {
            std::ifstream file(file_name);
            if (!file.is_open()) {
                throw std::invalid_argument("Can't open file");
            }

            std::string line;
            while (std::getline(file, line))
                code_lines_.push_back(line);
        
            file.close();
        }

        void Step() {
            begin = end;
        }

        void Columns(int count = 1) {
            end.column += count;
        }

        void Lines(int count = 1) {
            end.line += count;
        }
        
        const std::string &GetCodeLine(size_t line_num) const {
            return code_lines_[line_num];
        }

        Position begin;
        Position end;
    private:
        inline static std::vector<std::string> code_lines_{};
    }; // class Location
};