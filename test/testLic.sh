#!/bin/bash
# online resources: 
# http://steve-parker.org/sh/sh.shtml
# http://www.freeos.com/guides/lsst/
# http://www.ss64.com/bash/


##########################################################################################
#
#	This script tests various license restrictions ("trial period") 
#
#-----------------------------------------------------------------------------------------
#
#	Actions:
#
#		# check contents of config table: there are two more config fields now (added by smafewrapd)
#		# set system clock minus 1 day (sudo rights required)
#		# start smafewrapd and expect error (102)
#		# set system clock to + 7 days (sudo rights required)
#		# start smafewrapd and expect error
#		# set system clock to + 20 yers (sudo rights required)
#		# start smafewrapd and expect error
#		# reset system clock to hardware clock (sudo rights required)
#
#-----------------------------------------------------------------------------------------
#
#	Note: This script is supposed to be run *after* testWrap.sh (need not be directly after)
#
##########################################################################################

# set name of test for logfile
CANONICALTESTNAME=testLic
TESTLOGFILE=../test/$CANONICALTESTNAME.log
 
echo "Running test $CANONICALTESTNAME....                 [NB: This test can only pass if run as *root* user!]"
echo "                                         [NB: This test changes system date. Beware!]"

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

#################### END header #########################


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


##########
echo >> $TESTLOGFILE 2>&1
echo "-------------------------------NOTE-----------------------------"								>> $TESTLOGFILE 2>&1
echo "You must be root to successful pass the remainder of this test" 	>> $TESTLOGFILE 2>&1
echo 			>> $TESTLOGFILE 2>&1
echo "Background: this test changes system date to test license restrictions."			>> $TESTLOGFILE 2>&1
echo "Failing this test as non-root user does NOT mean that there is an issue with Smafe!"		>> $TESTLOGFILE 2>&1
echo "Failing this test as root, however, should be investigated."			>> $TESTLOGFILE 2>&1
echo "-------------------------------NOTE-----------------------------"								>> $TESTLOGFILE 2>&1




#### set system clock minus 1 day (sudo rights required)
#		# start smafewrapd and expect error (102)

testResultClock=0

echo    >> $TESTLOGFILE 2>&1
echo "Setting date to -1 day" >> $TESTLOGFILE 2>&1

if [ -n "$SMAFESETUP_ISLINUX" ]; then
	date --set="-1 day"                 >> $TESTLOGFILE 2>&1
	if [ $? != 0 ]; then
		echo "ERROR setting date. This test $CANONICALTESTNAME will fail."   >> $TESTLOGFILE 2>&1
		echo -n "Date: "  >> $TESTLOGFILE 2>&1
		date  >> $TESTLOGFILE 2>&1
		testResultClock=1
	else
		echo -n "Date: "  >> $TESTLOGFILE 2>&1
		date  >> $TESTLOGFILE 2>&1
	fi
elif [ -n "$SMAFESETUP_ISMAC" ]; then
   echo "WARNING not implemented yet"   >> $TESTLOGFILE 2>&1
   testResultClock=1
fi

# no daemon
$SMAFETESTEXECPREFIX/smafewrapd/smafewrapd  --id=testsmafewrapd --no-daemon --interval -1 -v $SMAFETESTPARAMV --dbconf ../test/resources/localhost-testcasedb-dbconnection.opt  >> $TESTLOGFILE 2>&1
testResult1=$?
echo "This ERROR 1xx is expected!" >> $TESTLOGFILE 2>&1




#### set system clock +  days (sudo rights required)
#		# start smafewrapd and expect error 

echo    >> $TESTLOGFILE 2>&1
echo "Setting date to +7 days" >> $TESTLOGFILE 2>&1

if [ -n "$SMAFESETUP_ISLINUX" ]; then
	date --set="+7 day"                 >> $TESTLOGFILE 2>&1
	if [ $? != 0 ]; then
		echo "ERROR setting date. This test $CANONICALTESTNAME will fail."   >> $TESTLOGFILE 2>&1
		echo -n "Date: "  >> $TESTLOGFILE 2>&1
		date  >> $TESTLOGFILE 2>&1
		testResultClock=1
	else
		echo -n "Date: "  >> $TESTLOGFILE 2>&1
		date  >> $TESTLOGFILE 2>&1
	fi
elif [ -n "$SMAFESETUP_ISMAC" ]; then
   echo "WARNING not implemented yet"   >> $TESTLOGFILE 2>&1
   testResultClock=1
fi

