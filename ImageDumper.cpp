
#include "ImageDumper.h"
#include "poppler/Page.h"
#include "poppler/PDFDoc.h"
#include "poppler/CairoOutputDev.h"

#include "goo/ImgWriter.h"
#include "goo/JpegWriter.h"
#include "goo/PNGWriter.h"
// #include "goo/TiffWriter.h"

#include <cairo/cairo.h>
#include <math.h>

ImageDumpper::ImageDumpper(PDFDoc *doc, ImageFormat format) : m_format(format), m_doc(doc)
{
    m_cairoOut = new CairoOutputDev();
}
ImageDumpper::~ImageDumpper() {}

void ImageDumpper::cropImage(const char *filename, unsigned int page, double resolution, int xMin, int yMin, int xMax, int yMax)
{
    // double resolution = 72.0;
    // double resolution = 150.0;
    // bool printing = false;
    double pg_w = 0, pg_h = 0;
    double scale = resolution / 72.0;

    m_cairoOut->startDoc(m_doc);

    // PDF 页面大小
    pg_w = m_doc->getPageMediaWidth(page);
    pg_h = m_doc->getPageMediaHeight(page);

    // if (printing && m_paperWidth < 0 && m_paperHeight < 0)
    // {
    //     m_paperWidth = (int)ceil(pg_w);
    //     m_paperHeight = (int)ceil(pg_h);
    // }

    if ((m_doc->getPageRotate(page) == 90) || (m_doc->getPageRotate(page) == 270))
    {
        double tmp = pg_w;
        pg_w = pg_h;
        pg_h = tmp;
    }

    int w = xMax;
    int h = yMax;

    if (xMax == 0)
        xMax = (int)ceil(pg_w * scale);

    if (yMax == 0)
        yMax = (int)ceil(pg_h * scale);

    // 真实输出的画布大小.
    double output_w = xMax > pg_w * scale ? (pg_w * scale - xMin) : (xMax - xMin);
    double output_h = yMax > pg_h * scale ? (pg_h * scale - yMin) : (yMax - yMin);

    // beginDocument(format, pg_w, pg_h);
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, ceil(output_w), ceil(output_h));
    // renderPage(format, resolution, m_doc, m_cairoOut, page, pg_w, pg_h, output_w, output_h);

    // ---------------------------------------------------------------
    // cairo_t *cr;
    cairo_status_t status;
    cairo_matrix_t m;

    cairo_t *cr = cairo_create(surface);

    m_cairoOut->setCairo(cr);
    m_cairoOut->setPrinting(false);
    m_cairoOut->setAntialias(CAIRO_ANTIALIAS_DEFAULT);

    cairo_save(cr);

    // todo
    // double crop_x = 0, crop_y = 0;
    cairo_translate(cr, -xMin, -yMin);

    cairo_scale(cr, scale, scale);

    m_doc->displayPageSlice(m_cairoOut,
                            page,
                            72.0, 72.0,
                            0,     /* rotate */
                            true,  /* useMediaBox */
                            false, /* Crop */
                            false,
                            -1, -1, -1, -1);
    cairo_restore(cr);
    m_cairoOut->setCairo(nullptr);

    // Blend onto white page, 不支持半透明的设置为白底。
    if (m_format != fmtPng && m_format != fmtTiff)
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
    // ----------------------------------------------------------

    // cairo_status_t status;

    writePageImage(filename, surface, resolution);
    cairo_surface_finish(surface);
    status = cairo_surface_status(surface);
    if (status)
        fprintf(stderr, "cairo error: %s\n", cairo_status_to_string(status));
    cairo_surface_destroy(surface);

    // ------------------------------
}

bool ImageDumpper::is_transp()
{
    return m_format == fmtPng || m_format == fmtTiff;
}

void ImageDumpper::writePageImage(const char *filename, void *_surface, double resolution)
{
    cairo_surface_t *surface = (cairo_surface_t *)_surface;
    ImgWriter *writer = nullptr;
    FILE *file;
    int height, width, stride;
    unsigned char *data;

    if (m_format == fmtPng)
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
    else if (m_format == fmtJpeg)
    {

        writer = new JpegWriter(JpegWriter::RGB);

        static_cast<JpegWriter *>(writer)->setOptimize(true);
        static_cast<JpegWriter *>(writer)->setProgressive(false);
        // if (jpegQuality >= 0)
        //     static_cast<JpegWriter *>(writer)->setQuality(jpegQuality);
    }

    if (!writer)
        return;

    file = fopen(filename, "wb");

    if (!file)
    {
        fprintf(stderr, "Error opening output file %s\n", filename);
        exit(2);
    }

    height = cairo_image_surface_get_height(surface);
    width = cairo_image_surface_get_width(surface);
    stride = cairo_image_surface_get_stride(surface);
    cairo_surface_flush(surface);
    data = cairo_image_surface_get_data(surface);

    // if (!writer->init(file, width, height, x_resolution, y_resolution))
    if (!writer->init(file, width, height, resolution, resolution))
    {
        fprintf(stderr, "Error writing %s\n", filename);
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
            if (is_transp())
            {
                if (m_format == fmtTiff)
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
            // else if (gray || mono)
            // {
            //     // convert to gray
            //     // The PDF Reference specifies the DeviceRGB to DeviceGray conversion as
            //     // gray = 0.3*red + 0.59*green + 0.11*blue
            //     const int r = (*pixel & 0x00ff0000) >> 16;
            //     const int g = (*pixel & 0x0000ff00) >> 8;
            //     const int b = (*pixel & 0x000000ff) >> 0;
            //     // an arbitrary integer approximation of .3*r + .59*g + .11*b
            //     const int grayValue = (r * 19661 + g * 38666 + b * 7209 + 32829) >> 16;
            //     if (mono)
            //     {
            //         if (bit == 7)
            //             *rowp = 0;
            //         if (grayValue > 127)
            //             *rowp |= (1 << bit);
            //         bit--;
            //         if (bit < 0)
            //         {
            //             bit = 7;
            //             rowp++;
            //         }
            //     }
            //     else
            //     {
            //         *rowp++ = grayValue;
            //     }
            // }
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
