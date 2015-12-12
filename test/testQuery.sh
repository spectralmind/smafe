#!/bin/bash
# online resources: 
# http://steve-parker.org/sh/sh.shtml
# http://www.freeos.com/guides/lsst/
# http://www.ss64.com/bash/


##########################################################################################
#
#	This script tests functions of the tool smafequery
#
#-----------------------------------------------------------------------------------------
#
#	Actions:
#		# xy
#
#-----------------------------------------------------------------------------------------
#
#	Note:
#		Is expected to be run after CreateDB, Config, Wrap, Norm, Distd
#
##########################################################################################


# set name of test for logfile
CANONICALTESTNAME=testQuery
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

# query for featurevectors of files with name *test* and write to somlib file
# we expect the file to be created and a message in the log
$SMAFETESTEXECPREFIX/smafequery/smafequery --fv -v $SMAFETESTPARAMV -f 1 -o testoutput1-$CANONICALTESTNAME -n test --dbconf ../test/resources/localhost-testcasedb-dbconnection.opt  >> $TESTLOGFILE 2>&1
testResult1=$?
grep "testoutput1-$CANONICALTESTNAME.RH.vec has been written (5 vectors)." $TESTLOGFILE >> /dev/null 2>&1
testSuccess1=$?
echo "testSuccess1=$testSuccess1"  >> $TESTLOGFILE 2>&1


# query for featurevectors of files with name *testnotfound* and write to somlib file
# we expect an error in the log
$SMAFETESTEXECPREFIX/smafequery/smafequery --fv -v $SMAFETESTPARAMV -f 1 -o testoutput2-$CANONICALTESTNAME -n testnotfound --dbconf ../test/resources/localhost-testcasedb-dbconnection.opt  >> $TESTLOGFILE 2>&1
testResult2=$?
grep "EE testnotfound not found in database. Skipping." $TESTLOGFILE >> /dev/null 2>&1
testSuccess2=$?
echo "testSuccess2=$testSuccess2"  >> $TESTLOGFILE 2>&1


# give an example file 
# we expect 1 vector
$SMAFETESTEXECPREFIX/smafequery/smafequery --fv -v $SMAFETESTPARAMV -f 1 -o testoutput3-$CANONICALTESTNAME ../test/resources/testsmall.mp3 --dbconf ../test/resources/localhost-testcasedb-dbconnection.opt  >> $TESTLOGFILE 2>&1
testResult3=$?
# check 
grep "testoutput3-$CANONICALTESTNAME.RH.vec has been written (1 vectors)." $TESTLOGFILE >> /dev/null 2>&1
testSuccess3a=$?
echo "testSuccess3a=$testSuccess3a"  >> $TESTLOGFILE 2>&1
grep "\$DATA_DIM 60x1" testoutput3-$CANONICALTESTNAME.RH.vec >> $TESTLOGFILE 2>&1
testSuccess3b=$?


# query for all featurevectors and write to somlib file
# we expect the file to be created and a message in the log
$SMAFETESTEXECPREFIX/smafequery/smafequery --fv -v $SMAFETESTPARAMV -f 1 -o testoutput4-$CANONICALTESTNAME --all --dbconf ../test/resources/localhost-testcasedb-dbconnection.opt  >> $TESTLOGFILE 2>&1
testResult4=$?
# why 3????
grep "testoutput4-$CANONICALTESTNAME.RH.vec has been written (3 vectors)." $TESTLOGFILE >> /dev/null 2>&1
testSuccess4=$?
echo "testSuccess4=$testSuccess4"  >> $TESTLOGFILE 2>&1


# query for nearest neighbour of file with *third*
# we expect the file to be created and a message in the log
$SMAFETESTEXECPREFIX/smafequery/smafequery --nn -v $SMAFETESTPARAMV -f 2 -d 2  -k 5 -n third --dbconf ../test/resources/localhost-testcasedb-dbconnection.opt  >> $TESTLOGFILE 2>&1
testResult10=$?
# check for first NN with a distance of about 18
grep "testWrap/testlong.mp3.fortesting.mp3: d=18" $TESTLOGFILE >> /dev/null 2>&1
testSuccess10a=$?
echo "testSuccess10a=$testSuccess10a"  >> $TESTLOGFILE 2>&1

# check for second NN with a distance of about 29
grep "testsmall10percent faster.mp3.fortesting.mp3: d=29" $TESTLOGFILE >> /dev/null 2>&1
testSuccess10b=$?
echo "testSuccess10b=$testSuccess10b"  >> $TESTLOGFILE 2>&1

# Nearest neighbours for track_id = 3:
grep "Nearest neighbours for track_id = 3:" $TESTLOGFILE >> /dev/null 2>&1
testSuccess10c=$?
echo "testSuccess10c=$testSuccess10c"  >> $TESTLOGFILE 2>&1




# clean
rm testoutput?-$CANONICALTESTNAME.*






# also check if no warnings,  errors (EE) and no fatal errors (FF) occured
# Note: grep returns 0 if at least one occurence is found, 1 if not a singel one is found, and 2 for errors
grep " WW " $TESTLOGFILE
testErrors0=$?
#grep " EE " $TESTLOGFILE
#testErrors1=$?
grep " FF " $TESTLOGFILE
testErrors2=$?


echo "Variable testResult1 is now set to $testResult1. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testSuccess1 is now set to $testSuccess1. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testResult2 is now set to $testResult2. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testSuccess2 is now set to $testSuccess2. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testResult3 is now set to $testResult3. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testSuccess3a is now set to $testSuccess3a. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testSuccess3b is now set to $testSuccess3b. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testResult4 is now set to $testResult4. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testSuccess4 is now set to $testSuccess4. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testResult10 is now set to $testResult10. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testSuccess10a is now set to $testSuccess10a. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testSuccess10b is now set to $testSuccess10b. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testSuccess10c is now set to $testSuccess10c. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testErrors0 is now set to $testErrors0.(expected 1)"				>> $TESTLOGFILE
#echo "Variable testErrors1 is now set to $testErrors1.(expected 1)"				>> $TESTLOGFILE
echo "Variable testErrors2 is now set to $testErrors2.(expected 1)"				>> $TESTLOGFILE


# move logfile to final destination
mv $TESTLOGFILE $SMAFETESTEXECPREFIX/



# exit with success / error code
if [ $testResult1 == 0 -a $testSuccess1 == 0 -a $testResult2 == 0 -a $testSuccess2 == 0 -a $testResult3 == 0 -a $testSuccess3a == 0 -a $testSuccess3b == 0 -a $testResult4 == 0 -a $testSuccess4 == 0 -a $testResult10 == 0 -a $testSuccess10a == 0 -a $testSuccess10b == 0 -a $testSuccess10c == 0 -a $testErrors0 == 1 -a $testErrors2 == 1 ] 
then
  echo "$CANONICALTESTNAME complete"
  exit 0 
else 
  echo "There was an error while running the $CANONICALTESTNAME test! For details see $SMAFETESTEXECPREFIX/$(basename $TESTLOGFILE) "
  exit 1
fi 
