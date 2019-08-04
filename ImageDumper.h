
#ifndef __IMAGE_DUMPPLER_H__
#define __IMAGE_DUMPPLER_H__

class PDFDoc;
class CairoOutputDev;

enum ImageFormat
{
    fmtJpeg = 1,
    fmtPng = 2,
    fmtTiff = 3,
    fmtSvg = 4,
};

class ImageDumpper
{
public:
    ImageDumpper(PDFDoc *doc, ImageFormat format);
    ~ImageDumpper();

    void cropImage(const char *filename, unsigned int page, double resolution, int xMin, int yMin, int xMax, int yMax);

private:
    bool is_transp();
    void writePageImage(const char *filename, void *surface, double resolution);

private:
    const ImageFormat m_format;
    PDFDoc *m_doc;

    CairoOutputDev *m_cairoOut;
};

#endif
