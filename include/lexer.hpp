#pragma once

#include "location.hpp"

#ifndef yyFlexLexer
#include <FlexLexer.h>
#endif

namespace yy {
    class Lexer final : public yyFlexLexer {
        Lexer(std::string &file_name) : yyFlexLexer(), loc_(file_name) {}
    public:
        static Lexer &QueryLexer(std::string &file_name) {
            static Lexer lexer{file_name};
            return lexer;
        }
        int yylex() override;

        // here we can return non-zero if lexing is not done inspite of EOF detected
        int yywrap() override { return 1; }

        void UpdateTokenPosition() {
            loc_.step();
            loc_.columns(yyleng);
        }

        void UpdateLine() {
            loc_.lines(yyleng);
            loc_.step();
        }

        const Location &GetLocation() const {
            return loc_;
        }
    private:
        Location loc_;
    }; // class Lexer
}