## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir). "\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	$(LIBPNG_CFLAGS) \
	$(LIBEXIF_CFLAGS) \
	$(JSONGLIB1_CFLAGS) \
	$(GTKMM_CFLAGS) \
	$(GLIBMM_CFLAGS)

libgenericimg_la_CFLAGS = \
	-std=c17 -pedantic -Wall -Wconversion

lib_LTLIBRARIES = libgenericimg.la

libgenericimg_la_CXXFLAGS = \
	-pedantic -Wall -Wconversion

libgenericimg_la_LDFLAGS = \
	@EXTRA_LDFLAGS@ \
	-version-info @version_info@

libgenericimg_la_SOURCES = \
	resources.c \
	ConcurrentCollections.cpp \
	LocaleContext.cpp \
	ImageUtils.cpp \
	ImageView.cpp \
	BinModel.cpp \
	BinView.cpp \
	ImageOptions.cpp \
	ImageFileChooser.cpp \
	ImageOptionDialog.cpp \
	ImageArea.cpp \
	ExifReader.cpp \
	ImageList.cpp \
	DisplayImage.cpp \
	ApplicationSupport.cpp \
	Mode.cpp \
	JsonHelper.cpp \
	Log.cpp \
	LogImpl.cpp \
	AbstractTableManager.cpp \
	KeyfileTableManager.cpp \
	TableProperties.cpp \
	StringUtils.cpp \
	KeyConfig.cpp

libgenericimg_la_LIBADD =  \
	$(LIBPNG_LIBS) \
	$(LIBEXIF_LIBS) \
	$(JSONGLIB1_LIBS) \
	$(GTKMM_LIBS) \
	$(GLIBMM_LIBS)


## using pkginclude_HEADERS will use include dir with packagename
##   (include_HEADERS  will use #install#/include)
pkginclude_HEADERS = \
	ConcurrentCollections.hpp \
	LocaleContext.hpp \
	ImageUtils.hpp \
	ImageView.hpp \
	BinModel.hpp \
	BinView.hpp \
	ImageOptions.hpp \
	ImageFileChooser.hpp \
	ImageOptionDialog.hpp \
	ImageArea.hpp \
	ExifReader.hpp \
	ImageList.hpp \
	DisplayImage.hpp \
	ApplicationSupport.hpp \
	Mode.hpp \
	JsonHelper.hpp \
	Log.hpp \
	LogImpl.hpp \
	AbstractTableManager.hpp \
	KeyfileTableManager.hpp \
	TableProperties.hpp \
	StringUtils.hpp \
	KeyConfig.hpp

if SYSDLOG
libgenericimg_la_LIBADD += $(SYSTEMDLOG_LIBS)
AM_CPPFLAGS += $(SYSTEMDLOG_CFLAGS)

endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = genericimg.pc

EXTRA_DIST = \
	genericimg.pc.in
