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

    struct CPageInfo
    {
        double width;
        double height;
        unsigned int flowLen;
        CTextFlow *flows;
    };

    void init_global_params(const char *poppler_data = nullptr);

    void destroy_global_params();

    void free_page_info(CPageInfo *page_info);

    void *create_arser(const char *fileName, const char *owner_pw = nullptr, const char *user_pw = nullptr);
    void destroy_parser(void *parser);
    bool parser_is_ok(void *parser);
    unsigned int parser_get_num_pages(void *parser);
    CPageInfo *parser_parse(void *parser, int page);
#ifdef __cplusplus
}
#endif

#endif
