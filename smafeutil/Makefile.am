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
lib_LTLIBRARIES = libsmafeutil.la
libsmafeutil_la_SOURCES = \
	src/smafeopt.h \
	src/smafeAbstractFeatureVector.cpp \
	src/smafeNumericFeatureVector.cpp \
	src/smafeutil.cpp \
	src/smafeAbstractOutput.h \
	src/smafeTextfileOutput.cpp \
	src/smafeExportDefs.h \
	src/smafeFVType.h \
	src/smafeNumericFeatureVector.h \
	src/smafeTextfileOutput.h \
	src/tAudioformat.h \
	src/smafeAbstractFeatureVector.h\
	src/smafeutil.h\
	src/smafeLogger.cpp\
	src/tLiveNNMessage.cpp\
	src/tLiveNNMessage.h
	
	
# list public header files that needs to be installed in order for people to use the library
# Headers listed in SOURCES are internal headers that are not part of the public interface
# include_HEADERS = 

# add compiler flags 
# add include directories like md5 
libsmafeutil_la_CPPFLAGS =  \
	-I$(CRYPTOPP_INCLUDE_FOLDER)
	
libsmafeutil_la_LIBADD = -lcryptopp


# linker flags
libsmafeutil_la_LDFLAGS = -version-info 1:0:1

# the test related things: (only compiled with make check)  
# check if cppunit is available 
if ENABLE_CPPUNIT
CPPUNIT_TESTS = testtemplate
else
CPPUNIT_TESTS = 
endif

# list tests and their sources
check_PROGRAMS = $(CPPUNIT_TESTS)
testtemplate_SOURCES = test/testtemplate.cpp $(top_srcdir)/test/TestRunner.cpp
# test libs and flags are set by AM_PATH_CPPUNIT(version) in configure.ac 
testtemplate_CXXFLAGS = $(CPPUNIT_CFLAGS) 
testtemplate_LDFLAGS = $(CPPUNIT_LIBS)

# and define to run the tests: they to be run by make check
# any executeable is allowed; if make does not find the executeable it will try to build it
TESTS = $(CPPUNIT_TESTS) test/test_with_shellscript.sh
