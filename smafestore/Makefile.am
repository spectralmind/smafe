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
lib_LTLIBRARIES = libsmafestore.la
libsmafestore_la_SOURCES = \
    src/smafestore_specific_include.h \
 	src/smafestoredb_pq.cpp \
   	src/smafestoredb_pq.h \
	src/smafestoredb_sl.cpp \
	src/smafestoredb_sl.h \
	src/smafestoredb.cpp \
	src/smafestoredb.h 

# add dependent libraries 
# if used for a library _LIBADD is the libtool version of _LDADD
libsmafestore_la_LIBADD = -lpq


# linker flags
libsmafestore_la_LDFLAGS = -version-info 1:0:1  -fopenmp    

# c++ flags
libsmafestore_la_CXXFLAGS = -fopenmp    

# todo add dependent compile on given flags 
#  	src/smafestoredb_sl.cpp 


# list public header files that needs to be installed in order for people to use the library
# Headers listed in SOURCES are internal headers that are not part of the public interface
#include_HEADERS = 


# add compiler flags 
# add include directories like md5 
libsmafestore_la_CPPFLAGS = \
    -I$(top_srcdir)/smafeutil/src \
    -I$(POSTGRES_INCLUDE_FOLDER) \
    -I$(CRYPTOPP_INCLUDE_FOLDER)


