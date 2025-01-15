#pragma once

#include "node.hpp"
#include "parser.tab.hh"
#include <FlexLexer.h>

namespace yy {

class Driver {
public:
    Driver(FlexLexer *plex) : plex_(plex) {}

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
        root_->Execute();
    }

private:
    FlexLexer *plex_;
    node::Node *root_;
};

} // namespace yy