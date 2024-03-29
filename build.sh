#!/bin/sh

# export CFLAGS=" -fPIC -static -static-libgcc -static-libstdc++ "

export CFLAGS=" -fPIC -static -static-libstdc++ "
export CXXFLAGS=" -fPIC -static -static-libstdc++ "
export CPPFLAGS=" -fPIC -static -static-libstdc++ "
export LDFLAGS=" -fPIC -L/usr/local/lib -L/usr/lib64 -static -static-libstdc++ "

# export CFLAGS=" -fPIC  -static-libstdc++"
# export CXXFLAGS=" -fPIC  -static-libstdc++"
# export CPPFLAGS=" -fPIC  -static-libstdc++"
# export LDFLAGS=" -fPIC -L/usr/local/lib -L/usr/lib64  -static-libstdc++ "

# export LDFLAGS=" $LDFLAGS -lexpat -lfontconfig -lfreetype"

basedir=$(cd `dirname $0`; pwd)

cd $basedir
tar -axvf ../packages/zlib-1.2.11.tar.gz \
    && mv zlib* zlib && cd zlib \
    && ./configure --static && make \
    && sudo make install 

cd $basedir
tar -axvf ../packages/jpegsrc.v9c.tar.gz \
    && mv jpeg* jpeg && cd jpeg && ./configure --enable-static \
    && make -j 10 && sudo make install

cd $basedir
tar -axvf ../packages/openjpeg2-v2.3.1.tar.gz \
    && mv openjpeg* openjpeg && mkdir openjpeg/build && cd openjpeg/build \
    && cmake .. -DBUILD_SHARED_LIBS=OFF && make && sudo make install

cd $basedir
tar -axvf ../packages/libpng-1.6.37.tar.gz \
    && mv libpng* libpng && cd libpng && ./configure --enable-static \
    && make -j 10 && sudo make install

# -------------------
cd $basedir
tar -axvf ../packages/lcms2-2.9.tar.gz \
    && mv lcms2* lcms2 && cd lcms2 \
    && ./configure --enable-static \
    && make -j 10 && sudo make install 

# cd $basedir
# tar -axvf ../packages/tiff-4.0.9.tar.gz \
#     && mv tiff* tiff && cd tiff \
#     && ./configure --enable-static \
#     && make -j 10 && sudo make install 

# -------------------

cd $basedir
tar -axvf ../packages/freetype-2.10.1.tar.gz \
    && mv freetype* freetype && cd freetype \
    && ./configure --enable-static \
    && make -j 10 && sudo make install

cd $basedir
tar xf ../packages/expat-2.2.7.tar.gz \
    && mv expat* expat && cd expat \
    && ./configure --enable-static \
    && make -j 10 && sudo make install

cd $basedir
tar xf ../packages/gperf-3.1.tar.gz \
    && mv gperf* gperf && cd gperf \
    && ./configure \
    && make -j 10 && sudo make install

cd $basedir
tar xf ../packages/fontconfig-2.12.6.tar.gz \
    && mv fontconfig* fontconfig && cd fontconfig \
    && ./configure --enable-static \
    && make -j 10 && sudo make install

cd $basedir
tar -axvf ../packages/pixman-0.38.2.tar.gz \
    && mv pixman* pixman && cd pixman \
    && ./configure --enable-static \
    && make -j 10 && sudo make install

export CFLAGS=" -fPIC  -static-libstdc++"
export CXXFLAGS=" -fPIC  -static-libstdc++"
export CPPFLAGS=" -fPIC  -static-libstdc++"
export LDFLAGS=" -fPIC -L/usr/local/lib -L/usr/lib64  -static-libstdc++ "

# export CFLAGS=" "
# export CXXFLAGS=" "
# export CPPFLAGS=" "
# export LDFLAGS="  "

cd $basedir
tar -axvf ../packages/cairo-1.14.12.tar.gz \
    && mv cairo* cairo && cd cairo \
    && ./configure --enable-static --enable-xlib=no --enable-xcb=no --enable-pdf=no \
    && make -j 10 && sudo make install

cd $basedir
tar -axvf ../packages/poppler-0.79.0.tar.gz \
    && mv poppler* poppler \
    && mkdir poppler/build && cd poppler/build \
    && cmake -DENABLE_UTILS=OFF -DENABLE_CPP=OFF -DENABLE_GLIB=OFF -DENABLE_QT5=OFF -DENABLE_LIBCURL=OFF -DBUILD_SHARED_LIBS=OFF .. \
    && make -j 10 && sudo make install
