#pragma once
#include <FlexLexer.h>
#include <vector>
#include "executer.hpp"
#include "drawer.hpp"
#include "parser.tab.hh"

namespace yy {

class Driver final {
public:
    Driver(FlexLexer *plex) : plex_(plex) {}
    ~Driver() {
        builder_.Clean();
    }

    parser::token_type yylex(parser::semantic_type *yylval) {
        parser::token_type tt = static_cast<parser::token_type>(plex_->yylex());

        if (tt == yy::parser::token_type::NUMBER) {
            yylval->as<int>() = std::stoi(GetCurrentTokenText());
        }

        if (tt == yy::parser::token_type::NAME) {
            parser::semantic_type tmp;
            tmp.as<std::string>() = GetCurrentTokenText();
            yylval->swap<std::string>(tmp);
        }

        if (tt == yy::parser::token_type::ERR) {
            int num_line = GetCurrentLineNumber();
            std::string error_mes("Lexical error, unrecoginzed lexem: '");
            error_mes += GetCurrentTokenText();
            error_mes += "' at line #";
            error_mes += std::to_string(num_line);
            error_mes += ":\n";
            error_mes += program_text_[num_line - 1];
            throw std::logic_error(error_mes);
        }

        return tt;
    }

    bool parse(std::string &file_name) {
        ReadFileToStrings(file_name);
        std::ifstream input_file(file_name);
        plex_->switch_streams(input_file, std::cout);

        parser parser(this);
        bool res = parser.parse();
        return !res;
    }

    void SetRootNode(node::Node *root) {
        root_ = root;
    }

    node::Node *GetRootNode() const {
        return root_;
    }

    void Execute() {
        executer::ExecuteVisitor executer;
        try {
            root_->Accept(executer);
        } catch (std::runtime_error &run_time_ex) {
            std::string error_mes = "Runtime error: ";
            error_mes += run_time_ex.what();
            error_mes += " at line #";
            error_mes += std::to_string(GetCurrentLineNumber());
            throw std::runtime_error(error_mes);
        }
    }

    void DrawAST() {
        dotter::Dotter dotter;
        drawer::DrawVisitor drawer(dotter);
        root_->Accept(drawer);
        dotter.PrintDotText();
        dotter.Render();
    }

    template <typename T, typename... Args>
    T *GetNode(Args... args) {
        return builder_.template GetObj<T>(args...);
    }

    const char *GetCurrentTokenText() const {
        return plex_->YYText();
    }

    int GetCurrentLineNumber() const {
        return plex_->lineno();
    }

    std::vector<std::string> program_text_;
private:
    void ReadFileToStrings(std::string &file_name) {
        std::ifstream file(file_name);
        if (!file.is_open())
        {
            throw std::invalid_argument("Can't open file");
        }

        std::string line;
        while (std::getline(file, line))
            program_text_.push_back(line);
        
        file.close();
    }

    FlexLexer *plex_;
    node::Node *root_;
    node::details::Builder<node::Node> builder_;
};
} // namespace yy