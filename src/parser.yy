/* ------------------------------------------------------------------------- **
 *
 *  Grammar of compiler 
 *
 *  StatementList -> Statement StatementList | Empty
 *  Statement -> While | If | Assigment;
 *  Assigment -> Variable = Expression
 *
 *
 *  Expression -> Summand Operator Expression | Summand
 *  Summand -> Multiplier PriorityOperator Summand | Multiplier
 *  Multiplier -> (Terminals) | Terminals
 *  Terminals -> Number | Variable
 *
 *
 *  While -> (Expression CompareOp Expression) { StatementList }
 *  If -> (Expression CompareOp Expression) { StatementList }
 *
 *  CompareOp -> Equal | NotEqual | Less | Greater | EqualOrLess | EqualOrGreater
 *  Operator -> + | -
 *  PriorityOperator -> * | /
 * ------------------------------------------------------------------------- */

%language "c++"

%skeleton "lalr1.cc"
%defines
%define api.value.type variant
%param {yy::NumDriver* driver}

%code requires
{

// forward decl of argument to parser
namespace yy { class NumDriver; }
}

%code
{
#include "driver.hpp"

namespace yy {
parser::token_type yylex(parser::semantic_type* yylval,                         
                         NumDriver* driver);
}
}

%token
/* Binary opetators */
    ADD
    SUB
    MULT
    DIV

/* Brackets and separetors */
    LBRAC
    RBRAC
    LCURBRAC
    RCURBRAC
    SEMICOLON

/* Statements */
    IF
    ELSE
    WHILE
    ASSIGMENT

/* I/O */
    INPUT
    OUTPUT

/* Compare binary opetators */
    EQUAL
    NOT_EQUAL
    GREATER
    LESS
    GREATER_OR_EQUAL
    LESS_OR_EQUAL

/* Error token */
    ERR
;

%token <int> NUMBER
%token <int> NAME

%start program

%%

program: StatementList
;

StatementList: Statement StatementList {std::cout << "" << std::endl; }
              | %empty {std::cout << "Empty" << std::endl; }
;

Statement: Output SEMICOLON {std::cout << ";" << std::endl;}
        |  Condition
        |  Loop
        |  Assigment SEMICOLON {std::cout << ";" << std::endl;}
;

Output: OUTPUT Expression {std::cout << "OUTPUT" << std::endl;}
;

Condition: IF LBRAC Predicat RBRAC LCURBRAC StatementList RCURBRAC
;

Loop: WHILE LBRAC Predicat RBRAC LCURBRAC StatementList RCURBRAC
;

Predicat: Expression CompareOpetators Expression 
;

CompareOpetators: EQUAL
                | NOT_EQUAL
                | GREATER
                | LESS
                | GREATER_OR_EQUAL
                | LESS_OR_EQUAL
;

Assigment: NAME ASSIGMENT Expression {std::cout << "NAME =" << std::endl; }
        |  NAME ASSIGMENT INPUT {std::cout << "NAME = INPUT" << std::endl;}
;

Expression: Expression ADD Summand {std::cout << "ADD Summand" << std::endl;}
          | Expression SUB Summand {std::cout << "SUB Summand" << std::endl;}
          | Summand {std::cout << "Summand" <<std::endl;}
;

Summand:  Summand MULT Multiplier {std::cout << " MULT Multiplier" << std::endl;}
        | Summand DIV Multiplier {std::cout << " DIV Multiplier" << std::endl;}
        | Multiplier {std::cout << "Multiplier" << std::endl;}
;

Multiplier: LBRAC Expression RBRAC {std::cout << "" << std::endl;}
          | Terminals {std::cout << "Terminals" << std::endl;}
;

Terminals: NUMBER {std::cout << "Number" << std::endl;}
         | NAME {std::cout << "Name" << std::endl;}
;

%%

namespace yy {

parser::token_type yylex(parser::semantic_type* yylval,                         
                         NumDriver* driver)
{
  return driver->yylex(yylval);
}

void parser::error(const std::string&){}
}