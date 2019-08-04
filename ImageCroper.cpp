
#include "poppler/CairoOutputDev.h"
#include "goo/ImgWriter.h"
#include "goo/JpegWriter.h"
#include "goo/PNGWriter.h"

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

static void getFitToPageTransform(double page_w, double page_h,
                                  double paper_w, double paper_h,
                                  cairo_matrix_t *m)
{
    double x_scale, y_scale, scale;

    x_scale = paper_w / page_w;
    y_scale = paper_h / page_h;
    if (x_scale < y_scale)
        scale = x_scale;
    else
        scale = y_scale;

    if (scale > 1.0 && !expand)
        scale = 1.0;
    if (scale < 1.0 && noShrink)
        scale = 1.0;

    cairo_matrix_init_identity(m);
    if (!noCenter)
    {
        // centre page
        cairo_matrix_translate(m, (paper_w - page_w * scale) / 2, (paper_h - page_h * scale) / 2);
    }
    else if (!svg)
    {
        // move to PostScript origin
        cairo_matrix_translate(m, 0, (paper_h - page_h * scale));
    }
    cairo_matrix_scale(m, scale, scale);
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

static void getCropSize(double page_w, double page_h, double *width, double *height)
{
    int w = crop_w;
    int h = crop_h;

    if (w == 0)
        w = (int)ceil(page_w);

    if (h == 0)
        h = (int)ceil(page_h);

    *width = (crop_x + w > page_w ? (int)ceil(page_w - crop_x) : w);
    *height = (crop_y + h > page_h ? (int)ceil(page_h - crop_y) : h);

    // int w = (int)ceil(page_w);
    // int h = (int)ceil(page_h);
    // *width = (w > page_w ? (int)ceil(page_w) : w);
    // *height = (h > page_h ? (int)ceil(page_h) : h);
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
    //     else if (tiff)
    //     {
    // #ifdef ENABLE_LIBTIFF
    //         if (transp)
    //             writer = new TiffWriter(TiffWriter::RGBA_PREMULTIPLIED);
    //         else if (gray)
    //             writer = new TiffWriter(TiffWriter::GRAY);
    //         else if (mono)
    //             writer = new TiffWriter(TiffWriter::MONOCHROME);
    //         else
    //             writer = new TiffWriter(TiffWriter::RGB);
    //         static_cast<TiffWriter *>(writer)->setCompressionString(tiffCompressionStr);
    // #endif
    //     }
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
