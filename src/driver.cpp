#include "driver.hpp"

// here we can return non-zero if lexing is not done inspite of EOF detected
int yyFlexLexer::yywrap() { return 1; }

int main() {
  FlexLexer *lexer = new yyFlexLexer;
  yy::Driver driver(lexer);
  driver.parse();
  driver.DrawAST();
  driver.Execute();
  delete lexer;
}