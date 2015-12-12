#!/bin/bash
# online resources: 
# http://steve-parker.org/sh/sh.shtml
# http://www.freeos.com/guides/lsst/
# http://www.ss64.com/bash/


##########################################################################################
#
#	This script tests various file types
#
#-----------------------------------------------------------------------------------------
#
#	Actions:
#		# calls smafewrap on three 0 byte files. Expected outcome are three failed jobs.
#		# At the end, the copies of the files are cleaned and the fact is expected that 
#			they have not been moved by smafewrapd.
#
#-----------------------------------------------------------------------------------------
#
#	Note:
#		
#		Preconditions:
#	 		- config is in existing db
#
##########################################################################################


# set name of test for logfile
CANONICALTESTNAME=testFiletypes
TESTLOGFILE=../test/$CANONICALTESTNAME.log
# remove old log file. -f supresses warnings for non-existant file
rm -f $TESTLOGFILE

echo "Running test $CANONICALTESTNAME.... "


#check env
. ../test/checkEnv.lib
# prepare log file
rm -f $TESTLOGFILE
touch $TESTLOGFILE

# sets the pg user unless PGDOUSERSET is set (used during deployment on target machine)
if [ -z $PGDOUSERSET ]; then
	# set to  user
	PGUSERSET=user
	setSmafePGUser >> $TESTLOGFILE 2>&1
fi
checkEnv >> $TESTLOGFILE 2>&1


################ init 
# remove files left over
rm -f ../test/resources/$CANONICALTESTNAME/*fortesting* >> $TESTLOGFILE 2>&1
rm -f /tmp/*fortesting* >> $TESTLOGFILE 2>&1


### insert files
INPATH=../test/resources/$CANONICALTESTNAME/*.*
doFileAdd


# run smafewrapd  
$SMAFETESTEXECPREFIX/smafewrapd/smafewrapd --no-daemon --id=test --interval -1 -v $SMAFETESTPARAMV --dbconf ../test/resources/localhost-testcasedb-dbconnection.opt  >> $TESTLOGFILE  2>&1
testResult1=$?


# post processing: check if files are moved
# NOTE this error is expected since the files are invalid and therefor not correctly processesd and moved!
MAKECLEAN=no
doFileCleanup
testResultc1=$testResultcheck



echo "Variable testResult1 is now set to $testResult1. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testResultc1 is now set to $testResultc1. (expected 0)" 			>> $TESTLOGFILE


# move logfile to final destination
mv $TESTLOGFILE $SMAFETESTEXECPREFIX/




# exit with success / error code
if [ $testResult1 == 0 -a $testResultc1 == 0 ] 
then
  echo "$CANONICALTESTNAME complete"
  exit 0 
else 
  echo "There was an error while running the $CANONICALTESTNAME test! For details see $SMAFETESTEXECPREFIX/$(basename $TESTLOGFILE) "
  exit 1
fi 