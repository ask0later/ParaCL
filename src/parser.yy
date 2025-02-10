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
%locations
%define api.location.type { yy::Location }

%param {yy::Driver* driver}

%code requires
{
    #include "node.hpp"
    // forward decl of argument to parser
    namespace yy { 
        class Driver; 
    }
}

%code
{
    #include <exception>
    #include "node.hpp"
    #include "driver.hpp"

    namespace yy {
        parser::token_type yylex(parser::semantic_type* yylval, Location* yylloc,
                         Driver* driver);
    }
}

%token
/* Binary opetators */
    ADD
    MINUS
    MULT
    DIV
    REMAINDER

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

/* Logic operators */
    LOGIC_AND
    LOGIC_OR

/* Compare binary opetators */ 
    NEGATION

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
%nterm <node::ScopeNode*> Else
%nterm <node::CondNode*> Condition
%nterm <node::LoopNode*> Loop

%nterm <node::ExprNode*> Expression
%nterm <node::ExprNode*> sExpression
%nterm <node::ExprNode*> ssExpression
%nterm <node::ExprNode*> sssExpression
%nterm <node::ExprNode*> ssssExpression
%nterm <node::ExprNode*> sssssExpression
%nterm <node::ExprNode*> Summand
%nterm <node::ExprNode*> Multiplier
%nterm <node::ExprNode*> Terminals

%nterm <node::BinCompOpNode_t> PriorityCompareOperators
%nterm <node::BinCompOpNode_t> NotPriorityCompareOperators

%start program

%%

NotPriorityCompareOperators: EQUAL {
    $$ = node::BinCompOpNode_t::equal;
} | NOT_EQUAL {
    $$ = node::BinCompOpNode_t::not_equal;
};

PriorityCompareOperators: GREATER {
    $$ = node::BinCompOpNode_t::greater;
} | LESS {
    $$ = node::BinCompOpNode_t::less;
} | GREATER_OR_EQUAL {
    $$ = node::BinCompOpNode_t::greater_or_equal;
} | LESS_OR_EQUAL {
    $$ = node::BinCompOpNode_t::less_or_equal;
};

program: Scope {
    driver->SetRootNode($1);
};

Scope: StatementList {
    $$ = $1;
};

StatementList: StatementList Statement {
    if ($2)
        $1->AddStatement($2);

    $$ = $1;
} | %empty {
    $$ = driver->template GetNode<node::ScopeNode>(driver->GetLocation());
};

Statement: OUTPUT Expression SEMICOLON {
    auto tmp = driver->template GetNode<node::OutputNode>($2, @1);
    $$ = static_cast<node::Node*>(tmp);
} | Condition {
    $$ = static_cast<node::Node*>($1);
} | Loop {
    $$ = static_cast<node::Node*>($1);
} | Assigment SEMICOLON {
    $$ = static_cast<node::Node*>($1);
} | SubScope {
    $$ = static_cast<node::Node*>($1);
} | SEMICOLON {
    $$ = nullptr;
};

Else: ELSE LCURBRAC Scope RCURBRAC {
    $$ = $3;
} | %empty {
    $$ = nullptr;
};

Condition: IF LBRAC Expression RBRAC LCURBRAC Scope RCURBRAC Else {
    $$ = driver->template GetNode<node::CondNode>($3, $6, $8, @1);
};

Loop: WHILE LBRAC Expression RBRAC LCURBRAC Scope RCURBRAC {
    $$ = driver->template GetNode<node::LoopNode>($3, $6, @1);
};

SubScope: LCURBRAC Scope RCURBRAC {
    $$ = $2;
}

Assigment: NAME ASSIGMENT Expression {
    auto name = driver->template GetNode<node::DeclNode>($1, @1);
    $$ = driver->template GetNode<node::AssignNode>(name, $3, @2);
};

Expression: Assigment {
    $$ = static_cast<node::ExprNode*>($1);
} | sExpression {
    $$ = $1;
};

