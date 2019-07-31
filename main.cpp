
#include <iostream>
#include "ObjectDumper.h"
#include "CDumpApi.h"

int main(int args, char **argv)
{
    std::cout << "hello world" << std::endl;

    init_global_params();
    ObjectDumper dumper("../test/ths.pdf");
    std::cout << dumper.isOk() << std::endl;
    std::cout << dumper.getNumPages() << std::endl;
    dumper.parse(1);
    destroy_global_params();

    return 0;
}
