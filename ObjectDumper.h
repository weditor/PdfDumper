
#ifndef __OBJECT_DUMPER_H__
#define __OBJECT_DUMPER_H__
#include <string>
#include "CDumpApi.h"

class GooString;
class PDFDoc;
class TextOutputDev;
class UnicodeMap;
class TextLine;
class CairoOutputDev;

// class CPageInfo;
// class CTextLine;

enum ImageFormat
{
    fmtJpeg = 1,
    fmtPng = 2,
    fmtTiff = 3,
    fmtSvg = 4,
};

class ObjectDumper
{
public:
    ObjectDumper(const char *fileName, const char *ownerPW = nullptr, const char *userPW = nullptr);
    ~ObjectDumper();

    bool isOk() const;
    unsigned int getNumPages();
    CPageInfo *parse(int page);

private:
    void dumpText(int page, int resolution, CPageInfo &page_info);
    void dumpLineText(TextLine *line, CTextLine &cLine);
    CGraphSvg dumpGraph(unsigned int page);

private:
    const std::string m_fileName;
    GooString *m_ownerPW;
    GooString *m_userPW;
    PDFDoc *m_doc;

    TextOutputDev *m_textOut;
    UnicodeMap *m_uMap;

    CairoOutputDev *m_cairoOut;
    // cairo_surface_t *m_surface;

    // 临时存储, cairo 需要用
    int m_paperWidth = -1;
    int m_paperHeight = -1;
};

#endif
