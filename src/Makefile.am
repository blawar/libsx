# what flags you want to pass to the C compiler & linker
AM_CXXFLAGS = -I$(top_srcdir)/src
# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
lib_LTLIBRARIES = libsx.la
libsx_ladir = $(includedir)/sx
libsx_la_LIBADD = -lboost_system -lboost_filesystem -lboost_thread 
libsx_la_SOURCES = \
stream.cpp \
stream.h \
strings.cpp \
strings.h \
mmfile.cpp \
mmfile.h \
datatypes.h \
tokenize.cpp \
tokenize.h \
handle.cpp \
file.cpp \
libsx.cpp \
conf.cpp \
network.cpp \
httpserver.cpp \
tcpserver.cpp \
http/controller.cpp \
http/mimes.cpp \
sxml/sxmlparser.cpp \
sxml/sxml.cpp \
sxml/sxmlstr.cpp \
sxml/sxmlerror.cpp \
libsx.h

nobase_libsx_la_HEADERS = \
tcpserver.h \
httpserver.h \
network.h \
conf.h \
file.h \
handle.h \
tokenize.h \
datatypes.h \
stream.h \
strings.h \
mmfile.h \
libsx.h \
sxml/sxmlstr.h \
sxml/sxml.h \
http/mimes.h \
http/controller.h \
http/core.h \
xml sx
