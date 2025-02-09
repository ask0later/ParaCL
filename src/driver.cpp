#include <string>
#include "driver.hpp"

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cout << "Choose program to execute" << std::endl;
        return 0;
    }

    try {
        std::string file_name(argv[1]);
        yy::Driver &driver = yy::Driver::QueryDriver(file_name);
        driver.parse();
        driver.DrawAST();
        driver.Execute();
    } catch (std::exception &ex) {
        std::cout << ex.what() << std::endl;
    };
}