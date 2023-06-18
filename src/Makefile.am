## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir). "\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
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
	ConcurrentCollections.cpp \
	LocaleContext.cpp \
	StringUtils.cpp

libgenericimg_la_LIBADD =  \
	$(GLIBMM_LIBS)


## using pkginclude_HEADERS will use include dir with packagename
##   (include_HEADERS  will use #install#/include)
pkginclude_HEADERS = \
	ConcurrentCollections.hpp \
	LocaleContext.hpp \
	StringUtils.hpp

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = genericimg-$(VERSION).pc

EXTRA_DIST = \
	genericimg-$(VERSION).pc.in
