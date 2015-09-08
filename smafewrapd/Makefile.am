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
bin_PROGRAMS = smafewrapd
smafewrapd_SOURCES = \
	src/smafewrapd.cpp

# add dependent libraries 
# if used for a library _LIBADD is the libtool version of _LDADD
smafewrapd_LDADD = \
	../smafeext/libsmafeext.la \
	../smafeutil/libsmafeutil.la \
	../smafestore/libsmafestore.la \
	../smafedec/libsmafedec.la \
	-lpq \
	-lboost_serialization$(BOOSTSUFFIX) \
 	-lboost_filesystem$(BOOSTSUFFIX) \
 	-lboost_system$(BOOSTSUFFIX) \
    -lboost_date_time$(BOOSTSUFFIX) \
 	-largtable2 \
 	-lmpg123 \
  -L$(INTEL_IPP_FOLDER)/$(IPP_LIB_PATH) \
  $(IPP_IPPS_LIBS) \
  $(IPP_CORE_LIBS) \
  $(IPP_LDFLAGS) \
 	-lcryptopp
	
# extra flags for the linker 
# runtime library search path rpath: if you link dynamic/shared libraries you need to tell the linker where to find the libs
smafewrapd_LDFLAGS = -rpath '$(INTEL_IPP_FOLDER)//$(IPP_LIB_PATH)'  -fopenmp   

# c++ flags
smafewrapd_CXXFLAGS = -fopenmp  

# list public header files that needs to be installed in order for people to use the library
# Headers listed in SOURCES are internal headers that are not part of the public interface
#include_HEADERS = 

# add compiler flags 
# add include directories like md5 

smafewrapd_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/smafeutil/src \
	-I$(top_srcdir)/smafeext/src \
	-I$(top_srcdir)/smafestore/src \
	-I$(top_srcdir)/smafedec/src \
	-I$(INTEL_IPP_FOLDER)/include \
	-I$(POSTGRES_INCLUDE_FOLDER) \
	-I$(CRYPTOPP_INCLUDE_FOLDER)

	
 	
 	