# no daemon
$SMAFETESTEXECPREFIX/smafewrapd/smafewrapd  --id=testsmafewrapd  --interval -1  --no-daemon -v $SMAFETESTPARAMV --dbconf ../test/resources/localhost-testcasedb-dbconnection.opt  >> $TESTLOGFILE 2>&1
testResult2=$?
echo "This ERROR 1xx is expected!" >> $TESTLOGFILE 2>&1





#### set system clock +  months (sudo rights required)
#		# start smafewrapd and expect error 

echo    >> $TESTLOGFILE 2>&1
echo "Setting date to +20years" >> $TESTLOGFILE 2>&1

if [ -n "$SMAFESETUP_ISLINUX" ]; then
	date --set="+20 year"                 >> $TESTLOGFILE 2>&1
	if [ $? != 0 ]; then
		echo "ERROR setting date. This test $CANONICALTESTNAME will fail."   >> $TESTLOGFILE 2>&1
		echo -n "Date: "  >> $TESTLOGFILE 2>&1
		date  >> $TESTLOGFILE 2>&1
		testResultClock=1
	else
		echo -n "Date: "  >> $TESTLOGFILE 2>&1
		date  >> $TESTLOGFILE 2>&1
	fi
elif [ -n "$SMAFESETUP_ISMAC" ]; then
   echo "WARNING not implemented yet"   >> $TESTLOGFILE 2>&1
   testResultClock=1
fi

# no daemon
$SMAFETESTEXECPREFIX/smafewrapd/smafewrapd  --id=testsmafewrapd  --interval -1  --no-daemon -v $SMAFETESTPARAMV --dbconf ../test/resources/localhost-testcasedb-dbconnection.opt  >> $TESTLOGFILE 2>&1
testResult3=$?
echo "This ERROR 1xx is expected!" >> $TESTLOGFILE 2>&1







### resetting clock back to normal

echo    >> $TESTLOGFILE 2>&1
echo "Setting date back to normal" >> $TESTLOGFILE 2>&1


if [ -n "$SMAFESETUP_ISLINUX" ]; then
	hwclock --hctosys
	if [ $? != 0 ]; then
		echo "ERROR setting date. This test $CANONICALTESTNAME will fail."   >> $TESTLOGFILE 2>&1
		echo -n "Date: "  >> $TESTLOGFILE 2>&1
		date  >> $TESTLOGFILE 2>&1
		testResultClock=1
	else
		echo -n "Date: "  >> $TESTLOGFILE 2>&1
		date  >> $TESTLOGFILE 2>&1
	fi
elif [ -n "$SMAFESETUP_ISMAC" ]; then
   echo "WARNING not implemented yet"   >> $TESTLOGFILE 2>&1
   testResultClock=1
fi



echo    >> $TESTLOGFILE 2>&1


################### BEGIN footer ##############################

# also check if no errors (EE) and no fatal errors (FF) occured
# Note: grep returns 0 if at least one occurence is found, 1 if not a singel one is found, and 2 for errors
grep " EE " $TESTLOGFILE
testErrors1=$?
grep "FF ERROR 102." -v  $TESTLOGFILE  | grep "FF ERROR 103." -v | grep "FF ERROR 104." -v | grep " FF "
testErrors2=$?



echo "Variable testResultClock is now set to $testResultClock. (expected == 0)"		>> $TESTLOGFILE
echo "Variable testResult1 is now set to $testResult1. (expected != 0)"			>> $TESTLOGFILE
echo "Variable testResult2 is now set to $testResult2. (expected != 0)"			>> $TESTLOGFILE
echo "Variable testResult3 is now set to $testResult3. (expected != 0)"			>> $TESTLOGFILE
echo "Variable testResultc1 is now set to $testResultc1.(expected 0)"			>> $TESTLOGFILE
echo "Variable testErrors1 is now set to $testErrors1.(expected 1)"				>> $TESTLOGFILE
echo "Variable testErrors2 is now set to $testErrors2.(expected 1)"				>> $TESTLOGFILE


# move logfile to final destination
mv $TESTLOGFILE $SMAFETESTEXECPREFIX/





# exit with success / error code
if [  $testResultClock == 0 -a $testResult1 != 0 -a $testResult2 != 0 -a $testResult3 != 0 -a  $testResultc1 == 0 -a $testErrors1 == 1 -a $testErrors2 == 1 ]
then
  echo "$CANONICALTESTNAME completed successfully."
  exit 0 
else 
  echo "There was an error while running the $CANONICALTESTNAME test! For details see $SMAFETESTEXECPREFIX/$(basename $TESTLOGFILE)!"
  exit 1
fi 
