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
lib_LTLIBRARIES = libsmafedist.la
libsmafedist_la_SOURCES = \
        src/smafeDistancesCalc.h \
        src/smafeDistancesCalc.cpp
	
# list public header files that needs to be installed in order for people to use the library
# Headers listed in SOURCES are internal headers that are not part of the public interface
# include_HEADERS = 


libsmafedist_la_LIBADD = \
        ../smafeutil/libsmafeutil.la

# add compiler flags 
# add include directories like smafeutil 
# -D_FILE_OFFSET_BITS=64 is necessary for 32 bit systems as libmp123 has large file support enabled by default.
# On 64 bit systems this is teh default anyway.
#libsmafedist_la_CPPFLAGS = -I$(top_srcdir)/smafeutil/src -I$(top_srcdir)/md5
libsmafedist_la_CPPFLAGS = \
        -I$(top_srcdir) \
        -I$(top_srcdir)/smafeutil/src \
        -I$(INTEL_IPP_FOLDER)/include \
        -I$(CRYPTOPP_INCLUDE_FOLDER) \
        $(NULL) 
        
# linker flags
libsmafedist_la_LDFLAGS = -version-info 1:0:1