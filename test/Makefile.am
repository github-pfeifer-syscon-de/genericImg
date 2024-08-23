


TESTS = unic_test

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)."\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	$(GLIBMM_CFLAGS) \
	-I ../src

AM_CFLAGS= \
	-Wall -Wpedantic

AM_CXXFLAGS = \
	-Wall -Wpedantic

AM_LDFLAGS =

unic_test_LDADD =  \
	-L../src/.libs -lgenericimg \
	$(GLIBMM_LIBS)

check_PROGRAMS = unic_test

unic_test_SOURCES = \
    unic_test.cpp


