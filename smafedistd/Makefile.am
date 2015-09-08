###########################################################################
## Copyright (c) 2009 spectralmind.com
## All rights reserved.
## ------------------------------------------------------------------------
## Makefile.am
##  Created on: 03/2009
##      Author: wolfgang, epei 
## ------------------------------------------------------------------------
## SVN Version: $Id$
###########################################################################

# define NULL as empty to use it at end of pretty printed lists
# to avoid the 
#		blank line following trailing backslash
# error.

NULL=
        
# list binaries and their sources
bin_PROGRAMS = smafedistd
smafedistd_SOURCES = \
        src/smafedistd.cpp


# add dependent libraries 
# if used for a library _LIBADD is the libtool version of _LDADD
smafedistd_LDADD = \
        ../smafeutil/libsmafeutil.la \
        ../smafestore/libsmafestore.la \
        ../smafedist/libsmafedist.la \
        -lpq \
        -lpthread \
        -lboost_serialization$(BOOSTSUFFIX) \
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
smafedistd_LDFLAGS = -rpath '$(INTEL_IPP_FOLDER)/sharedlib' -fopenmp

# list public header files that needs to be installed in order for people to use the library
# Headers listed in SOURCES are internal headers that are not part of the public interface
#include_HEADERS = 

# add compiler flags 
# add include directories like md5 

smafedistd_CPPFLAGS = \
        -I$(top_srcdir) \
        -I$(top_srcdir)/smafeutil/src \
        -I$(top_srcdir)/smafedist/src \
        -I$(POSTGRES_INCLUDE_FOLDER) \
        -I$(top_srcdir)/smafestore/src \
        -I$(INTEL_IPP_FOLDER)/include \
        -I$(CRYPTOPP_INCLUDE_FOLDER) \
        $(NULL) 	

smafedistd_CXXFLAGS =  -fopenmp      
        
        
        
        
        
        
 
 # the test related things: (only compiled with make check)  
# check if cppunit is available 
if ENABLE_CPPUNIT
CPPUNIT_TESTS = testDistcalcPerf
else
CPPUNIT_TESTS = 
endif

        
  # list tests and their sources
  check_PROGRAMS = $(CPPUNIT_TESTS) 
        
        
        
testDistcalcPerf_SOURCES = test/testDistcalcPerf.cpp $(top_srcdir)/test/TestRunner.cpp
        
        
testDistcalcPerf_CPPFLAGS = -I$(top_srcdir)/smafedistd/src \
        -I$(top_srcdir) \
        -I$(top_srcdir)/smafeutil/src \
        -I$(top_srcdir)/smafedist/src \
        -I$(POSTGRES_INCLUDE_FOLDER) \
        -I$(top_srcdir)/smafestore/src \
        -I$(INTEL_IPP_FOLDER)/include \
		-I$(CRYPTOPP_INCLUDE_FOLDER) \
        $(NULL) 	

        
testDistcalcPerf_LDFLAGS = $(CPPUNIT_LIBS)   -rpath '$(INTEL_IPP_FOLDER)/sharedlib' -fopenmp     
testDistcalcPerf_CXXFLAGS = $(CPPUNIT_CFLAGS) -fopenmp


# add dependent libraries 
# if used for a library _LIBADD is the libtool version of _LDADD
testDistcalcPerf_LDADD = \
        ../smafeutil/libsmafeutil.la \
        ../smafestore/libsmafestore.la \
        ../smafedist/libsmafedist.la \
        -lpq \
        -lpthread \
        -lboost_serialization$(BOOSTSUFFIX) \
        -largtable2 \
        -L$(INTEL_IPP_FOLDER)/$(IPP_LIB_PATH) \
        $(IPP_IPPS_LIBS) \
        $(IPP_CORE_LIBS) \
        $(IPP_LDFLAGS)        
        
        
        
        
        
        
# and define to run the tests: they to be run by make check
# any executeable is allowed; if make does not find the executeable it will try to build it
TESTS = $(CPPUNIT_TESTS)

        