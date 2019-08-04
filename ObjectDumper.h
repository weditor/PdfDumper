
#ifndef __OBJECT_DUMPER_H__
#define __OBJECT_DUMPER_H__
#include <string>
#include <map>
#include "CDumpApi.h"
#include "ImageDumper.h"

class GooString;
class PDFDoc;
class TextOutputDev;
class UnicodeMap;
class TextLine;
class CairoOutputDev;

enum ImageFormat;
class ImageDumpper;

// class CPageInfo;
// class CTextLine;

class ObjectDumper
{
public:
    ObjectDumper(const char *fileName, const char *ownerPW = nullptr, const char *userPW = nullptr);
    ~ObjectDumper();

    bool isOk() const;
    unsigned int getNumPages();
    CPageInfo *parse(int page);
    ImageDumpper *getImageDumpper(ImageFormat format);

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

    std::map<ImageFormat, ImageDumpper *> m_imageDumpper;

    // 临时存储, cairo 需要用
    int m_paperWidth = -1;
    int m_paperHeight = -1;
};

#endif
