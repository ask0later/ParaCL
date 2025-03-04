/* ------------------------------------------------------------------------- **
 *
 *  Grammar of compiler 
 *  
 *  Scope -> StatementList 
 *  StatementList -> Statement StatementList | Empty
 *  Statement -> Output Expression | Condition | Loop | Assigment SEMICOLON | SubScope | SEMICOLON
 *
 *
 *  Expression -> Assigment | LogicExpr
 *  LogicExpr -> LogicExpr LOGIC_OR LogicExpr | LogicExpr LOGIC_AND LogicExpr | CompExpr
 *                                                          ^_ has more priority 
 *  CompExpr -> CompExpr EQUAL CompExpr | CompExpr NOT_EQUAL CompExpr | <-- have less priority
 *  CompExpr GREATER CompExpr |
 *  CompExpr LESS CompExpr | 
 *  CompExpr GREATER_OR_EQUAL CompExpr |
 *  CompExpr LESS_OR_EQUAL CompExpr | MathExpr
 *  MathExpr -> MathExpr ADD Summand | MathExpr MINUS Summand | Summand
 *  Summand -> Summand MULT Multiplier | Summand DIV Multiplier | Summand REMAINDER Multiplier | Multiplier
 *  Multiplier -> LBRAC Expression RBRAC | NEGATION Multiplier | MINUS Multiplier | Terminals
 *  Terminals -> NUMBER | NAME | INPUT
 *
 *  Condition -> IF LBRAC Expression RBRAC Statement %prec LOWER_THAN_ELSE | IF LBRAC Expression RBRAC Statement ELSE Statement
 *  Loop -> WHILE LBRAC Expression RBRAC Statement 
 *  Assigment -> NAME ASSIGMENT Expression
 *  SubScope -> LCURBRAC Scope RCURBRAC
 *
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
%nterm <node::ExprNode*> LogicExpr
%nterm <node::ExprNode*> CompExpr
%nterm <node::ExprNode*> MathExpr

%nterm <node::ExprNode*> Summand
%nterm <node::ExprNode*> Multiplier
%nterm <node::ExprNode*> Terminals

%left EQUAL NOT_EQUAL
%left GREATER LESS GREATER_OR_EQUAL LESS_OR_EQUAL

%left LOGIC_OR
%left LOGIC_AND

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
    $$ = driver->GetNode<node::CondNode>($3, $5, nullptr, @1);
} | IF LBRAC Expression RBRAC Statement ELSE Statement {
    $$ = driver->GetNode<node::CondNode>($3, $5, $7, @1);
};

Loop: WHILE LBRAC Expression RBRAC Statement {
    $$ = driver->GetNode<node::LoopNode>($3, $5, @1);
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
} | LogicExpr {
    $$ = $1;
};

LogicExpr: LogicExpr LOGIC_OR LogicExpr {
    $$ = driver->GetNode<node::LogicOpNode>(node::LogicOpNode_t::logic_or, $1, $3, @2);
} | LogicExpr LOGIC_AND LogicExpr {
    $$ = driver->GetNode<node::LogicOpNode>(node::LogicOpNode_t::logic_and, $1, $3, @2);
} | CompExpr {
    $$ = $1;
};

CompExpr: CompExpr EQUAL CompExpr { 
	$$ = driver->GetNode<node::BinCompOpNode>(node::BinCompOpNode_t::equal, $1, $3, @2); 
} | CompExpr NOT_EQUAL CompExpr {
	$$ = driver->GetNode<node::BinCompOpNode>(node::BinCompOpNode_t::not_equal, $1, $3, @2); 
} | CompExpr GREATER CompExpr { 
	$$ = driver->GetNode<node::BinCompOpNode>(node::BinCompOpNode_t::greater, $1, $3, @2); 
} | CompExpr LESS CompExpr {
	$$ = driver->GetNode<node::BinCompOpNode>(node::BinCompOpNode_t::less, $1, $3, @2);
} | CompExpr GREATER_OR_EQUAL CompExpr {
	$$ = driver->GetNode<node::BinCompOpNode>(node::BinCompOpNode_t::greater_or_equal, $1, $3, @2);
} | CompExpr LESS_OR_EQUAL CompExpr {
	$$ = driver->GetNode<node::BinCompOpNode>(node::BinCompOpNode_t::less_or_equal, $1, $3, @2);
} | MathExpr {
	$$ = $1;
};

MathExpr: MathExpr ADD Summand {
    $$ = driver->GetNode<node::BinOpNode>(node::BinOpNode_t::add, $1, $3, @2);
} | MathExpr MINUS Summand {
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