###########################################################################
## Copyright (c) 2009 spectralmind.com
## All rights reserved.
## ------------------------------------------------------------------------
## Makefile.am
##  Created on: 03/2009
##      Author: wolfgang
## ------------------------------------------------------------------------
## SVN Version: $Id$
###########################################################################

# list libraries and their sources (la is the suffix for libtools libraries)
# _LTLIBRARIES use libtool to perform platform specific linking
# all libraries should start with lib...
lib_LTLIBRARIES = libsmafedec.la
libsmafedec_la_SOURCES = \
	src/smafefile.cpp \
	src/smafefile.h \
	src/wavIO.cpp \
	src/wavIO.h \
	src/smafeFileMP3.h \
	src/smafeFileWAV.h 	\
	src/smafeFileExternal.h
	
# list public header files that needs to be installed in order for people to use the library
# Headers listed in SOURCES are internal headers that are not part of the public interface
# include_HEADERS = 

# add compiler flags 
# add include directories like smafeutil 
# -D_FILE_OFFSET_BITS=64 is necessary for 32 bit systems as libmp123 has large file support enabled by default.
# On 64 bit systems this is teh default anyway.
libsmafedec_la_CPPFLAGS = -D_FILE_OFFSET_BITS=64 \
    -I$(top_srcdir)/smafeutil/src \
 	-I$(CRYPTOPP_INCLUDE_FOLDER)
#libsmafedec_la_CPPFLAGS = -I$(top_srcdir)/smafeutil/src 

# linker flags
libsmafedec_la_LDFLAGS = -version-info 1:0:1