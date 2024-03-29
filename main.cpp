
#include <iostream>
#include "ObjectDumper.h"
#include "CDumpApi.h"
#include <stdio.h>
#include <stdlib.h>

int main(int args, char **argv)
{
    std::cout << "hello world" << std::endl;

    init_global_params();
    ObjectDumper *dumper = new ObjectDumper("../test/ths.pdf");
    // ObjectDumper dumper("../mydata/yb/gu_yb.pdf");
    std::cout << "dumper.isOk(): " << dumper->isOk() << std::endl;
    std::cout << "dumper.getNumPages(): " << dumper->getNumPages() << std::endl;
    CPageInfo *info = dumper->parse(1);
    // auto graph = dumper.cropImage(1);
    std::cout << "svg size: " << info->graph.size << std::endl;

    // if (info->graph.size)
    // {
    //     std::cout << "svg content: " << info->graph.content << std::endl;
    // }

    FILE *output = fopen("test.svg", "wb");
    fwrite(info->graph.content, 1, info->graph.size, output);
    fclose(output);

    // auto imgDumper = dumper->getImageDumpper(fmtPng);
    // imgDumper->cropImage("test.png", 1, 150, 0, 0, 0, 0);
    // imgDumper->cropImage("test2.png", 1, 150, 600, 0, 900, 1000);
    auto imgDumper = dumper->getImageDumpper(fmtJpeg);
    imgDumper->cropImage("test.jpeg", 1, 150, 0, 0, 0, 0);
    imgDumper->cropImage("test2.jpeg", 1, 150, 600, 0, 900, 1000);

    free_page_info(info);
    delete dumper;

    destroy_global_params();

    return 0;
}
