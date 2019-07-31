#include "CDumpApi.h"
#include "poppler/GlobalParams.h"

void init_global_params(const char *poppler_data)
{
    if (!globalParams)
    {
        globalParams = new GlobalParams(poppler_data);
    }
}

void destroy_global_params()
{
    if (globalParams)
    {
        delete globalParams;
    }
}

void freePageInfo(CPageInfo *page_info) {}
