/* ------------------------------------------------------------------------- **
 *
 *  Grammar of compiler 
 *  
 *  Scope -> StatementList 
 *  StatementList -> Statement StatementList | Empty
 *  Statement -> Output Expression | Condition | Loop | Assigment SEMICOLON | SubScope | SEMICOLON
 *
 *
 *  Expression -> Assigment | sExpression
 *  sExpression -> sExpression LOGIC_OR ssExpression | ssExpression
 *  ssExpression -> ssExpression LOGIC_AND sssExpression | sssExpression
 *  sssExpression -> sssExpression NotPriorityCompareOperators ssssExpression | ssssExpression
 *  ssssExpression -> ssssExpression PriorityCompareOperators sssssExpression | sssssExpression
 *  sssssExpression -> sssssExpression ADD Summand | sssssExpression MINUS Summand | Summand 
 *  Summand -> Summand MULT Multiplier | Summand DIV Multiplier | Summand REMAINDER Multiplier | Multiplier
 *  Multiplier -> LBRAC Expression RBRAC | NEGATION Multiplier | MINUS Multiplier | Terminals
 *  Terminals -> NUMBER | NAME | INPUT
 *
 *
 *  Condition -> IF LBRAC Expression RBRAC Statement %prec LOWER_THAN_ELSE | IF LBRAC Expression RBRAC Statement ELSE Statement
 *  Loop -> WHILE LBRAC Expression RBRAC Statement 
 *  Assigment -> NAME ASSIGMENT Expression
 *  SubScope -> LCURBRAC Scope RCURBRAC
 *
 *  NotPriorityCompareOperators: EQUAL | NOT_EQUAL
 *  PriorityCompareOperators: GREATER | LESS | GREATER_OR_EQUAL | LESS_OR_EQUAL
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

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

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

/* Compare binary operators */ 
    NEGATION

/* Compare binary operators */
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
    $$ = driver->GetNode<node::ScopeNode>(driver->GetLocation());
};

Statement: OUTPUT Expression SEMICOLON {
    $$ = driver->GetNode<node::OutputNode>($2, @1);
} | Condition {
    $$ = $1;
} | Loop {
    $$ = $1;
} | Assigment SEMICOLON {
    $$ = $1;
} | SubScope {
    $$ = $1;
} | SEMICOLON {
    $$ = nullptr;
};

Condition: IF LBRAC Expression RBRAC Statement %prec LOWER_THAN_ELSE {
    node::ScopeNode* scope = nullptr;
    if ($5->type_ == node::Node_t::scope) {
        scope = static_cast<node::ScopeNode*>($5);
    } else {
        scope = driver->GetNode<node::ScopeNode>(@5);
        scope->AddStatement($5);
    }
    
    $$ = driver->GetNode<node::CondNode>($3, scope, nullptr, @1);
} | IF LBRAC Expression RBRAC Statement ELSE Statement {
    node::ScopeNode* scope1 = nullptr;
    if ($5->type_ == node::Node_t::scope) {
        scope1 = static_cast<node::ScopeNode*>($5);
    } else {
        scope1 = driver->GetNode<node::ScopeNode>(@5);
        scope1->AddStatement($5);
    }

    node::ScopeNode* scope2 = nullptr;
    if ($7->type_ == node::Node_t::scope) {
        scope2 = static_cast<node::ScopeNode*>($7);
    } else {
        scope2 = driver->GetNode<node::ScopeNode>(@7);
        scope2->AddStatement($7);
    }

    $$ = driver->GetNode<node::CondNode>($3, scope1, scope2, @1);
};

Loop: WHILE LBRAC Expression RBRAC Statement {
    node::Node* node = nullptr;
    if ($5->type_ == node::Node_t::scope) {
        node = $5;
    } else {
        auto scope = driver->GetNode<node::ScopeNode>(@5);
        scope->AddStatement($5);
        node = static_cast<node::Node*>(scope);
    }

    $$ = driver->GetNode<node::LoopNode>($3, static_cast<node::ScopeNode*>(node), @1);
};

SubScope: LCURBRAC Scope RCURBRAC {
    $$ = $2;
}

Assigment: NAME ASSIGMENT Expression {
    auto name = driver->GetNode<node::DeclNode>($1, @1);
    $$ = driver->GetNode<node::AssignNode>(name, $3, @2);
};

Expression: Assigment {
    $$ = $1;
} | sExpression {
    $$ = $1;
};

sExpression: sExpression LOGIC_OR ssExpression {
    $$ = driver->GetNode<node::LogicOpNode>(node::LogicOpNode_t::logic_or, $1, $3, @2);
} | ssExpression {
    $$ = $1;
};

ssExpression: ssExpression LOGIC_AND sssExpression {
    $$ = driver->GetNode<node::LogicOpNode>(node::LogicOpNode_t::logic_and, $1, $3, @2);
} | sssExpression {
    $$ = $1;
};

sssExpression: sssExpression NotPriorityCompareOperators ssssExpression {
    $$ = driver->GetNode<node::BinCompOpNode>($2, $1, $3, @2);
} | ssssExpression {
    $$ = $1;
};

ssssExpression: ssssExpression PriorityCompareOperators sssssExpression {
    $$ = driver->GetNode<node::BinCompOpNode>($2, $1, $3, @2);
} | sssssExpression {
    $$ = $1;
};

sssssExpression: sssssExpression ADD Summand {
    $$ = driver->GetNode<node::BinOpNode>(node::BinOpNode_t::add, $1, $3, @2);
} | sssssExpression MINUS Summand {
    $$ = driver->GetNode<node::BinOpNode>(node::BinOpNode_t::sub, $1, $3, @2);
} | Summand {
    $$ = $1;
};

Summand: Summand MULT Multiplier {
    $$ = driver->GetNode<node::BinOpNode>(node::BinOpNode_t::mul, $1, $3, @2);
} | Summand DIV Multiplier {
    $$ = driver->GetNode<node::BinOpNode>(node::BinOpNode_t::div, $1, $3, @2);
} | Summand REMAINDER Multiplier {
    $$ = driver->GetNode<node::BinOpNode>(node::BinOpNode_t::remainder, $1, $3, @2);
} | Multiplier {
    $$ = $1;
};

Multiplier: LBRAC Expression RBRAC {
    $$ = $2;
} | NEGATION Multiplier {
    $$ = driver->GetNode<node::UnOpNode>(node::UnOpNode_t::negation, $2, @1);
} | MINUS Multiplier {
    $$ = driver->GetNode<node::UnOpNode>(node::UnOpNode_t::minus, $2, @1);
} | Terminals {
    $$ = $1;
};

Terminals: NUMBER {
    $$ = driver->GetNode<node::NumberNode>($1, @1);
} | NAME {
    $$ = driver->GetNode<node::VarNode>($1, @1);
} | INPUT {
    $$ = driver->GetNode<node::InputNode>(@1);
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