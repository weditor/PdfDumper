#ifndef __CDUMP_API_H__
#define __CDUMP_API_H__

#ifdef __cplusplus
extern "C"
{
#endif

    struct CColor
    {
        double r;
        double g;
        double b;
    };

    struct CTextChar
    {
        char *bytes;
        unsigned char len;
        int font;

        double xMin;
        double yMin;
        double xMax;
        double yMax;
    };

    struct CTextWord
    {
        double fontSize;
        int rotation;
        bool spaceAfter;
        CColor color;

        double xMin;
        double yMin;
        double xMax;
        double yMax;

        unsigned int charLen;
        CTextChar *chars;
    };

    struct CTextLine
    {
        unsigned int wordLen;
        CTextWord *words;

        double xMin;
        double yMin;
        double xMax;
        double yMax;
    };
    struct CTextBlock
    {
        unsigned int lineLen;
        CTextLine *lines;

        double xMin;
        double yMin;
        double xMax;
        double yMax;
    };

    struct CTextFlow
    {
        unsigned int blockLen;
        CTextBlock *blocks;
    };

    struct CGraphSvg
    {
        long size;
        char *content;
    };

    struct CPageInfo
    {
        double width;
        double height;
        unsigned int flowLen;
        CTextFlow *flows;
        CGraphSvg graph;
    };

    extern void init_global_params(const char *poppler_data = nullptr);
    extern void destroy_global_params();

    extern void free_page_info(CPageInfo *page_info);

    extern void *create_parser(const char *fileName, const char *owner_pw = nullptr, const char *user_pw = nullptr);
    extern void destroy_parser(void *parser);
    extern bool parser_is_ok(void *parser);
    extern unsigned int parser_get_num_pages(void *parser);
    extern CPageInfo *parser_parse(void *parser, int page);

    extern void *get_image_dumper(void *parser, int format);
    extern void crop_page(const char *filename, unsigned int page, double resolution, int left, int top, int right, int bottom);

#ifdef __cplusplus
}
#endif

#endif