sExpression: sExpression LOGIC_OR ssExpression {
    auto logic_op = driver->template GetNode<node::LogicOpNode>(node::LogicOpNode_t::logic_or, $1, $3, @2);
    $$ = static_cast<node::ExprNode*>(logic_op);
} | ssExpression {
    $$ = $1;
};

ssExpression: ssExpression LOGIC_AND sssExpression {
    auto logic_op = driver->template GetNode<node::LogicOpNode>(node::LogicOpNode_t::logic_and, $1, $3, @2);
    $$ = static_cast<node::ExprNode*>(logic_op);
} | sssExpression {
    $$ = $1;
};

sssExpression: sssExpression NotPriorityCompareOperators ssssExpression {
    auto bin_op = driver->template GetNode<node::BinCompOpNode>($2, $1, $3, @2);
    $$ = static_cast<node::ExprNode*>(bin_op);
} | ssssExpression {
    $$ = $1;
};

ssssExpression: ssssExpression PriorityCompareOperators sssssExpression {
    auto bin_op = driver->template GetNode<node::BinCompOpNode>($2, $1, $3, @2);
    $$ = static_cast<node::ExprNode*>(bin_op);
} | sssssExpression {
    $$ = $1;
};

sssssExpression: sssssExpression ADD Summand {
    auto binop = driver->template GetNode<node::BinOpNode>(node::BinOpNode_t::add, $1, $3, @2);
    $$ = static_cast<node::ExprNode*>(binop);
} | sssssExpression MINUS Summand {
    auto binop = driver->template GetNode<node::BinOpNode>(node::BinOpNode_t::sub, $1, $3, @2);
    $$ = static_cast<node::ExprNode*>(binop);
} | Summand {
    $$ = $1;
};

Summand: Summand MULT Multiplier {
    auto binop = driver->template GetNode<node::BinOpNode>(node::BinOpNode_t::mul, $1, $3, @2);
    $$ = static_cast<node::ExprNode*>(binop);
} | Summand DIV Multiplier {
    auto binop = driver->template GetNode<node::BinOpNode>(node::BinOpNode_t::div, $1, $3, @2);
    $$ = static_cast<node::ExprNode*>(binop);
} | Summand REMAINDER Multiplier {
    auto binop = driver->template GetNode<node::BinOpNode>(node::BinOpNode_t::remainder, $1, $3, @2);
    $$ = static_cast<node::ExprNode*>(binop);
} | Multiplier {
    $$ = $1;
};

Multiplier: LBRAC Expression RBRAC {
    $$ = $2;
} | NEGATION Multiplier {
    auto unop = driver->template GetNode<node::UnOpNode>(node::UnOpNode_t::negation, $2, @1);
    $$ = static_cast<node::ExprNode*>(unop);
} | MINUS Multiplier {
    auto unop = driver->template GetNode<node::UnOpNode>(node::UnOpNode_t::minus, $2, @1);
    $$ = static_cast<node::ExprNode*>(unop);
} | Terminals {
    $$ = $1;
};

Terminals: NUMBER {
    auto number = driver->template GetNode<node::NumberNode>($1, @1);
    $$ = static_cast<node::ExprNode*>(number);
} | NAME {
    auto name = driver->template GetNode<node::VarNode>($1, @1);
    $$ = static_cast<node::ExprNode*>(name);
} | INPUT {
    auto input = driver->template GetNode<node::InputNode>(@1);
    $$ = static_cast<node::ExprNode*>(input);
};

%%

namespace yy {

parser::token_type yylex(parser::semantic_type* yylval, Location* loc,
                         Driver* driver) {
  return driver->yylex(yylval, loc);
}

void parser::error(const location_type& loc, const std::string&) {
    throw std::runtime_error(std::string("got '" + std::string(driver->GetCurrentTokenText()) + "'"));
}
}