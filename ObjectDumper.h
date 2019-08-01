
#ifndef __OBJECT_DUMPER_H__
#define __OBJECT_DUMPER_H__
#include <string>

class GooString;
class PDFDoc;
class TextOutputDev;
class UnicodeMap;
class TextLine;
class CairoOutputDev;

class CPageInfo;
class CTextLine;

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

    void cropImage(unsigned int page, int resolution, double xMin, double yMin, double xMax, double yMax, ImageFormat format);

private:
    void dumpText(int page, int resolution, CPageInfo &page_info);
    void dumpLineText(TextLine *line, CTextLine &cLine);

    void beginDocument(ImageFormat format, double w, double h);
    void beginPage(ImageFormat format, double resolution, double *w, double *h);
    void renderPage(ImageFormat format, double resolution, PDFDoc *doc, CairoOutputDev *cairoOut, int pg,
                    double page_w, double page_h,
                    double output_w, double output_h);
    void endPage(ImageFormat format, GooString *imageFileName);
    void endDocument(ImageFormat format);

private:
    const std::string m_fileName;
    GooString *m_ownerPW;
    GooString *m_userPW;
    PDFDoc *m_doc;

    TextOutputDev *m_textOut;
    UnicodeMap *m_uMap;

    CairoOutputDev *m_cairoOut;
    cairo_surface_t *m_surface;

    // 临时存储, cairo 需要用
    int m_paperWidth = -1;
    int m_paperHeight = -1;
};

#endif
