#pragma once

#include "location.hpp"

#ifndef yyFlexLexer
#include <FlexLexer.h>
#endif

namespace yy {
    class Lexer final : public yyFlexLexer {
        Lexer(const std::string_view file_name) : yyFlexLexer(), loc_(file_name) {}
    public:
        static Lexer &QueryLexer(const std::string_view file_name) {
            static Lexer lexer{file_name};
            return lexer;
        }
        int yylex() override;

        // here we can return non-zero if lexing is not done inspite of EOF detected
        int yywrap() override { return 1; }

        void UpdateTokenPosition() {
            loc_.Step();
            loc_.Columns(yyleng);
        }

        void UpdateLine() {
            loc_.Lines(yyleng);
            loc_.end.column = 1;
            loc_.Step();
        }

        void UpdateComment() {
            for (size_t i = 0; i < yyleng; ++i) {
                if (yytext[i] == '\n') {
                    loc_.Lines(1);
                }
            }
        }

        const Location &GetLocation() const {
            return loc_;
        }
    private:
        Location loc_;
    }; // class Lexer
}