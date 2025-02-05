/* ------------------------------------------------------------------------- **
 *
 *  Grammar of compiler 
 *  
 *  Scope -> StatementList 
 *  StatementList -> Statement StatementList | Empty
 *  Statement -> While | If | Assigment | SubScope;
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
 *  Assigment -> Variable = Expression
 *  SubScope -> Scope
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
#include <exception>
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
%nterm <node::ScopeNode*> SubScope
%nterm <node::ScopeNode*> StatementList
%nterm <node::Node*> Statement

%nterm <node::AssignNode*> Assigment
%nterm <node::CondNode*> Condition
%nterm <node::ScopeNode*> Else
%nterm <node::LoopNode*> Loop

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
};

Scope: StatementList {
    $$ = $1;
};

StatementList: StatementList Statement {
    if ($2)
        $1->AddStatement($2);
    else {
        int num_line = driver->GetCurrentLineNumber();
        std::string error_mes = "Syntax error, '";
        error_mes += driver->GetCurrentTokenText();
        error_mes += "' at line #";
        error_mes += std::to_string(num_line);
        error_mes += ":\n";
        error_mes += driver->program_text_[num_line - 1];
        throw std::logic_error(error_mes);
    }

    $$ = $1;
} | %empty {
    $$ = driver->template GetNode<node::ScopeNode>(driver->GetCurrentLineNumber());
};

Statement: OUTPUT Expression SEMICOLON {
    auto tmp = driver->template GetNode<node::OutputNode>($2, driver->GetCurrentLineNumber());
    $$ = static_cast<node::Node*>(tmp);
} | Condition {
    $$ = static_cast<node::Node*>($1);
} | Loop {
    $$ = static_cast<node::Node*>($1);
} | Assigment SEMICOLON {
    $$ = static_cast<node::Node*>($1);
} | SubScope {
    $$ = static_cast<node::Node*>($1);
} | error {
    $$ = nullptr;
};

Condition: IF LBRAC Predicat RBRAC LCURBRAC Scope RCURBRAC Else {
    $$ = driver->template GetNode<node::CondNode>($3, $6, $8, driver->GetCurrentLineNumber());
};

Else: ELSE LCURBRAC Scope RCURBRAC {
    $$ = $3;
} | %empty {
    $$ = nullptr;
};

Loop: WHILE LBRAC Predicat RBRAC LCURBRAC Scope RCURBRAC {
    $$ = driver->template GetNode<node::LoopNode>($3, $6, driver->GetCurrentLineNumber());
};

SubScope: LCURBRAC Scope RCURBRAC {
    $$ = $2;
}

Predicat: Expression CompareOpetators Expression {
    $$ = driver->template GetNode<node::BinCompOpNode>($2, $1, $3, driver->GetCurrentLineNumber());
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
    auto name = driver->template GetNode<node::DeclNode>($1, driver->GetCurrentLineNumber());
    $$ = driver->template GetNode<node::AssignNode>(name, $3, driver->GetCurrentLineNumber());
};

Expression: Expression ADD Summand {
    auto binop = driver->template GetNode<node::BinOpNode>(node::BinOpNode_t::add, $1, $3, driver->GetCurrentLineNumber());
    $$ = static_cast<node::ExprNode*>(binop);
} | Expression SUB Summand {
    auto binop = driver->template GetNode<node::BinOpNode>(node::BinOpNode_t::sub, $1, $3, driver->GetCurrentLineNumber());
    $$ = static_cast<node::ExprNode*>(binop);
} | Summand {
    $$ = $1;  
};

Summand: Summand MULT Multiplier {
    auto binop = driver->template GetNode<node::BinOpNode>(node::BinOpNode_t::mul, $1, $3, driver->GetCurrentLineNumber());
    $$ = static_cast<node::ExprNode*>(binop);
} | Summand DIV Multiplier {
    auto binop = driver->template GetNode<node::BinOpNode>(node::BinOpNode_t::div, $1, $3, driver->GetCurrentLineNumber());
    $$ = static_cast<node::ExprNode*>(binop);
} | Multiplier {
    $$ = $1;
};

Multiplier: LBRAC Expression RBRAC {
    $$ = $2;
} | Terminals {
    $$ = $1;
};

Terminals: NUMBER {
    auto number = driver->template GetNode<node::NumberNode>($1, driver->GetCurrentLineNumber());
    $$ = static_cast<node::ExprNode*>(number);
} | NAME {
    auto name = driver->template GetNode<node::VarNode>($1, driver->GetCurrentLineNumber());
    $$ = static_cast<node::ExprNode*>(name);
} | INPUT {
    auto input = driver->template GetNode<node::InputNode>(driver->GetCurrentLineNumber());
    $$ = static_cast<node::ExprNode*>(input);
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