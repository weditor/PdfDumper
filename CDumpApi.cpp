#include "CDumpApi.h"
#include "poppler/GlobalParams.h"
#include "ImageDumper.h"
#include "ObjectDumper.h"

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

void free_page_info(CPageInfo *page_info)
{
    if (!page_info)
    {
        return;
    }

    // delete graph
    if (page_info->graph.size && page_info->graph.content)
    {
        delete[] page_info->graph.content;
    }

    // delete text
    if (page_info->flowLen && page_info->flows)
    {
        for (size_t flowIdx = 0; flowIdx < page_info->flowLen; flowIdx++)
        {
            CTextFlow &flow = page_info->flows[flowIdx];
            for (size_t blkIdx = 0; blkIdx < flow.blockLen; blkIdx++)
            {
                CTextBlock &block = flow.blocks[blkIdx];
                for (size_t lineIdx = 0; lineIdx < block.lineLen; lineIdx++)
                {
                    CTextLine &line = block.lines[lineIdx];
                    for (size_t wIdx = 0; wIdx < line.wordLen; wIdx++)
                    {
                        CTextWord &word = line.words[wIdx];
                        for (size_t cIdx = 0; cIdx < word.charLen; cIdx++)
                        {
                            CTextChar &c = word.chars[cIdx];
                            delete[] c.bytes;
                        }
                        delete[] word.chars;
                    }
                    delete[] line.words;
                }
                delete[] block.lines;
            }
            delete[] flow.blocks;
        }

        delete[] page_info->flows;
    }
    delete page_info;
}

void *create_arser(const char *fileName, const char *owner_pw, const char *user_pw)
{
    return new ObjectDumper(fileName, owner_pw, user_pw);
}
void destroy_parser(void *parser)
{
    delete (ObjectDumper *)parser;
}
bool parser_is_ok(void *parser)
{
    return ((ObjectDumper *)parser)->isOk();
}
unsigned int parser_get_num_pages(void *parser)
{
    return ((ObjectDumper *)parser)->getNumPages();
}
CPageInfo *parser_parse(void *parser, int page)
{
    return ((ObjectDumper *)parser)->parse(page);
}

void *getImageDumper(void *parser, int format)
{
    return ((ObjectDumper *)parser)->getImageDumpper((ImageFormat)format);
}
void *cropPage(void *dumper, const char *filename, unsigned int page, double resolution, int left, int top, int right, int bottom)
{
    ((ImageDumpper *)dumper)->cropImage(filename, page, resolution, left, top, right, bottom);
}
