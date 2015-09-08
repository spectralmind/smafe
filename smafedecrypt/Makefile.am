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
bin_PROGRAMS = smafedecrypt
smafedecrypt_SOURCES = \
	src/smafedecrypt.cpp
# add dependent libraries 
# if used for a library _LIBADD is the libtool version of _LDADD
smafedecrypt_LDADD = \
	../smafeutil/libsmafeutil.la \
	-lboost_serialization$(BOOSTSUFFIX) \
 	-largtable2 \
 	-lpthread \
 	-lcryptopp
# list public header files that needs to be installed in order for people to use the library
# Headers listed in SOURCES are internal headers that are not part of the public interface
#include_HEADERS = 
# add compiler flags 
# add include directories like md5 
smafedecrypt_CPPFLAGS = \
	-I$(top_srcdir)/smafeutil/src \
	-I$(CRYPTOPP_INCLUDE_FOLDER) 	