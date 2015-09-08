#!/bin/bash


##########################################################################################
#
#	This script tests whether the API can be reached via web server, ie.
#	the tests passes if we get an HTTP code of 200 and it fails otherwise.
#
#-----------------------------------------------------------------------------------------
#
#	Parmeters:
#		# <API endpoint> The endpoint of the API
#
#-----------------------------------------------------------------------------------------
#
#	Actions:
#		# Call API call version to check if HTTP 200 returns.
#
#-----------------------------------------------------------------------------------------
#
#	Notes:
#		# URL is assumed to be standard (http, and standard alias)
#
##########################################################################################

# set name of test for logfile
CANONICALTESTNAME=testAccessibility
TESTLOGFILE=$CANONICALTESTNAME.log
rm -f $TESTLOGFILE

EXPECTED_ARGS=1
if [ $# -ne $EXPECTED_ARGS ]; then
  echo "Usage: `basename $0` <API endpoint>"
  echo "        where <API endpoint> is, eg, http://localhost/smintapi/"
  exit 1
fi

echo "Running test $CANONICALTESTNAME.... "


URL_API_ENDPOINT=$1


# for ssl: --no-check-certificate
wget --header='Accept: application/xml'  "$URL_API_ENDPOINT/smintapi.php/version"  >> $TESTLOGFILE 2>&1
testResult1=$?




echo "Variable testResult1 is now set to $testResult1.(expected 0)"			>> $TESTLOGFILE


# exit with success / error code
if [ $testResult1 == 0  ]
then
  echo "$CANONICALTESTNAME completed successfully."
  exit 0 
else 
  echo "There was an error while running the $CANONICALTESTNAME test! For details see $(basename $TESTLOGFILE)!"
  exit 1
fi 
