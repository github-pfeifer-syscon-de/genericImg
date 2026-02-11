project('genericimg', 'c', 'cpp'
    , default_options: ['c_std=c11'
                       , 'cpp_std=c++20'
                       , 'buildtype=debugoptimized'
                       , 'warning_level=3']
    , license : 'GPL-3.0'
    , meson_version: '>= 1.9.0'
    , version: '0.4.8')
project_description = 'Common Gui functions for psc projects'

gtkmm3_deps     = dependency('gtkmm-3.0')
gdk3_deps       = dependency('gdk-3.0')
glibmm2_deps    = dependency('glibmm-2.4 giomm-2.4')
libpng_deps     = dependency('libpng')
libexif_deps    = dependency('libexif', version : '>= 0.6.21')
jsonglib1_deps  = dependency('json-glib-1.0', version : '>= 0.8')
fontconfig_deps = dependency('fontconfig')
thread_deps     = dependency('threads')
#uncomment to use libfmt e.g. with gcc < 13 see README.md
#libfmt_deps     = dependency('fmt')

if get_option('log') == 'sysd'
  sysdlog_deps = dependency('libsystemd')
else
  sysdlog_deps = declare_dependency()
endif

cc = meson.get_compiler('c')
libharu_deps = declare_dependency(dependencies : cc.find_library('hpdf', required: false))

pkgdatadir = join_paths(get_option('prefix'), get_option('datadir'), meson.project_name())
# localedir = share/locale
localdir = join_paths(get_option('prefix'), get_option('localedir'))
add_project_arguments('-DPACKAGE_DATA_DIR="' + pkgdatadir + '"', language: 'cpp')
add_project_arguments('-DPACKAGE_LOCALE_DIR="' + localdir + '"', language: 'cpp')
public_headers = include_directories('include')

conf = configuration_data()
conf.set       ('SYSDLOG', get_option('log') == 'sysd')
conf.set       ('SYSLOG', get_option('log') == 'sys')
conf.set_quoted('VERSION', meson.project_version())     # Surround the version in quotes to make it a C string
conf.set       ('USE_PDF', libharu_deps.found())
subdir('include')
subdir('res')
subdir('src')
deps = [ gtkmm3_deps
       , gdk3_deps
       , glibmm2_deps
       , libpng_deps
       , libexif_deps
       , jsonglib1_deps
       , fontconfig_deps
       , thread_deps
       , libharu_deps
       , sysdlog_deps
       ]
# append resource.c
sources += app_resources
project_target = both_libraries(meson.project_name()
    , sources
    , app_resources
    , install : true
    , dependencies: deps
    , gnu_symbol_visibility : 'default'         # visibility was hidden, but selected visible for compatibility
    , include_directories : public_headers
    , version : meson.project_version())
subdir('test')

configure_file ( output : 'config.h'
               , configuration : conf)


pkg_mod = import('pkgconfig')
pkg_mod.generate(
    name : meson.project_name()
  , filebase : meson.project_name()
  , description : project_description
  , subdirs : meson.project_name()
  , libraries : project_target
  , requires : deps
)

# Make this library usable as a Meson subproject.
project_deps = declare_dependency(
  include_directories: public_headers,
  link_with : project_target
)
set_variable(meson.project_name() + '_dep', project_deps )