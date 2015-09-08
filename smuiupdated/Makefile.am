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
bin_PROGRAMS = smuiupdated
smuiupdated_SOURCES = \
	src/smuiupdated.cpp

# add dependent libraries 
# if used for a library _LIBADD is the libtool version of _LDADD
smuiupdated_LDADD = \
	../smafeutil/libsmafeutil.la \
	../smafestore/libsmafestore.la \
	../smafedist/libsmafedist.la \
	-lpq \
	-lboost_serialization$(BOOSTSUFFIX) \
 	-lboost_filesystem$(BOOSTSUFFIX) \
 	-lboost_system$(BOOSTSUFFIX) \
    -lboost_date_time$(BOOSTSUFFIX) \
 	-largtable2 \
 	      -L$(INTEL_IPP_FOLDER)/$(IPP_LIB_PATH) \
        $(IPP_IPPS_LIBS) \
        $(IPP_CORE_LIBS) \
        $(IPP_LDFLAGS) \
 	-lcryptopp
 		
# extra flags for the linker 
# runtime library search path rpath: if you link dynamic/shared libraries you need to tell the linker where to find the libs
#smuiupdated_LDFLAGS = -rpath '$(INTEL_IPP_FOLDER)//$(IPP_LIB_PATH)'

# list public header files that needs to be installed in order for people to use the library
# Headers listed in SOURCES are internal headers that are not part of the public interface
#include_HEADERS = 

# add compiler flags 
# add include directories like md5 

smuiupdated_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/smafeutil/src \
	-I$(top_srcdir)/smafedist/src \
	-I$(top_srcdir)/smafestore/src \
	-I$(POSTGRES_INCLUDE_FOLDER) \
	-I$(INTEL_IPP_FOLDER)/include \
	-I$(CRYPTOPP_INCLUDE_FOLDER) \
        $(NULL) 


# c++ flags
smuiupdated_CXXFLAGS = -fopenmp 	
	
smuiupdated_LDFLAGS=-rpath '$(INTEL_IPP_FOLDER)/sharedlib' -pthread -fopenmp 
 	
 	
