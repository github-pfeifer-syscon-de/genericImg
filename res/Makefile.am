PKGCONFIG = pkg-config
GLIB_COMPILE_RESOURCES = $(shell $(PKGCONFIG) --variable=glib_compile_resources gio-2.0)

GENERICIMG_RESOURCE_XML=$(srcdir)/genericimg.gresources.xml
GENERICIMG_RESOURCES=$(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir) --generate-dependencies $(GENERICIMG_RESOURCE_XML))
RESOURCES_C=$(top_srcdir)/src/resources.c

all:$(RESOURCES_C)

$(RESOURCES_C): $(GENERICIMG_RESOURCE_XML) $(GENERICIMG_RESOURCES)
	$(GLIB_COMPILE_RESOURCES) --target=$(RESOURCES_C) --sourcedir=$(srcdir) --generate-source $(GENERICIMG_RESOURCE_XML)

# maybe you want to use ua1r8a.afm ua1r8a.pfb here ???
#   e.g. check https://ctan.org/tex-archive/fonts/urw/arial
dist_pkgdata_DATA =

EXTRA_DIST = $(MONGLMM_RESOURCES) $(MONGLMM_RESOURCE_XML) $(dist_pkgdata_DATA)

clean-local:
	-rm -rf $(RESOURCES_C)
