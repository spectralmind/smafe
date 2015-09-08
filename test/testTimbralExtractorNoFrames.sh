#!/bin/bash
# online resources: 
# http://steve-parker.org/sh/sh.shtml
# http://www.freeos.com/guides/lsst/
# http://www.ss64.com/bash/


##########################################################################################
#
#	This script verifies that the timbral extractor correctly refuses to process files
#		which have 0 frames
#
#-----------------------------------------------------------------------------------------
#
#	Actions:
#		# calls smafewrap / timbral extractor on small file with not enough audio data for 
#			one frame
#
#-----------------------------------------------------------------------------------------
#
#	Note:
#
##########################################################################################


# set name of test for logfile
CANONICALTESTNAME=testTimbralExtractorNoFrames
TESTLOGFILE=../test/$CANONICALTESTNAME.log

# this test will try to run smafewrap with a given options file 
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

### insert files
INPATH=../test/resources/$CANONICALTESTNAME/*.mp3
doFileAdd


# run smafewrap  
$SMAFETESTEXECPREFIX/smafewrapd/smafewrapd --no-daemon --id=test --interval -1 -v $SMAFETESTPARAMV --dbconf ../test/resources/localhost-testcasedb-dbconnection.opt >> $TESTLOGFILE  2>&1
testResult1=$?



# post processing: check if files are moved
# NOTE this error is expected since the files are invalid and therefor not correctly processesd and moved!
MAKECLEAN=yes
doFileCleanup
testResultc1=$testResultcheck



echo "Variable testResult1 is now set to $testResult1. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testResultc1 is now set to $testResultc1. (expected 1)" 			>> $TESTLOGFILE



# move logfile to final destination
mv $TESTLOGFILE $SMAFETESTEXECPREFIX/


# exit with success / error code
if [ $testResult1 == 0 -a $testResultc1 == 1 ]
then
  echo "$CANONICALTESTNAME complete"
  exit 0 
else 
  echo "There was an error while running the $CANONICALTESTNAME test! For details see $SMAFETESTEXECPREFIX/$(basename $TESTLOGFILE) "
  exit 1
fi 



# todo

# check if file exists (must not!)
if [ -e  "out.$CANONICALTESTNAME.TIMEDOM.vec" ] 
then
  echo "File out.$CANONICALTESTNAME.TIMEDOM.vec exists but should not since it (supposedly) has 0 frames"
  echo "There was an error while running the $CANONICALTESTNAME test! For details see $SMAFETESTEXECPREFIX/$(basename $TESTLOGFILE) "
  exit 1
fi

# check if file exists (must not!)
if [ -e  "out.$CANONICALTESTNAME.SPECTRAL.vec" ] 
then
  echo "File out.$CANONICALTESTNAME.SPECTRAL.vec exists but should not  since it (supposedly) has 0 frames"
  echo "There was an error while running the $CANONICALTESTNAME test! For details see $SMAFETESTEXECPREFIX/$(basename $TESTLOGFILE) "
  exit 1
fi



# remove out files
rm -f out.$CANONICALTESTNAME*

echo "$CANONICALTESTNAME complete"
exit 0 
