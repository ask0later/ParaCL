#pragma once
#include <FlexLexer.h>
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
            yylval->as<int>() = std::stoi(plex_->YYText());
        }

        if (tt == yy::parser::token_type::NAME) {
            parser::semantic_type tmp;
            tmp.as<std::string>() = plex_->YYText();
            yylval->swap<std::string>(tmp);
        }

        if (tt == yy::parser::token_type::ERR) {
            std::cout << "Lexical error, unrecoginzed lexem" << plex_->YYText() 
                      << "' at line #" << plex_->lineno() << std::endl;
            std::terminate();
        }

        return tt;
    }

    bool parse() {
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

private:
    FlexLexer *plex_;
    node::Node *root_;
    node::details::Builder<node::Node> builder_;
};
} // namespace yy