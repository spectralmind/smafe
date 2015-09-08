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


# list binaries and their sources
bin_PROGRAMS = smafenorm
smafenorm_SOURCES = \
	src/smafenorm.cpp


# add dependent libraries 
# if used for a library _LIBADD is the libtool version of _LDADD
smafenorm_LDADD = \
	../smafeutil/libsmafeutil.la \
	../smafestore/libsmafestore.la \
	../smafedec/libsmafedec.la \
	-lpq \
	-lboost_serialization$(BOOSTSUFFIX) \
 	-lboost_filesystem$(BOOSTSUFFIX) \
    -lboost_date_time$(BOOSTSUFFIX) \
 	-largtable2 \
 	-lmpg123 \
 	-lpthread \
 	-lcryptopp



smafenorm_CXXFLAGS = -fopenmp
smafenorm_LDFLAGS = -fopenmp

# list public header files that needs to be installed in order for people to use the library
# Headers listed in SOURCES are internal headers that are not part of the public interface
#include_HEADERS = 

# add compiler flags 
# add include directories like md5 

smafenorm_CPPFLAGS = \
	-I$(top_srcdir)/smafeutil/src \
	-I$(top_srcdir)/smafestore/src \
	-I$(top_srcdir)/smafedec/src \
	-I$(POSTGRES_INCLUDE_FOLDER) \
	-I$(CRYPTOPP_INCLUDE_FOLDER) 
