#pragma once
#include <FlexLexer.h>
#include <vector>
#include "executer.hpp"
#include "drawer.hpp"
#include "parser.tab.hh"

namespace yy {

class Driver final {
    Driver(FlexLexer *plex, std::string &file_name) : 
                plex_(plex), file_name_(file_name), err_handler_(err::ErrorHandler::QueryErrorHandler()) {
        ParseFileToStrings(file_name, err_handler_.GetStrings());
    }

    ~Driver() {
        builder_.Clean();
    }
public:
    static Driver &QueryDriver(FlexLexer *plex, std::string &file_name) {
        static Driver driver{plex, file_name};
        return driver;
    }

    const char *GetCurrentTokenText() const {
        return plex_->YYText();
    }

    size_t GetCurrentLineNumber() const {
        return size_t(plex_->lineno());
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
            throw std::logic_error(err_handler_.GetFullErrorMessage("Lexical error, unrecoginzed lexem", \
                                            std::string("'" + std::string(GetCurrentTokenText()) + "'"), \
                                            GetCurrentLineNumber()));
        }

        return tt;
    }

    bool parse() {
        std::ifstream input_file(file_name_);
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
        executer::ExecuteVisitor executer(err_handler_);
        root_->Accept(executer);
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

    std::string GetFullErrorMessage(std::string error_name, std::string error_mes, size_t line) {
        return err_handler_.GetFullErrorMessage(error_name, error_mes, line);
    }

private:
    void ParseFileToStrings(std::string &file_name, std::vector<std::string> &parsed_strings) {
        std::ifstream file(file_name);
        if (!file.is_open()) {
            throw std::invalid_argument("Can't open file");
        }

        std::string line;
        while (std::getline(file, line))
            parsed_strings.push_back(line);
        
        file.close();
    }

    FlexLexer *plex_ = nullptr;
    node::Node *root_ = nullptr;
    node::details::Builder<node::Node> builder_;
    const std::string &file_name_;
    err::ErrorHandler &err_handler_;
};
} // namespace yy