
#include "poppler/CairoOutputDev.h"

class CairoOutputDevNoText : public CairoOutputDev
{
public:
    CairoOutputDevNoText() : CairoOutputDev() {}
    virtual ~CairoOutputDevNoText() {}

    void drawChar(GfxState *state, double x, double y,
                  double dx, double dy,
                  double originX, double originY,
                  CharCode code, int nBytes, Unicode *u, int uLen) override {}
    bool beginType3Char(GfxState *state, double x, double y,
                        double dx, double dy,
                        CharCode code, Unicode *u, int uLen) override { return false; }
    void endType3Char(GfxState *state) override {}
    void beginTextObject(GfxState *state) override {}
    void endTextObject(GfxState *state) override {}
    bool interpretType3Chars() override { return false; }
};
