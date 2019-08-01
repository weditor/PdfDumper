
#include "ObjectDumper.h"
#include "CDumpApi.h"
#include "poppler/Page.h"
#include "poppler/PDFDoc.h"
#include "poppler/PDFDocFactory.h"
#include "poppler/TextOutputDev.h"
#include "poppler/GlobalParams.h"
#include "goo/GooString.h"

#include "poppler/CairoOutputDev.h"
#include "goo/ImgWriter.h"
#include "goo/JpegWriter.h"
#include "goo/PNGWriter.h"

#include <cairo/cairo.h>
#include <cairo/cairo-svg.h>

#include <string>
#include <vector>

static const int default_resolution = 72;

ObjectDumper::ObjectDumper(const char *fileName, const char *ownerPW, const char *userPW) : m_fileName(fileName)
{
    m_ownerPW = ownerPW ? new GooString(ownerPW) : nullptr;
    m_userPW = userPW ? new GooString(userPW) : nullptr;

    m_doc = PDFDocFactory().createPDFDoc(GooString(m_fileName), m_ownerPW, m_userPW);

    bool physLayout = false;
    bool fixedPitch = false;
    if (fixedPitch)
    {
        physLayout = true;
    }
    bool rawOrder = false;
    bool htmlMeta = true;
    m_textOut = new TextOutputDev(nullptr, physLayout, fixedPitch, rawOrder, htmlMeta);
    m_cairoOut = new CairoOutputDev();

    m_uMap = globalParams->getTextEncoding();
    m_surface = nullptr
}

ObjectDumper::~ObjectDumper()
{
    m_uMap->decRefCnt();
    delete m_cairoOut;
    delete m_textOut;
    delete m_doc;
    delete m_userPW;
    delete m_ownerPW;
}

bool ObjectDumper::isOk() const
{
    return m_uMap && m_doc->isOk();
}

unsigned int ObjectDumper::getNumPages()
{
    return m_doc->getNumPages();
}

CPageInfo *ObjectDumper::parse(int page)
{
    if (!this->isOk())
    {
        return nullptr;
    }
    if (!m_textOut->isOk())
    {
        return nullptr;
    }
    CPageInfo *page_info = new CPageInfo;
    page_info->width = m_doc->getPageMediaWidth(page);
    page_info->height = m_doc->getPageMediaHeight(page);
    this->dumpText(page, default_resolution * 2, *page_info);
    return page_info;
}

void ObjectDumper::dumpText(int page, int resolution, CPageInfo &page_info)
{
    m_doc->displayPage(m_textOut, page, resolution, resolution, 0, true, false, false);

    std::vector<TextFlow *> textFlows;
    for (TextFlow *flow = m_textOut->getFlows(); flow; flow = flow->getNext())
    {
        textFlows.push_back(flow);
    }
    page_info.flowLen = textFlows.size();
    page_info.flows = new CTextFlow[textFlows.size()];
    for (size_t flowIdx = 0; flowIdx < textFlows.size(); flowIdx++)
    {
        auto cFlow = page_info.flows[flowIdx];
        auto flow = textFlows[flowIdx];

        std::vector<TextBlock *> textBlocks;
        for (TextBlock *blk = flow->getBlocks(); blk; blk = blk->getNext())
        {
            textBlocks.push_back(blk);
        }
        cFlow.blockLen = textBlocks.size();
        cFlow.blocks = new CTextBlock[textBlocks.size()];
        for (size_t blkIdx = 0; blkIdx < textBlocks.size(); blkIdx++)
        {
            auto cBlock = cFlow.blocks[blkIdx];
            auto blk = textBlocks[blkIdx];

            blk->getBBox(&cBlock.xMin, &cBlock.yMin, &cBlock.xMax, &cBlock.yMax);

            std::vector<TextLine *> textLines;
            for (TextLine *line = blk->getLines(); line; line = line->getNext())
            {
                textLines.push_back(line);
            }
            cBlock.lineLen = textLines.size();
            cBlock.lines = new CTextLine[textLines.size()];
            for (size_t lineIdx = 0; lineIdx < textLines.size(); lineIdx++)
            {
                auto cLine = cBlock.lines[lineIdx];
                auto line = textLines[lineIdx];
                this->dumpLineText(line, cLine);
            }
        }
    }
}

