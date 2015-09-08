#!/bin/bash
# online resources: 
# http://steve-parker.org/sh/sh.shtml
# http://www.freeos.com/guides/lsst/
# http://www.ss64.com/bash/


##########################################################################################
#
#	This script tests the decryption tool.
#
#-----------------------------------------------------------------------------------------
#
#	Actions:
#
#		# executes smafeDecrypt with no passphrase -> check for error
#		# executes smafeDecrypt with passphrase and un-decryptable ciphertext ->  check error
#		# executes smafeDecrypt with passphrase and decryptable ciphertext -> check no error, check plaintext
#		# executes smafeDecrypt with passphrase and decryptable ciphertext (contains \012, as if from db) ->  check no error, check plaintext
#
#-----------------------------------------------------------------------------------------
#
#	Note:
#
##########################################################################################

# set name of test for logfile
CANONICALTESTNAME=testDecrypt
TESTLOGFILE=../test/$CANONICALTESTNAME.log

# this test will try to run smafewrapd with a given options file 
echo "Running test $CANONICALTESTNAME.... "
#echo "Interim logfile is $TESTLOGFILE"

#check env
. ../test/checkEnv.lib
# prepare log file
rm -f $TESTLOGFILE
touch $TESTLOGFILE


checkEnv >> $TESTLOGFILE 2>&1


# run #1 
$SMAFETESTEXECPREFIX/smafedecrypt/smafedecrypt  >> $TESTLOGFILE 2>&1
testResult1=$?
# error expected



# run #2 
echo "illegal ciphertext" | $SMAFETESTEXECPREFIX/smafedecrypt/smafedecrypt -p pass  >> $TESTLOGFILE 2>&1
testResult2=$?
# error expected


# run #3 
$SMAFETESTEXECPREFIX/smafedecrypt/smafedecrypt -p soundminer  >> $TESTLOGFILE 2>&1 < ../test/resources/$CANONICALTESTNAME/cipher1.txt
testResult3=$?
# no error expected
grep "$(cat ../test/resources/$CANONICALTESTNAME/plain1.txt)" $TESTLOGFILE >> /dev/null 2>&1
testResultGrep3=$?
echo "testResultGrep3=$testResultGrep3" >> $TESTLOGFILE
# should find it


# run #4 
$SMAFETESTEXECPREFIX/smafedecrypt/smafedecrypt -p fxON+VAL  >> $TESTLOGFILE 2>&1 < ../test/resources/$CANONICALTESTNAME/cipher2.txt
testResult4=$?
# no error expected
grep "$(cat ../test/resources/$CANONICALTESTNAME/plain2.txt)" $TESTLOGFILE >> /dev/null 2>&1
testResultGrep4=$?
echo "testResultGrep4=$testResultGrep4" >> $TESTLOGFILE
# should find it








# also check if no errors (EE) and no fatal errors (FF) occured
# Note: grep returns 0 if at least one occurence is found, 1 if not a singel one is found, and 2 for errors
grep " EE " $TESTLOGFILE
testErrors1=$?
grep " FF " $TESTLOGFILE
testErrors2=$?



echo "Variable testResult1 is now set to $testResult1.(expected != 0)"			>> $TESTLOGFILE
echo "Variable testResult2 is now set to $testResult2.(expected != 0)"			>> $TESTLOGFILE
echo "Variable testResult3 is now set to $testResult3.(expected 0)"			>> $TESTLOGFILE
echo "Variable testResultGrep3 is now set to $testResultGrep3.(expected 0)"			>> $TESTLOGFILE
echo "Variable testResult4 is now set to $testResult4.(expected 0)"			>> $TESTLOGFILE
echo "Variable testResultGrep4 is now set to $testResultGrep4.(expected 0)"			>> $TESTLOGFILE
echo "Variable testErrors1 is now set to $testErrors1.(expected 1)"				>> $TESTLOGFILE
echo "Variable testErrors2 is now set to $testErrors2.(expected 1)"				>> $TESTLOGFILE


# move logfile to final destination
mv $TESTLOGFILE $SMAFETESTEXECPREFIX/





# exit with success / error code
if [ $testResult1 != 0 -a $testResult2 != 0 -a $testResult3 == 0 -a $testResultGrep3 == 0   -a $testResult4 == 0 -a $testResultGrep4 == 0  -a $testErrors1 == 1 -a $testErrors2 == 1 ]
then
  echo "$CANONICALTESTNAME completed successfully."
  exit 0 
else 
  echo "There was an error while running the $CANONICALTESTNAME test! For details see $SMAFETESTEXECPREFIX/$(basename $TESTLOGFILE)!"
  exit 1
fi 
