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
lib_LTLIBRARIES = libsmafeext.la
libsmafeext_la_SOURCES = \
	src/smafeRPExtractor.cpp \
	src/smafeRPExtractor.h \
	src/smafeExtractor.h \
	src/smafeExtractorUtil.h \
	src/SmafeTimbralExtractor.h \
	src/SmafeTimbralExtractor.cpp \
	src/SmafeSomlibfileExtractor.h \
	src/SmafeSomlibfileExtractor.cpp
	
	
# add dependent libraries and library path names
# if used for a library _LIBADD is the libtool version of _LDADD
libsmafeext_la_LIBADD = \
        -L$(INTEL_IPP_FOLDER)/$(IPP_LIB_PATH) \
        $(IPP_LDFLAGS)

# extra flags for the linker 
# libsmafeext_la_LDFLAGS = 
	
# linker flags
libsmafeext_la_LDFLAGS = -version-info 1:0:1	
	

# list public header files that needs to be installed in order for people to use the library
# Headers listed in SOURCES are internal headers that are not part of the public interface
#include_HEADERS = 

# add compiler flags 
# add include directories like md5 

libsmafeext_la_CPPFLAGS = \
	-I$(top_srcdir)/smafeutil/src \
	-I$(INTEL_IPP_FOLDER)/include \
	-I$(top_srcdir)/smafestore/src \
	-I$(CRYPTOPP_INCLUDE_FOLDER)



  # the test related things: (only compiled with make check)  
# check if cppunit is available 
if ENABLE_CPPUNIT
CPPUNIT_TESTS = testSmafeRPExtractor 
else
CPPUNIT_TESTS = 
endif

        
  # list tests and their sources
  check_PROGRAMS = $(CPPUNIT_TESTS) 
 
 ############ RP features #####################


  testSmafeRPExtractor_SOURCES = test/testSmafeRPExtractor.cpp $(top_srcdir)/test/TestRunner.cpp
  # test libs and flags are set by AM_PATH_CPPUNIT(version) in configure.ac 
  testSmafeRPExtractor_CXXFLAGS = $(CPPUNIT_CFLAGS) 
  testSmafeRPExtractor_LDFLAGS = $(CPPUNIT_LIBS) -rpath '$(INTEL_IPP_FOLDER)//$(IPP_LIB_PATH)' -pthread
  testSmafeRPExtractor_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/smafeutil/src \
	-I$(top_srcdir)/smafeext/src \
	-I$(top_srcdir)/smafedec/src \
	-I$(INTEL_IPP_FOLDER)/include \
	-I$(top_srcdir)/smafestore/src \
	-I$(CRYPTOPP_INCLUDE_FOLDER)

 testSmafeRPExtractor_LDADD = \
	../smafeutil/libsmafeutil.la \
	../smafeext/libsmafeext.la \
 	-lboost_system$(BOOSTSUFFIX) \
	-lboost_serialization$(BOOSTSUFFIX) \
	-lboost_date_time$(BOOSTSUFFIX) \
  -L$(INTEL_IPP_FOLDER)/$(IPP_LIB_PATH) \
  $(IPP_IPPS_LIBS) \
  $(IPP_CORE_LIBS) \
  $(IPP_LDFLAGS)  
  
  
  
  
  
  
	 

# and define to run the tests: they to be run by make check
# any executeable is allowed; if make does not find the executeable it will try to build it
TESTS = $(CPPUNIT_TESTS)


