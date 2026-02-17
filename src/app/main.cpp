#include "headers/mtportscanner.hpp"

int main()
{
    MTPortScanner mtscanner{ "localhost", 0, 100, 1};
    mtscanner.run();

    mtscanner.printResults();

    return 0;
}