void ObjectDumper::dumpLineText(TextLine *line, CTextLine &cLine)
{
    char buf[8];

    std::vector<TextWord *> textWords;
    for (TextWord *word = line->getWords(); word; word = word->getNext())
    {
        textWords.push_back(word);
    }
    cLine.wordLen = textWords.size();
    cLine.words = new CTextWord[textWords.size()];
    cLine.xMin = cLine.yMin = cLine.xMax = cLine.yMax = 0;

    for (size_t wordIdx = 0; wordIdx < textWords.size(); wordIdx++)
    {
        auto cWord = cLine.words[wordIdx];
        auto word = textWords[wordIdx];
        word->getBBox(&cWord.xMin, &cWord.yMin, &cWord.xMax, &cWord.yMax);
        word->getColor(&cWord.color.r, &cWord.color.g, &cWord.color.b);
        cWord.fontSize = word->getFontSize();
        cWord.rotation = word->getRotation();
        cWord.spaceAfter = word->getSpaceAfter();

        if (cLine.xMin == 0 || cLine.xMin > cWord.xMin)
            cLine.xMin = cWord.xMin;
        if (cLine.yMin == 0 || cLine.yMin > cWord.yMin)
            cLine.yMin = cWord.yMin;
        if (cLine.xMax < cWord.xMax)
            cLine.xMax = cWord.xMax;
        if (cLine.yMax < cWord.yMax)
            cLine.yMax = cWord.yMax;

        cWord.charLen = word->getLength();
        cWord.chars = new CTextChar[cWord.charLen];

        for (size_t charIdx = 0; charIdx < word->getLength(); charIdx++)
        {
            CTextChar textChar = cWord.chars[charIdx];
            const Unicode *u = word->getChar(charIdx);

            textChar.len = m_uMap->mapUnicode(*u, buf, sizeof(buf));
            textChar.bytes = new char[textChar.len];
            memcpy(textChar.bytes, buf, textChar.len);
            // std::cout << std::string(textChar.bytes, textChar.len);
            // std::cout << buf << std::endl;
            word->getCharBBox(charIdx, &textChar.xMax, &textChar.yMin, &textChar.xMax, &textChar.yMax);
        }
    }
}

static void getCropSize(double page_w, double page_h, double *width, double *height)
{
    // int w = crop_w;
    // int h = crop_h;

    // if (w == 0)
    //     w = (int)ceil(page_w);

    // if (h == 0)
    //     h = (int)ceil(page_h);

    // *width = (crop_x + w > page_w ? (int)ceil(page_w - crop_x) : w);
    // *height = (crop_y + h > page_h ? (int)ceil(page_h - crop_y) : h);
    int w = (int)ceil(page_w);
    int h = (int)ceil(page_h);
    *width = (w > page_w ? (int)ceil(page_w) : w);
    *height = (h > page_h ? (int)ceil(page_h) : h);
}

static inline bool is_printing(ImageFormat format)
{
    if (format == fmtPng || format == fmtJpeg || format == fmtTiff)
    {
        return false;
    }
    else
    {
        return true;
    }
}

static cairo_status_t writeStream(void *closure, const unsigned char *data, unsigned int length)
{
    FILE *file = (FILE *)closure;

    if (fwrite(data, length, 1, file) == 1)
        return CAIRO_STATUS_SUCCESS;
    else
        return CAIRO_STATUS_WRITE_ERROR;
}

void ObjectDumper::beginDocument(ImageFormat format, double w, double h)
{
    if (is_printing(format))
    {
        if (format == fmtSvg)
        {
            m_surface = cairo_svg_surface_create_for_stream(writeStream, output_file, w, h);
            cairo_svg_surface_restrict_to_version(m_surface, CAIRO_SVG_VERSION_1_2);
        }
    }
}

void ObjectDumper::beginPage(ImageFormat format, double resolution, double *w, double *h)
{
    if (is_printing(format))
    {

        cairo_surface_set_fallback_resolution(m_surface, resolution, resolution);
    }
    else
    {
        m_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, ceil(*w), ceil(*h));
    }
}

