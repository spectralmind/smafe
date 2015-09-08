#!/bin/bash
# online resources: 
# http://steve-parker.org/sh/sh.shtml
# http://www.freeos.com/guides/lsst/
# http://www.ss64.com/bash/



##########################################################################################
#
#	This script tries to drop the testcase db and the two test users using the PG
#		environment variables.
#
#-----------------------------------------------------------------------------------------
#
#	Actions:
#		# calls dropdb on the test case db
#		# calls to DROP ROLE commands (testsmafeadmin and testsmurf)
#
#-----------------------------------------------------------------------------------------
#
#	Note:
#		- The script in fact DOES NOT CHECK if the commands return successfully!
#			Reason: This script is seen as a general cleanup script that is called after
#					the test scripts that create the test db. It is also called after
#					smui test  but as this test is only conditionally performed (if 
#					the smui package is installed the the pertinent environment variable
#					points to it) the script wil fail in those cases where smui test is
#					in fact not performed.
#			Q: Why is the dropdb not called in the respective script?
#			A: Because:
#				- the db is created with the PG env variables
#				- then, these are overwritten with either testsmafeadmin or testsmurf,
#					depending on the test
#				- finally we would need the original env variables again because the 
#					test users do not have the permission to drop the db*
#
#			* why actually? Owner of db should be smafeadmins, and testsmafeadmin is member
#				of that role.
#
##########################################################################################

# set name of test for logfile
CANONICALTESTNAME=testCleanupRemoveDB
TESTLOGFILE=../test/$CANONICALTESTNAME.log


# this test will undo all changes to your system 
echo "Cleaning up test setup .... "

#check env
. ../test/checkEnv.lib
# prepare log file
rm -f $TESTLOGFILE
touch $TESTLOGFILE


checkEnv >> $TESTLOGFILE 2>&1

# drop testdb
dropdb  $TESTDATABASE >> $TESTLOGFILE 2>&1
testResult=0



# delete test roles
psql  postgres -c "drop role testsmafeadmin;"  >> $TESTLOGFILE 2>&1
psql  postgres -c "drop role testsmurf;"  >> $TESTLOGFILE 2>&1

# move logfile to final destination
mv $TESTLOGFILE $SMAFETESTEXECPREFIX/


# exit with success / error code
# currently, this is a tautology
if [ $testResult == 0 ] 
then
  echo "Cleanup complete / tried to delete test database and test users."
  exit 0 
else 
  echo "there was an error while trying to undo changes to your system. For details see $SMAFETESTEXECPREFIX/$(basename $TESTLOGFILE)"
  exit 1
fi 
