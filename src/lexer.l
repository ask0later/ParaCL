%option c++
%option yylineno noyywrap
%option yyclass="yy::Lexer"

%{
#include "parser.tab.hh"
#include "lexer.hpp"
#define YY_USER_ACTION UpdateTokenPosition();
%}

WS          [ \t\v]+
NL          [\n]+
NAME        [a-zA-Z][a-zA-Z0-9_]*
NUMBER      [1-9][0-9]*|0

START_COMMENT   "/*"
END_COMMENT     "*/"
COMMENT "/*"(.*{NL})*.*"*/"|"//".*

%%
{NUMBER}{NAME}      { return yy::parser::token_type::ERR; }

{WS}                { /* White spaces
                         skip blanks and tabs */ }
"\n"                { /* skip new line */ 
                      UpdateLine(); }

{COMMENT}           { /* skip text as a comment */ }

"if"                { return yy::parser::token_type::IF;    }    
"else"              { return yy::parser::token_type::ELSE;  }
"while"             { return yy::parser::token_type::WHILE; }
"?"                 { return yy::parser::token_type::INPUT; }
"print"             { return yy::parser::token_type::OUTPUT; }

{NAME}              { return yy::parser::token_type::NAME; }
{NUMBER}            { return yy::parser::token_type::NUMBER; }

"&&"                { /* Logic operators */
                      return yy::parser::token_type::LOGIC_AND; }
"||"                { return yy::parser::token_type::LOGIC_OR; }

"+"                 { /* Binary operators */
                      return yy::parser::token_type::ADD;       }
"-"                 { return yy::parser::token_type::MINUS;     }
"*"                 { return yy::parser::token_type::MULT;      }
"/"                 { return yy::parser::token_type::DIV;       }
"%"                 { return yy::parser::token_type::REMAINDER; }


"("                 { /* Brackets and separetors */
                      return yy::parser::token_type::LBRAC; }
")"                 { return yy::parser::token_type::RBRAC; }
"{"                 { return yy::parser::token_type::LCURBRAC;  }
"}"                 { return yy::parser::token_type::RCURBRAC;  }
";"                 { return yy::parser::token_type::SEMICOLON; }

"=="                { /* Compare binary opetators */
                      return yy::parser::token_type::EQUAL;     }
"!="                { return yy::parser::token_type::NOT_EQUAL; }
">"                 { return yy::parser::token_type::GREATER;   }
"<"                 { return yy::parser::token_type::LESS;      }
">="                { return yy::parser::token_type::GREATER_OR_EQUAL;  }
"<="                { return yy::parser::token_type::LESS_OR_EQUAL;     }

"!"                 { /* Compare unary opetators */
                      return yy::parser::token_type::NEGATION;  } 

"="                 { return yy::parser::token_type::ASSIGMENT; }

.                   { return yy::parser::token_type::ERR; }
%%

// nothing