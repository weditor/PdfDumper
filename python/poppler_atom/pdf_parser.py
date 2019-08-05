
from ctypes import *  # NOQA
from os.path import join, abspath, dirname


class CColor(Structure):
    _fields_ = [
        ("r", c_double),
        ("g", c_double),
        ("b", c_double),
    ]


class CTextChar(Structure):
    _fields_ = [
        ("bytes", POINTER(c_char)),
        ("len", c_ubyte),
        ("font", c_int),
        ("xMin", c_double),
        ("yMin", c_double),
        ("xMax", c_double),
        ("xMin", c_double),
    ]


class CTextWord(Structure):
    _fields_ = [
        ("fontSize", c_double),
        ("rotation", c_int),
        ("spaceAfter", c_bool),
        ("color", CColor),
        ("xMin", c_double),
        ("yMin", c_double),
        ("xMax", c_double),
        ("xMin", c_double),
        ("charLen", c_uint),
        ("chars", POINTER(CTextChar)),
    ]


class CTextLine(Structure):
    _fields_ = [
        ("wordLen", c_uint),
        ("words", POINTER(CTextWord)),

        ("xMin", c_double),
        ("yMin", c_double),
        ("xMax", c_double),
        ("xMin", c_double),
    ]


class CTextBlock(Structure):
    _fields_ = [
        ("lineLen", c_uint),
        ("lines", POINTER(CTextLine)),

        ("xMin", c_double),
        ("yMin", c_double),
        ("xMax", c_double),
        ("xMin", c_double),
    ]


class CTextFlow(Structure):
    _fields_ = [
        ("blockLen", c_uint),
        ("blocks", POINTER(CTextBlock)),
    ]


class CGraphSvg(Structure):
    _fields_ = [
        ("size", c_long),
        ("content", POINTER(c_char)),
    ]


class CPageInfo(Structure):
    _fields_ = [
        ("width", c_double),
        ("height", c_double),
        ("flowLen", c_uint),
        ("flows", POINTER(CTextFlow)),
        ("graph", CGraphSvg),
    ]


_dll_path = abspath(join(dirname(__file__), "libpoppler_atom.so"))
pdflib = cdll.LoadLibrary(_dll_path)

# global params initial
init_global_params = pdflib.init_global_params
init_global_params.argtypes = [c_char_p]

destroy_global_params = pdflib.destroy_global_params

# create pdf parser ; get status
create_parser = pdflib.create_parser
create_parser.argtypes = [c_char_p, c_char_p, c_char_p]
create_parser.restype = c_void_p

destroy_parser = pdflib.destroy_parser
destroy_parser.argtypes = [c_void_p]

parser_is_ok = pdflib.parser_is_ok
parser_is_ok.argtypes = [c_void_p]

parser_get_num_pages = pdflib.parser_get_num_pages
parser_get_num_pages.argtypes = [c_void_p]
parser_get_num_pages.restype = c_uint

# do parse, free memory.
parser_parse = pdflib.parser_parse
parser_parse.argtypes = [c_void_p, c_int]
parser_parse.restype = POINTER(CPageInfo)

free_page_info = pdflib.free_page_info
free_page_info.argtypes = [POINTER(CPageInfo)]

# dump image;
get_image_dumper = pdflib.get_image_dumper
get_image_dumper.argtypes = [c_void_p, c_int]
get_image_dumper.restype = c_void_p

crop_page = pdflib.crop_page
crop_page.argtypes = [c_char_p, c_uint, c_double, c_int, c_int, c_int, c_int]

"""
    extern void init_global_params(const char *poppler_data = nullptr);
    extern void destroy_global_params();

    extern void free_page_info(CPageInfo *page_info);

    extern void *create_parser(const char *fileName, const char *owner_pw = nullptr, const char *user_pw = nullptr);
    extern void destroy_parser(void *parser);
    extern bool parser_is_ok(void *parser);
    extern unsigned int parser_get_num_pages(void *parser);
    extern CPageInfo *parser_parse(void *parser, int page);

    extern void *getImageDumper(void *parser, int format);
    extern void *cropPage(const char *filename, unsigned int page, double resolution, int left, int top, int right, int bottom);

"""
