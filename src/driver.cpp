#include "driver.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Choose program to execute" << std::endl;
        return 0;
    }

    try {
        yy::Driver &driver = yy::Driver::QueryDriver(argv[1]);
        driver.Parse();
        driver.DrawAST();
        driver.Execute();
    } catch (std::exception &ex) {
        std::cout << ex.what() << std::endl;
    };
}