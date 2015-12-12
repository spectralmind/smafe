#!/bin/bash
# online resources: 
# http://steve-parker.org/sh/sh.shtml
# http://www.freeos.com/guides/lsst/
# http://www.ss64.com/bash/


##########################################################################################
#
#	This script tests smafenorm 
#
#-----------------------------------------------------------------------------------------
#
#	Actions:
#		# calls smafenorm on feature vector types 1, 2 and 3
#
#-----------------------------------------------------------------------------------------
#
#	Note:
#
##########################################################################################


# set name of test for logfile
CANONICALTESTNAME=testNorm
TESTLOGFILE=../test/$CANONICALTESTNAME.log
# remove old log file. -f supresses warnings for non-existant file
rm -f $TESTLOGFILE

 
echo "Running test $CANONICALTESTNAME.... "



#check env
. ../test/checkEnv.lib
# set to user
PGUSERSET=user
setSmafePGUser > $TESTLOGFILE 2>&1
checkEnv >> $TESTLOGFILE 2>&1

# run program  
#$SMAFETESTEXECPREFIX/smafenorm/smafenorm -v $SMAFETESTPARAMV -f 1 -f 2 -f 3 --dbhost=localhost --dbname=$TESTDATABASE --dbuser=smurf --dbpwd=papa  >> $TESTLOGFILE 2>&1
$SMAFETESTEXECPREFIX/smafenorm/smafenorm -v $SMAFETESTPARAMV -f 1 -f 2 -f 3 --dbconf ../test/resources/localhost-testcasedb-dbconnection.opt  >> $TESTLOGFILE 2>&1
testResult1=$?

# check for "Wrote 3 feature vectors for this type
grep "Wrote 3 feature vectors for this type" $TESTLOGFILE >> /dev/null 2>&1
testSuccess=$?
echo "testSuccess=$testSuccess"  >> $TESTLOGFILE 2>&1

# also check if no warnings,  errors (EE) and no fatal errors (FF) occured
# Note: grep returns 0 if at least one occurence is found, 1 if not a singel one is found, and 2 for errors
grep " WW " $TESTLOGFILE
testErrors0=$?
grep " EE " $TESTLOGFILE
testErrors1=$?
grep " FF " $TESTLOGFILE
testErrors2=$?


echo "Variable testResult1 is now set to $testResult1. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testSuccess is now set to $testSuccess. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testErrors0 is now set to $testErrors0.(expected 1)"				>> $TESTLOGFILE
echo "Variable testErrors1 is now set to $testErrors1.(expected 1)"				>> $TESTLOGFILE
echo "Variable testErrors2 is now set to $testErrors2.(expected 1)"				>> $TESTLOGFILE


# move logfile to final destination
mv $TESTLOGFILE $SMAFETESTEXECPREFIX/



# exit with success / error code
if [ $testResult1 == 0 -a $testSuccess == 0   -a $testErrors0 == 1 -a $testErrors1 == 1 -a $testErrors2 == 1 ] 
then
  echo "$CANONICALTESTNAME complete"
  exit 0 
else 
  echo "There was an error while running the $CANONICALTESTNAME test! For details see $SMAFETESTEXECPREFIX/$(basename $TESTLOGFILE) "
  exit 1
fi 
