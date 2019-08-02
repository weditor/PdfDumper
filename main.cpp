
#include <iostream>
#include "ObjectDumper.h"
#include "CDumpApi.h"
#include <stdio.h>
#include <stdlib.h>

int main(int args, char **argv)
{
    std::cout << "hello world" << std::endl;

    init_global_params();
    ObjectDumper dumper("../test/ths.pdf");
    // ObjectDumper dumper("../mydata/yb/gu_yb.pdf");
    std::cout << "dumper.isOk(): " << dumper.isOk() << std::endl;
    std::cout << "dumper.getNumPages(): " << dumper.getNumPages() << std::endl;
    auto info = *dumper.parse(1);
    // auto graph = dumper.cropImage(1);
    std::cout << "svg size: " << info.graph.size << std::endl;

    // if (info.graph.size)
    // {
    //     std::cout << "svg content: " << info.graph.content << std::endl;
    // }

    FILE *output = fopen("test.svg", "wb");
    fwrite(info.graph.content, 1, info.graph.size, output);
    fclose(output);

    destroy_global_params();

    return 0;
}
