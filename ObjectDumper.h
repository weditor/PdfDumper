
#ifndef __OBJECT_DUMPER_H__
#define __OBJECT_DUMPER_H__
#include <string>

class GooString;
class PDFDoc;
class TextOutputDev;
class UnicodeMap;
class TextLine;

class CPageInfo;
class CTextLine;

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

private:
    const std::string m_fileName;
    GooString *m_ownerPW;
    GooString *m_userPW;
    PDFDoc *m_doc;

    TextOutputDev *m_textOut;
    UnicodeMap *m_uMap;
};

#endif
