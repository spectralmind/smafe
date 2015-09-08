#!/bin/bash
# online resources: 
# http://steve-parker.org/sh/sh.shtml
# http://www.freeos.com/guides/lsst/
# http://www.ss64.com/bash/

##########################################################################################
#
#	This script tests smafedistd 
#
#-----------------------------------------------------------------------------------------
#
#	Actions:
#		# Check distancejobs (0 with OK status)
#		# Start smafedistd with:
#			- interval -1 	(quit after no more open tasks)
#			- no-daemon		(not daemon mode)
#			- standard db related options
#			- --initial-run
#			- verbosity 1
#		# Check distancejobs (42 with OK status)
#		# check if no WW, EE or FF occured in log file
#
#-----------------------------------------------------------------------------------------
#
#	Note:
#		Preconditions:
#		- config stored
#		- fvs, files, tracks inserted
#		- 42 distance jobs to be done (eg 3 (distancetype_ids) * 2 (tracks) * 7 (fvtypes) )
#
##########################################################################################


# set name of test for logfile
CANONICALTESTNAME=testDistd
TESTLOGFILE=../test/$CANONICALTESTNAME.log

# this test will try to run smafewrapd with a given options file 
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



### check contents before: no OK tasks
#: number of OK jobs
SQLQUERY="select count(*) from distancejob where status='OK';"
echo $SQLQUERY | psql  -d $TESTDATABASE  >> $TESTLOGFILE 2>&1
#echo $SQLQUERY | psql  -d $TESTDATABASE  | sed '$d' | sed '$d'  >> ../test/resources/$CANONICALTESTNAME/query0.txt 2>&1
# cmp with reference
# - parameter for diff means stdin
# tail -n X: X = lines of reference files + 2
# sed command dleetes the last line(s)
tail -n 5 $TESTLOGFILE | sed '$d' | sed '$d' | diff --strip-trailing-cr ../test/resources/$CANONICALTESTNAME/query0.txt - >> $TESTLOGFILE 2>&1
testResultc0=$?



 
# run smafedistd
$SMAFETESTEXECPREFIX/smafedistd/smafedistd --jobs --id=testCaseDaemon --interval="-1" --dbconf ../test/resources/localhost-testcasedb-dbconnection.opt --no-daemon  --initial-run -v $SMAFETESTPARAMV >> $TESTLOGFILE 2>&1
testResult1=$?


### check contents AFTER: 30 OK tasks
#: number of OK jobs
SQLQUERY="select count(*) from distancejob where status='OK';"
echo $SQLQUERY | psql  -d $TESTDATABASE  >> $TESTLOGFILE 2>&1
#echo $SQLQUERY | psql  -d $TESTDATABASE  | sed '$d' | sed '$d'  >> ../test/resources/$CANONICALTESTNAME/query1.txt 2>&1
# cmp with reference
# - parameter for diff means stdin
# tail -n X: X = lines of reference files + 2
# sed command dleetes the last line(s)
tail -n 5 $TESTLOGFILE | sed '$d' | sed '$d' | diff --strip-trailing-cr ../test/resources/$CANONICALTESTNAME/query1.txt - >> $TESTLOGFILE 2>&1
testResultc1=$?




# also check if no warnings,  errors (EE) and no fatal errors (FF) occured
# Note: grep returns 0 if at least one occurence is found, 1 if not a singel one is found, and 2 for errors
grep " WW " $TESTLOGFILE
testErrors0=$?
grep " EE " $TESTLOGFILE
testErrors1=$?
grep " FF " $TESTLOGFILE
testErrors2=$?


echo "Variable testResult1 is now set to $testResult1. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testResultc0 is now set to $testResultc0. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testResultc1 is now set to $testResultc1. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testErrors0 is now set to $testErrors0.(expected 1)"				>> $TESTLOGFILE
echo "Variable testErrors1 is now set to $testErrors1.(expected 1)"				>> $TESTLOGFILE
echo "Variable testErrors2 is now set to $testErrors2.(expected 1)"				>> $TESTLOGFILE



# move logfile to final destination
mv $TESTLOGFILE $SMAFETESTEXECPREFIX/


# exit with success / error code
if [ $testResult1 == 0 -a $testResultc0 == 0  -a $testResultc1 == 0  -a $testErrors0 == 1 -a $testErrors1 == 1 -a $testErrors2 == 1 ] 
then
  echo "$CANONICALTESTNAME completed successfully."
  exit 0 
else 
  echo "There was an error while running test $CANONICALTESTNAME! For details see $SMAFETESTEXECPREFIX/$(basename $TESTLOGFILE)!"
  exit 1
fi 
