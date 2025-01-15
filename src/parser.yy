/* ------------------------------------------------------------------------- **
 *
 *  Grammar of compiler 
 *  
 *  Scope -> StatementList 
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
 *  While -> (Expression CompareOp Expression) Scope
 *  If -> (Expression CompareOp Expression) Scope | (Expression CompareOp Expression) Scope Scope 
 *
 *  CompareOp -> Equal | NotEqual | Less | Greater | EqualOrLess | EqualOrGreater
 *  Operator -> + | -
 *  PriorityOperator -> * | /
 * ------------------------------------------------------------------------- */

%language "c++"

%skeleton "lalr1.cc"
%defines
%define api.value.type variant
%param {yy::Driver* driver}

%code requires
{
#include "node.hpp"
// forward decl of argument to parser
namespace yy { class Driver; }
}

%code
{
#include "node.hpp"
#include "driver.hpp"

namespace yy {
parser::token_type yylex(parser::semantic_type* yylval,                         
                         Driver* driver);
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
%token <std::string> NAME

%nterm <node::ScopeNode*> Scope
%nterm <node::ScopeNode*> StatementList
%nterm <node::Node*> Statement

%nterm <node::Node*> Assigment
%nterm <node::Node*> Condition
%nterm <node::ScopeNode*> Else
%nterm <node::Node*> Loop

%nterm <node::ExprNode*> Expression
%nterm <node::ExprNode*> Summand
%nterm <node::ExprNode*> Multiplier
%nterm <node::ExprNode*> Terminals

%nterm <node::BinCompOpNode*> Predicat
%nterm <node::BinCompOpNode_t> CompareOpetators

%start program

%%

program: Scope {
    driver->SetRootNode($1);
    std::cout << "main\n";
};

Scope: StatementList {
    $$ = $1;
};

StatementList: StatementList Statement {
    std::cout << "add statement" << std::endl;
    $1->AddStatement($2);
    $$ = $1;
} | %empty {
    std::cout << "empty" << std::endl;
    $$ = new node::ScopeNode();
};

Statement: OUTPUT Expression SEMICOLON {
    std::cout << "Expression" << std::endl;
    $$ = static_cast<node::Node*>(new node::OutputNode($2));
} | Condition {
    std::cout << "cond" << std::endl;
    $$ = $1;
}
| Loop {
    std::cout << "loop" << std::endl;
    $$ = $1;
} | Assigment SEMICOLON {
    std::cout << "Assign" << std::endl;
    $$ = $1;
};

Condition: IF LBRAC Predicat RBRAC LCURBRAC Scope RCURBRAC Else {
    $$ = static_cast<node::Node*>(new node::CondNode($3, $6, $8));
    std::cout << "in cond" << std::endl;
};

Else: ELSE LCURBRAC Scope RCURBRAC {
    std::cout << "else" << std::endl;
    $$ = $3;
} | %empty {
    $$ = nullptr;
};

Loop : WHILE LBRAC Predicat RBRAC LCURBRAC Scope RCURBRAC {
    std::cout << "in loop" << std::endl;
    $$ = static_cast<node::Node*>(new node::LoopNode($3, $6));
};

Predicat: Expression CompareOpetators Expression {
    std::cout << "Predicat" << std::endl;
    $$ = new node::BinCompOpNode($2, $1, $3);
};

CompareOpetators: EQUAL {
    $$ = node::BinCompOpNode_t::equal;
} | NOT_EQUAL {
    $$ = node::BinCompOpNode_t::not_equal;
} | GREATER {
    $$ = node::BinCompOpNode_t::greater;
} | LESS {
    $$ = node::BinCompOpNode_t::less;
} | GREATER_OR_EQUAL {
    $$ = node::BinCompOpNode_t::greater_or_equal;
} | LESS_OR_EQUAL {
    $$ = node::BinCompOpNode_t::less_or_equal;
};

Assigment: NAME ASSIGMENT Expression {
    std::cout << "in assign" << std::endl;
    $$ = static_cast<node::Node*>(new node::AssignNode(new node::DeclNode($1), $3));
};

Expression: Expression ADD Summand {
    std::cout << "ADD" << std::endl;
    $$ = static_cast<node::ExprNode*>(new node::BinOpNode(node::BinOpNode_t::add, $1, $3));
} | Expression SUB Summand {
    $$ = static_cast<node::ExprNode*>(new node::BinOpNode(node::BinOpNode_t::sub, $1, $3));
    std::cout << "Sub" << std::endl;
} | Summand {
    $$ = $1;  
    std::cout << "Summand" << std::endl;
};

Summand: Summand MULT Multiplier {
    $$ = static_cast<node::ExprNode*>(new node::BinOpNode(node::BinOpNode_t::mul, $1, $3));
    std::cout << "mul" << std::endl;
} | Summand DIV Multiplier {
    $$ = static_cast<node::ExprNode*>(new node::BinOpNode(node::BinOpNode_t::div, $1, $3));
    std::cout << "dic" << std::endl;
} | Multiplier {
    $$ = $1;
    std::cout << "Multiplier" << std::endl;
};

Multiplier: LBRAC Expression RBRAC {
    $$ = $2;
    std::cout << "in Multiplier" << std::endl;
} | Terminals {
    $$ = $1;
    std::cout << "Terminals" << std::endl;
};

Terminals: NUMBER {
    std::cout << "Number = " << $1 << std::endl;
    // $$ = static_cast<node::ExprNode*>(new node::NumberNode($1));
} | NAME {
    std::cout << "Name = " << $1 << std::endl;
    std::cout << "Param " << std::endl;
    // $$ = static_cast<node::ExprNode*>(new node::VarNode($1));
} | INPUT {
    std::cout << "input" << std::endl;
    // $$ = static_cast<node::ExprNode*>(new node::InputNode());
};

%%

namespace yy {

parser::token_type yylex(parser::semantic_type* yylval,                         
                         Driver* driver)
{
  return driver->yylex(yylval);
}

void parser::error(const std::string&){}
}