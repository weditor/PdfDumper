
#include "ObjectDumper.h"
#include "CDumpApi.h"
#include "poppler/Page.h"
#include "poppler/PDFDoc.h"
#include "poppler/PDFDocFactory.h"
#include "poppler/TextOutputDev.h"
#include "poppler/GlobalParams.h"
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

    m_uMap = globalParams->getTextEncoding();
}

ObjectDumper::~ObjectDumper()
{
    m_uMap->decRefCnt();
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
