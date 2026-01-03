


TESTS = unic_test log_test worker_test json_test util_test pdf_test

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)."\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	$(GLIBMM_CFLAGS) \
	$(GTKMM_CFLAGS) \
	$(JSONGLIB1_CFLAGS) \
	$(FONTCONFIG_CFLAGS) \
	-I $(top_srcdir)/src

AM_CFLAGS= \
	-Wall -Wpedantic

AM_CXXFLAGS = \
	-Wall -Wpedantic

AM_LDFLAGS =

check_PROGRAMS = unic_test log_test worker_test json_test util_test pdf_test

unic_test_LDADD =  \
	-L../src/.libs -lgenericimg \
	$(GLIBMM_LIBS)

unic_test_SOURCES = unic_test.cpp

log_test_LDADD =  \
	-L../src/.libs -lgenericimg \
	$(GLIBMM_LIBS)

log_test_SOURCES = log_test.cpp

worker_test_LDADD = \
	-L../src/.libs -lgenericimg \
	$(GLIBMM_LIBS) \
	$(GTKMM_LIBS) \
	$(GENERICIMG_LIBS)

worker_test_SOURCES = WorkerTest.cpp

json_test_LDADD = \
	-L../src/.libs -lgenericimg \
	$(GLIBMM_LIBS) \
	$(JSONGLIB1_LIBS)
	$(GENERICIMG_LIBS)

json_test_SOURCES = JsonTest.cpp

util_test_LDADD = \
	-L../src/.libs -lgenericimg \
	$(GLIBMM_LIBS) \
	$(GENERICIMG_LIBS)

util_test_SOURCES = util_test.cpp

pdf_test_LDADD = \
	-L../src/.libs -lgenericimg \
	$(GLIBMM_LIBS) \
        $(FONTCONFIG_LIBS) \
	$(GENERICIMG_LIBS)

pdf_test_SOURCES = pdf_test.cpp