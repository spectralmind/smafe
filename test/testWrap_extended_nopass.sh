#!/bin/bash
# online resources: 
# http://steve-parker.org/sh/sh.shtml
# http://www.freeos.com/guides/lsst/
# http://www.ss64.com/bash/


##########################################################################################
#
#	This script tests functions of smafewrapd that are only available if
#   NOT compiled for deployment. Uses NO passphrase
#
#-----------------------------------------------------------------------------------------
#
#	Actions:
#
#		# start smafewrapd 
#			- in no-daemon mode
#			- using a directory as input
#			- admin mode (give parameters directly from opt file)
#			- text file output
#			- extract RP, RH, SSD, TIMEDOM, TSSD, SPECTRAL
#
#		# check existance of somlib files
#		# check if DATA_TYPE is in somlib file (ie, files are not encrypted)
#
#-----------------------------------------------------------------------------------------
#
#	Note:
#
##########################################################################################

# set name of test for logfile
CANONICALTESTNAME=testWrap_extended_nopass
TESTLOGFILE=../test/$CANONICALTESTNAME.log

# this test will try to run smafewrapd with a given options file 
echo "Running test $CANONICALTESTNAME.... "

#check env
. ../test/checkEnv.lib
# set to user
PGUSERSET=user
setSmafePGUser  > $TESTLOGFILE 2>&1
checkEnv >> $TESTLOGFILE 2>&1



# first, delete vector file
rm -rf testoutput.*.vec
# run smafewrap
$SMAFETESTEXECPREFIX/smafewrapd/smafewrapd --admin ../test/resources/$CANONICALTESTNAME/test.opt  --no-daemon --text=testoutput --id=test --interval -1   ../test/resources/ >> $TESTLOGFILE 2>&1
testResult3=$?

# also check if file exist
if [ ! -f testoutput.TIMEDOM.vec  -o  ! -f testoutput.RH.vec  -o  ! -f testoutput.RP.vec  -o  ! -f testoutput.SSD.vec ]
then
	echo "somlib vector file(s) missing!" >>  $TESTLOGFILE
	testResult3=1
fi  


# check if file is unecrypted
grep DATA_TYPE testoutput.RH.vec >>  $TESTLOGFILE
testResult4=$?



# also check if no errors (EE) and no fatal errors (FF) occured
# Note: grep returns 0 if at least one occurence is found, 1 if not a singel one is found, and 2 for errors
grep " EE " $TESTLOGFILE
testErrors1=$?
grep " FF " $TESTLOGFILE
testErrors2=$?



echo "Variable testResult3 is now set to $testResult3.(expected 0)"			>> $TESTLOGFILE
echo "Variable testResult4 is now set to $testResult4.(expected 0)"			>> $TESTLOGFILE
echo "Variable testErrors1 is now set to $testErrors1.(expected 1)"				>> $TESTLOGFILE
echo "Variable testErrors2 is now set to $testErrors2.(expected 1)"				>> $TESTLOGFILE



# move logfile to final destination
mv $TESTLOGFILE $SMAFETESTEXECPREFIX/


# exit with success / error code
if [  $testResult3 == 0 -a $testResult4 == 0 -a $testErrors1 == 1 -a $testErrors2 == 1 ] 
then
  echo "$CANONICALTESTNAME completed successfully."
  exit 0 
else 
  echo "There was an error while running the $CANONICALTESTNAME test! For details see $SMAFETESTEXECPREFIX/$(basename $TESTLOGFILE)!"
  exit 1
fi 