void ObjectDumper::renderPage(ImageFormat format, double resolution, PDFDoc *doc, CairoOutputDev *cairoOut, int pg,
                              double page_w, double page_h,
                              double output_w, double output_h)
{
    cairo_t *cr;
    cairo_status_t status;
    cairo_matrix_t m;

    cr = cairo_create(m_surface);

    cairoOut->setCairo(cr);
    cairoOut->setPrinting(is_printing(format));
    cairoOut->setAntialias(CAIRO_ANTIALIAS_DEFAULT);

    cairo_save(cr);

    // todo
    double crop_x = 0, crop_y = 0;
    cairo_translate(cr, -crop_x, -crop_y);
    if (is_printing(format))
    {
        double cropped_w, cropped_h;
        getCropSize(page_w, page_h, &cropped_w, &cropped_h);
        getFitToPageTransform(cropped_w, cropped_h, output_w, output_h, &m);
        cairo_transform(cr, &m);
        cairo_rectangle(cr, crop_x, crop_y, cropped_w, cropped_h);
        cairo_clip(cr);
    }
    else
    {
        cairo_scale(cr, resolution / 72.0, resolution / 72.0);
    }
    doc->displayPageSlice(cairoOut,
                          pg,
                          72.0, 72.0,
                          0,     /* rotate */
                          true,  /* useMediaBox */
                          false, /* Crop */
                          is_printing(format),
                          -1, -1, -1, -1);
    cairo_restore(cr);
    cairoOut->setCairo(nullptr);

    // Blend onto white page, 不支持半透明的设置为白底。
    if (!is_printing(format) && format != fmtPng && format != fmtTiff)
    {
        cairo_save(cr);
        cairo_set_operator(cr, CAIRO_OPERATOR_DEST_OVER);
        cairo_set_source_rgb(cr, 1, 1, 1);
        cairo_paint(cr);
        cairo_restore(cr);
    }

    status = cairo_status(cr);
    if (status)
        fprintf(stderr, "cairo error: %s\n", cairo_status_to_string(status));
    cairo_destroy(cr);
}

static void writePageImage(ImageFormat format， GooString *filename)
{
    ImgWriter *writer = nullptr;
    FILE *file;
    int height, width, stride;
    unsigned char *data;

    if (format == fmtPng)
    {
        // if (transp)
        writer = new PNGWriter(PNGWriter::RGBA);
        // else if (gray)
        //     writer = new PNGWriter(PNGWriter::GRAY);
        // else if (mono)
        //     writer = new PNGWriter(PNGWriter::MONOCHROME);
        // else
        //     writer = new PNGWriter(PNGWriter::RGB);
    }
    else if (format == fmtJpeg)
    {

        writer = new JpegWriter(JpegWriter::RGB);

        static_cast<JpegWriter *>(writer)->setOptimize(jpegOptimize);
        static_cast<JpegWriter *>(writer)->setProgressive(jpegProgressive);
        if (jpegQuality >= 0)
            static_cast<JpegWriter *>(writer)->setQuality(jpegQuality);
    }

    if (!writer)
        return;

    if (filename->cmp("fd://0") == 0)
        file = stdout;
    else
        file = fopen(filename->c_str(), "wb");

    if (!file)
    {
        fprintf(stderr, "Error opening output file %s\n", filename->c_str());
        exit(2);
    }

    height = cairo_image_surface_get_height(surface);
    width = cairo_image_surface_get_width(surface);
    stride = cairo_image_surface_get_stride(surface);
    cairo_surface_flush(surface);
    data = cairo_image_surface_get_data(surface);

    if (!writer->init(file, width, height, x_resolution, y_resolution))
    {
        fprintf(stderr, "Error writing %s\n", filename->c_str());
        exit(2);
    }
    unsigned char *row = (unsigned char *)gmallocn(width, 4);

    for (int y = 0; y < height; y++)
    {
        uint32_t *pixel = reinterpret_cast<uint32_t *>((data + y * stride));
        unsigned char *rowp = row;
        int bit = 7;
        for (int x = 0; x < width; x++, pixel++)
        {
            if (transp)
            {
                if (tiff)
                {
                    // RGBA premultipled format
                    *rowp++ = (*pixel & 0xff0000) >> 16;
                    *rowp++ = (*pixel & 0x00ff00) >> 8;
                    *rowp++ = (*pixel & 0x0000ff) >> 0;
                    *rowp++ = (*pixel & 0xff000000) >> 24;
                }
                else
                {
                    // unpremultiply into RGBA format
                    uint8_t a;
                    a = (*pixel & 0xff000000) >> 24;
                    if (a == 0)
                    {
                        *rowp++ = 0;
                        *rowp++ = 0;
                        *rowp++ = 0;
                    }
                    else
                    {
                        *rowp++ = (((*pixel & 0xff0000) >> 16) * 255 + a / 2) / a;
                        *rowp++ = (((*pixel & 0x00ff00) >> 8) * 255 + a / 2) / a;
                        *rowp++ = (((*pixel & 0x0000ff) >> 0) * 255 + a / 2) / a;
                    }
                    *rowp++ = a;
                }
            }
            else if (gray || mono)
            {
                // convert to gray
                // The PDF Reference specifies the DeviceRGB to DeviceGray conversion as
                // gray = 0.3*red + 0.59*green + 0.11*blue
                const int r = (*pixel & 0x00ff0000) >> 16;
                const int g = (*pixel & 0x0000ff00) >> 8;
                const int b = (*pixel & 0x000000ff) >> 0;
                // an arbitrary integer approximation of .3*r + .59*g + .11*b
                const int grayValue = (r * 19661 + g * 38666 + b * 7209 + 32829) >> 16;
                if (mono)
                {
                    if (bit == 7)
                        *rowp = 0;
                    if (grayValue > 127)
                        *rowp |= (1 << bit);
                    bit--;
                    if (bit < 0)
                    {
                        bit = 7;
                        rowp++;
                    }
                }
                else
                {
                    *rowp++ = grayValue;
                }
            }
            else
            {
                // copy into RGB format
                *rowp++ = (*pixel & 0x00ff0000) >> 16;
                *rowp++ = (*pixel & 0x0000ff00) >> 8;
                *rowp++ = (*pixel & 0x000000ff) >> 0;
            }
        }
        writer->writeRow(&row);
    }
    gfree(row);
    writer->close();
    delete writer;
    if (file == stdout)
        fflush(file);
    else
        fclose(file);
}

