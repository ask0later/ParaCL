#pragma once
#include <vector>
#include <utility>

#include "error_handler.hpp"
#include "lexer.hpp"
#include "executer.hpp"
#include "drawer.hpp"
#include "parser.tab.hh"

namespace yy {

class Driver final {
    Driver(const std::string_view file_name) : file_name_(file_name), 
                                         lex_(yy::Lexer::QueryLexer(file_name)),
                                         err_handler_(err::ErrorHandler::QueryErrorHandler()) {}
    ~Driver() {}
public:
    static Driver &QueryDriver(const std::string_view file_name) {
        static Driver driver{file_name};
        return driver;
    }

    const char *GetCurrentTokenText() const {
        return lex_.YYText();
    }

    size_t GetCurrentLineNumber() const {
        return size_t(lex_.lineno());
    }

    template <typename T, typename... Args>
    T *GetNode(Args&&... args) {
        return builder_.template GetObj<T>(std::forward<Args>(args)...);
    }

    const Location &GetLocation() const {
        return lex_.GetLocation();
    }

    parser::token_type yylex(parser::semantic_type *yylval, Location *loc) {
        parser::token_type tt = static_cast<parser::token_type>(lex_.yylex());

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
                                    GetLocation()));
        }

        *loc = lex_.GetLocation();
        
        return tt;
    }

    bool Parse() {
        std::ifstream input_file(file_name_.data());
        lex_.switch_streams(input_file, std::cout);
        
        bool res = false; 
        try {
            parser parser(this);
            res = parser.parse();
        } catch (std::runtime_error &ex) {
            throw std::runtime_error(err_handler_.GetFullErrorMessage("Syntax error", \
                                                                        ex.what(), \
                                                                        GetLocation()));
        }

        return !res;
    }

    void SetRootNode(node::Node *root) {
        root_ = root;
    }

    node::Node *GetRootNode() const {
        return root_;
    }

    void Execute() const {
        executer::ExecuteVisitor executer(err_handler_);
        root_->Accept(executer);
    }

    void DrawAST() const {
        dotter::Dotter dotter;
        drawer::DrawVisitor drawer(dotter);
        root_->Accept(drawer);
        dotter.PrintDotText();
        dotter.Render();
    }

private:
    yy::Lexer &lex_;
    node::Node *root_ = nullptr;
    node::details::Builder<node::Node> builder_;
    const std::string_view file_name_;
    err::ErrorHandler &err_handler_;
};
} // namespace yy