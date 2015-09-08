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

# define NULL as empty to use it at end of pretty printed lists
# to avoid the 
#		blank line following trailing backslash
# error.
NULL=

# include support for doxygen
include $(top_srcdir)/doxygen.am

# contains options to pass to aclocal 
# -I m4 : include macros from directory m4 instead of building a large acinclude.m4 in project root
ACLOCAL_AMFLAGS = -I m4







  
# check if we have deploy version or "admin" version 
if ENABLE_DEPLOY

# List all directories that need to be parsed by automake
# deploy version
SUBDIRS = \
	smafeutil \
	smafedec \
	smafestore \
	smafeext \
	smafedist \
	smafedistd \
	smafewrapd \
	smuiupdated \
	$(OPTIONAL_MAKE_FOLDERS) \
  $(NULL)


# NOTE: if you change the tests, also edit accordingly in misc/smafesetup.sh and misc/mkdeploy.sh!
# NOTE: The CleanupRemoveDB tests are just there to dlete the test database and users
TESTS = \
    test/testCreateDB.sh \
    test/testConfig.sh \
    test/testWrap.sh \
    test/testDistd.sh \
    test/testTicket75_twobadfiles.sh \
    test/testWrapNoDoubleFree.sh \
    test/testTimbralExtractorNoFrames.sh \
    test/testLic.sh \
    test/testCleanupRemoveDB.sh \
    test/testCreateDB.sh \
    test/testConfig.sh \
    test/testFiletypes.sh \
    test/testCleanupRemoveDB.sh \
    test/testCreateDB.sh \
    test/testDistd_extended.sh \
    test/testCleanupRemoveDB.sh \
    test/testCreateDB.sh \
    test/testDistd_live.sh \
    test/testCleanupRemoveDB.sh \
    test/testCreateDB.sh \
    test/testSmuiupdated.sh \
    test/testCleanupRemoveDB.sh
    
# disabled in this branch due to race condition problem in test
#	 test/testCreateDB.sh \
#    test/testDistd_extended2.sh \
#    test/testCleanupRemoveDB.sh \
#   


else

# admin version

# List all directories that need to be parsed by automake
# admin version
SUBDIRS = \
	smafeutil \
	smafedec \
	smafestore \
	smafequery \
	smafeext \
	smafedist \
	smafedistd \
	smafewrapd \
	smafenorm \
	smuiupdated \
	smafedecrypt \
	$(OPTIONAL_MAKE_FOLDERS) \
  $(NULL)



# NOTE: The CleanupRemoveDB tests are just there to dlete the test database and users

TESTS = \
    test/testCreateDB.sh \
    test/testConfig.sh \
    test/testWrap.sh \
    test/testNorm.sh \
    test/testDistd.sh \
    test/testQuery.sh \
    test/testTicket75_twobadfiles.sh \
    test/testWrapNoDoubleFree.sh \
    test/testTimbralExtractorNoFrames.sh \
    test/testLic.sh \
    test/testCleanupRemoveDB.sh \
    test/testCreateDB.sh \
    test/testConfig.sh \
    test/testFiletypes.sh \
    test/testCleanupRemoveDB.sh \
    test/testCreateDB.sh \
    test/testDistd_extended.sh \
    test/testCleanupRemoveDB.sh \
    test/testCreateDB.sh \
    test/testDistd_live.sh \
    test/testCleanupRemoveDB.sh \
    test/testCreateDB.sh \
    test/testSmuiupdated.sh \
    test/testCleanupRemoveDB.sh \
    test/testWrap_extended.sh \
    test/testWrap_extended_nopass.sh \
    test/testDecrypt.sh

#    test/testCreateDB.sh \
#    test/testDistd_extended2.sh \
#    test/testCleanupRemoveDB.sh \
#
#	test/testSMAFE_NODB.sh \
#

endif


 
