#pragma once

#include "parser.tab.hh"
#include <FlexLexer.h>

namespace yy {

class NumDriver {
  FlexLexer *plex_;

public:
  NumDriver(FlexLexer *plex) : plex_(plex) {}

  parser::token_type yylex(parser::semantic_type *yylval) {
    parser::token_type tt = static_cast<parser::token_type>(plex_->yylex());
    if (tt == yy::parser::token_type::NUMBER)
      yylval->as<int>() = std::stoi(plex_->YYText());

    if (tt == yy::parser::token_type::ERR) {
      std::cout << "Lexical error, unrecoginzed lexem: '" << plex_->YYText()
                << "' at line #" << plex_->lineno() << std::endl;
      std::terminate();
    }

    return tt;
  }

  bool parse() {
    parser parser(this);
    bool res = parser.parse();
    return !res;
  }
};

} // namespace yy