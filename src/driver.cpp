#include <string>
#include "driver.hpp"

// here we can return non-zero if lexing is not done inspite of EOF detected
int yyFlexLexer::yywrap() { return 1; }

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Choose program to execute" << std::endl;
        return 0;
    }

    try {
        FlexLexer *lexer = new yyFlexLexer;
        yy::Driver driver(lexer);
        std::string file_name(argv[1]);
        driver.parse(file_name);
        driver.DrawAST();
        driver.Execute();
        delete lexer;
    } catch (std::exception &ex) {
        std::cout << ex.what() << std::endl;
    };
}