void ObjectDumper::endPage(ImageFormat format, GooString *imageFileName)
{
    cairo_status_t status;

    if (is_printing(format))
    {
        cairo_surface_show_page(m_surface);
    }
    else
    {
        writePageImage(format, imageFileName);
        cairo_surface_finish(m_surface);
        status = cairo_surface_status(m_surface);
        if (status)
            fprintf(stderr, "cairo error: %s\n", cairo_status_to_string(status));
        cairo_surface_destroy(m_surface);
    }
}

void ObjectDumper::endDocument(ImageFormat format)
{
    cairo_status_t status;

    if (is_printing(format))
    {
        cairo_surface_finish(m_surface);
        status = cairo_surface_status(m_surface);
        if (status)
            fprintf(stderr, "cairo error: %s\n", cairo_status_to_string(status));
        cairo_surface_destroy(m_surface);

        // if (output_file)
        //     fclose(output_file);
    }
}

void ObjectDumper::cropImage(unsigned int page, int resolution, double xMin, double yMin, double xMax, double yMax, ImageFormat format)
{

    double x_resolution = 150.0, y_resolution = 150.0;
    double resolution = 150.0;
    bool printing = false;
    double pg_w = 0, pg_h = 0;

    if (format == fmtPng || format == fmtJpeg || format == fmtTiff)
    {
        printing = false;
    }
    else
    {
        printing = true;
    }

    m_cairoOut->startDoc(m_doc);

    // PDF 页面大小
    pg_w = m_doc->getPageMediaWidth(page);
    pg_h = m_doc->getPageMediaHeight(page);

    if (printing && m_paperWidth < 0 && m_paperHeight < 0)
    {
        m_paperWidth = (int)ceil(pg_w);
        m_paperHeight = (int)ceil(pg_h);
    }

    if ((m_doc->getPageRotate(page) == 90) || (m_doc->getPageRotate(page) == 270))
    {
        double tmp = pg_w;
        pg_w = pg_h;
        pg_h = tmp;
    }
    // if (scaleTo != 0)
    // {
    // resolution = (72.0 * scaleTo) / (pg_w > pg_h ? pg_w : pg_h);
    // x_resolution = y_resolution = resolution;
    // }

    // 真实输出的画布大小.
    double output_w, output_h;

    // 对于不可缩放格式,使用原始大小.
    if (printing)
    {
        output_w = pg_w;
        output_w = pg_h;
    }
    else
    {
        // getCropSize(pg_w * (x_resolution / 72.0),
        //             pg_h * (y_resolution / 72.0),
        //             &output_w, &output_h);
        output_w = (int)ceil(pg_w * (x_resolution / 72.0));
        output_h = (int)ceil(pg_h * (y_resolution / 72.0));
    }

    // if (page == firstPage)
    beginDocument(format, pg_w, pg_h);
    beginPage(format, resolution, &output_w, &output_h);
    renderPage(format, resolution, m_doc, m_cairoOut, page, pg_w, pg_h, output_w, output_h);
    endPage('test');

    endDocument();
}
