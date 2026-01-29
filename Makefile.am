## Process this file with automake to produce Makefile.in

SUBDIRS = res src test

dist_doc_DATA = \
	README \
	COPYING \
	AUTHORS \
	ChangeLog \
	NEWS

ACLOCAL_AMFLAGS=-I m4
# this is a no-tech approach,
#  if you don't like gcc/clang, replace the optimisation, the rest should be optional
# optimisations, for use where compiled, remove native for wider usage
export OPTIM_CFLAGS= -O3
export OPTIM_CXXFLAGS = -mtune=native -march=native -O3
# some warnings
export WARN_CFLAGS = -Wall -Wconversion
export WARN_CXXFLAGS = -Wall -Wconversion -Wpedantic
# options originate from https://github.com/cpp-best-practices/cmake_template/blob/main/cmake/Hardening.cmake
#   -D_GLIBCXX_ASSERTIONS    : GLIBC++ Assertions (vector[], string[], ...)
#   -fstack-protector-strong :  -fstack-protector-strong
#   -fcf-protection          : checking that target addresses of control-flow transfer instructions are valid
#   -fsanitize=undefined     : undefined behaviour sanitizer
export INSTUMENT_CXXFLAGS = -D_GLIBCXX_ASSERTIONS -fstack-protector-strong -fcf-protection -fsanitize=undefined
export INSTUMENT_LINKFLAGS = -fsanitize=undefined

# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(docdir)
