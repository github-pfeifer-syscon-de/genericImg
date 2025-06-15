## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir). "\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	$(LIBFMT_CFLAGS) \
	$(LIBPNG_CFLAGS) \
	$(LIBEXIF_CFLAGS) \
	$(JSONGLIB1_CFLAGS) \
	$(GTKMM_CFLAGS) \
	$(LIBFMT_CFLAGS) \
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
	JsonObj.cpp \
	Log.cpp \
	LogImpl.cpp \
	LogView.cpp \
	LogViewSysd.cpp \
	LogViewFile.cpp \
	LogViewSyslog.cpp \
	DateUtils.cpp \
	AbstractTableManager.cpp \
	KeyfileTableManager.cpp \
	TableProperties.cpp \
	StringUtils.cpp \
	ThreadWorker.cpp \
	TreeNodeModel.cpp \
	KeyConfig.cpp

libgenericimg_la_LIBADD =  \
	$(LIBFMT_LIBS) \
	$(LIBPNG_LIBS) \
	$(LIBEXIF_LIBS) \
	$(JSONGLIB1_LIBS) \
	$(GTKMM_LIBS) \
	$(LIBFMT_LIBS) \
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
	JsonObj.hpp \
	Log.hpp \
	LogImpl.hpp \
	LogView.hpp \
	LogViewSysd.hpp \
	LogViewFile.hpp \
	LogViewSyslog.hpp \
	DateUtils.hpp \
	psc_format.hpp \
	psc_i18n.hpp \
	AbstractTableManager.hpp \
	KeyfileTableManager.hpp \
	TableProperties.hpp \
	StringUtils.hpp \
	ThreadWorker.hpp \
	TreeNodeModel.hpp \
	PdfExport.hpp \
	PdfFont.hpp \
	PdfImage.hpp \
	PdfPage.hpp \
	KeyConfig.hpp

if SYSDLOG
libgenericimg_la_LIBADD += $(SYSTEMDLOG_LIBS)
AM_CPPFLAGS += $(SYSTEMDLOG_CFLAGS)
endif

if PDF_LIBHARU
libgenericimg_la_SOURCES += \
	PdfExport.cpp \
	PdfFont.cpp \
	PdfImage.cpp \
	PdfPage.cpp
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = genericimg.pc

EXTRA_DIST = \
	genericimg.pc.in
