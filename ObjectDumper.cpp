
#include "ObjectDumper.h"
#include "CDumpApi.h"
#include "poppler/Page.h"
#include "poppler/PDFDoc.h"
#include "poppler/PDFDocFactory.h"
#include "poppler/TextOutputDev.h"
#include "poppler/GlobalParams.h"
#include "goo/GooString.h"

#include "poppler/CairoOutputDev.h"
#include "CairoOutputDevNoText.h"
// #include "goo/ImgWriter.h"

#include <cairo/cairo.h>
#include <cairo/cairo-svg.h>

#include <string>
#include <vector>
#include <math.h>
#include <iostream>

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
    m_cairoOut = new CairoOutputDevNoText();

    m_uMap = globalParams->getTextEncoding();
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
    page_info->graph = this->dumpGraph(page);
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

CGraphSvg ObjectDumper::dumpGraph(unsigned int page)
{

    double x_resolution = 150.0, y_resolution = 150.0;
    double resolution = 150.0;
    bool printing = true;
    double pg_w = 0, pg_h = 0;

    ImageFormat format = fmtSvg;

    m_cairoOut->startDoc(m_doc);

    // PDF 页面大小
    pg_w = m_doc->getPageMediaWidth(page);
    pg_h = m_doc->getPageMediaHeight(page);

    if (m_paperWidth < 0 && m_paperHeight < 0)
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
    output_w = pg_w;
    output_h = pg_h;

    // beginPage(format, resolution, &output_w, &output_h);

    FILE *output_file = tmpfile();

    cairo_surface_t *surface = cairo_svg_surface_create_for_stream(writeStream, output_file, output_w, output_h);
    cairo_svg_surface_restrict_to_version(surface, CAIRO_SVG_VERSION_1_2);
    cairo_surface_set_fallback_resolution(surface, resolution, resolution);

    // renderPage(format, resolution, m_doc, m_cairoOut, page, pg_w, pg_h, output_w, output_h);
    cairo_t *cr;
    cairo_status_t status;
    cairo_matrix_t m;

    cr = cairo_create(surface);

    m_cairoOut->setCairo(cr);
    m_cairoOut->setPrinting(true);
    m_cairoOut->setAntialias(CAIRO_ANTIALIAS_DEFAULT);

    cairo_save(cr);

    cairo_translate(cr, 0, 0);

    // double cropped_w, cropped_h;
    // getCropSize(page_w, page_h, &cropped_w, &cropped_h);
    // getFitToPageTransform(cropped_w, cropped_h, output_w, output_h, &m);
    cairo_matrix_init_identity(&m);
    cairo_matrix_scale(&m, 1.0, 1.0);
    cairo_transform(cr, &m);
    std::cout << "output_w/h:" << output_w << ", " << output_h << std::endl;
    cairo_rectangle(cr, 0, 0, output_w, output_h);
    cairo_clip(cr);

    m_doc->displayPageSlice(m_cairoOut,
                            page,
                            72.0, 72.0,
                            0,     /* rotate */
                            true,  /* useMediaBox */
                            false, /* Crop */
                            true,
                            -1, -1, -1, -1);
    cairo_restore(cr);
    m_cairoOut->setCairo(nullptr);

    status = cairo_status(cr);
    if (status)
        fprintf(stderr, "cairo error: %s\n", cairo_status_to_string(status));
    cairo_destroy(cr);

    cairo_surface_show_page(surface);

    cairo_surface_finish(surface);
    status = cairo_surface_status(surface);
    if (status)
        fprintf(stderr, "cairo error: %s\n", cairo_status_to_string(status));
    cairo_surface_destroy(surface);

    CGraphSvg graph;
    graph.size = ftell(output_file);
    if (graph.size <= 0)
    {
        graph.content = nullptr;
        return graph;
    }
    graph.content = new char[graph.size + 1];
    rewind(output_file);
    fread(graph.content, graph.size, 1, output_file);
    graph.content[graph.size] = '\0';
    fclose(output_file);
    return graph;
}
