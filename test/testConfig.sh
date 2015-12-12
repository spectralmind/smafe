#!/bin/bash
# online resources: 
# http://steve-parker.org/sh/sh.shtml
# http://www.freeos.com/guides/lsst/
# http://www.ss64.com/bash/


##########################################################################################
#
#	This script tests config mode of smafewrapd.
#	If the plain text config file is not available
#	the script tries to use the encrypted sql file instead (used in deployment)
#
#-----------------------------------------------------------------------------------------
#
#	Actions:
#
#		# executes smafeConfig to create sql file IF plain text options file exists
#		# start sql script into db (created by testCreateDB)
#		# check contents of config table
#
#-----------------------------------------------------------------------------------------
#
#	Note:
#
##########################################################################################

# set name of test for logfile
CANONICALTESTNAME=testConfig
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
#	if [ $PGUSERSET != "noset" ]; then
		# set to admin user
		PGUSERSET=admin
		setSmafePGUser >> $TESTLOGFILE 2>&1
#	fi
fi
checkEnv >> $TESTLOGFILE 2>&1


OPTFILE=../test/resources/test.opt
SQLFILE=$OPTFILE.sql


## check existance of options file
if [ -f $OPTFILE ]
then

# run smafeconfig
$SMAFETESTEXECPREFIX/smafewrapd/smafewrapd --config  $OPTFILE  >> $TESTLOGFILE 2>&1
testResult1=$?
# check existance of file
if [ -f $SQLFILE ]
then
	testResult2=0
else
	echo "Output SQL script file does not exist." >> $TESTLOGFILE 2>&1
	testResult2=1
fi

else
	echo Plain text options file not found, trying to use encrypted file. >> $TESTLOGFILE 2>&1
	testResult1=0
	testResult2=0
fi



# insert config
cat $SQLFILE  | psql  -d $TESTDATABASE    >> $TESTLOGFILE 2>&1
testResult3=$?



### check contents 
#: number of config params
echo "select count(*) from config;" | psql  -d $TESTDATABASE  >> $TESTLOGFILE 2>&1
#echo "select count(*) from config;" | psql  -d $TESTDATABASE  >> ../test/resources/$CANONICALTESTNAME/query1.txt 2>&1
# cmp with reference
# - parameter for diff means stdin
# tail -n X: X = lines of reference files + 2
# sed command dleetes the last line(s)
tail -n 5 $TESTLOGFILE | sed '$d' | sed '$d' | diff --strip-trailing-cr ../test/resources/$CANONICALTESTNAME/query1.txt - >> $TESTLOGFILE 2>&1
testResultc1=$?





# also check if no errors (EE) and no fatal errors (FF) occured
# Note: grep returns 0 if at least one occurence is found, 1 if not a singel one is found, and 2 for errors
grep " EE " $TESTLOGFILE
testErrors1=$?
grep " FF " $TESTLOGFILE
testErrors2=$?



echo "Variable testResult1 is now set to $testResult1.(expected 0)"			>> $TESTLOGFILE
echo "Variable testResult2 is now set to $testResult2.(expected 0)"			>> $TESTLOGFILE
echo "Variable testResult3 is now set to $testResult3.(expected 0)"			>> $TESTLOGFILE
echo "Variable testResultc1 is now set to $testResultc1.(expected 0)"			>> $TESTLOGFILE
echo "Variable testErrors1 is now set to $testErrors1.(expected 1)"				>> $TESTLOGFILE
echo "Variable testErrors2 is now set to $testErrors2.(expected 1)"				>> $TESTLOGFILE


# move logfile to final destination
mv $TESTLOGFILE $SMAFETESTEXECPREFIX/





# exit with success / error code
if [ $testResult1 == 0 -a $testResult2 == 0 -a $testResult3 == 0 -a $testResultc1 == 0  -a $testErrors1 == 1 -a $testErrors2 == 1 ]
then
  echo "$CANONICALTESTNAME completed successfully."
  exit 0 
else 
  echo "There was an error while running the $CANONICALTESTNAME test! For details see $SMAFETESTEXECPREFIX/$(basename $TESTLOGFILE)!"
  exit 1
fi 
