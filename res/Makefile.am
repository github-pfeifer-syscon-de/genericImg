PKGCONFIG = pkg-config
GLIB_COMPILE_RESOURCES = $(shell $(PKGCONFIG) --variable=glib_compile_resources gio-2.0)

GENERICIMG_RESOURCE_XML=genericimg.gresources.xml
GENERICIMG_RESOURCES=$(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=. --generate-dependencies $(GENERICIMG_RESOURCE_XML))
resources_c=../src/resources.c

all:$(resources_c)

$(resources_c): $(GENERICIMG_RESOURCE_XML) $(GENERICIMG_RESOURCES)
	$(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=$(dir $<) --generate-source $<

dist_pkgdata_DATA = ua1r8a.afm ua1r8a.pfb

EXTRA_DIST = $(MONGLMM_RESOURCES) $(MONGLMM_RESOURCE_XML) $(dist_pkgdata_DATA)

clean-local:
	-rm -rf $(resources_c)
