#!/bin/bash
# online resources: 
# http://steve-parker.org/sh/sh.shtml
# http://www.freeos.com/guides/lsst/
# http://www.ss64.com/bash/

##########################################################################################
#
#	This script tests the "live" functionality of smafedistd.
#
#-----------------------------------------------------------------------------------------
#
#	Actions:
#		# expects empty database with official db dump
#		# generates config sql file from special test.opt, insert that sql file into db and delete file
#		# dump some data into the db (new collection, two tracks, feature vectors, one fv type, config for this test)
#		# start smafedistd in live mode ("server") using --portion 1
#		# Start second server on same port and check if fails (because port is taken)
#		# simulate client via nc (netcat), send feature vector
#		# check if fv is received by server
#		# simulate server via nc (netcat), start real client and compare result with reference
#		# start real server and real client and restrict to collection id 3 (rules out track 1) and 2 NN 
#		# (database and users are deleted by a following Cleanup test)
#
#	Notes:
#		# Since there are a variety of netcat programs out there:
#			the one that this test is built upon is : [v1.10-38] (not the GNU version)
#
#-----------------------------------------------------------------------------------------
#
#	History:
#		2010-08-03		initial version				ep
#		2011-06-24		added new test: third run with collection id and top k restriction
#
##########################################################################################


# set name of test for logfile
CANONICALTESTNAME=testDistd_live
TESTLOGFILE=../test/$CANONICALTESTNAME.log
 
echo "Running test $CANONICALTESTNAME.... "

#check env
. ../test/checkEnv.lib

# prepare log file
rm -f $TESTLOGFILE
touch $TESTLOGFILE

checkEnv >> $TESTLOGFILE 2>&1


## clean up from old runs
rm -f testsmafedistd.log
rm -f mockup.log
rm -f stdout.log
rm -f stderr.log



# set to user
PGUSERSET=user
setSmafePGUser > $TESTLOGFILE 2>&1



### insert fv, track, file, config
psql  -d $TESTDATABASE -f ../test/resources/$CANONICALTESTNAME/data.dump.sql >> $TESTLOGFILE 2>&1
testResult2=$?
echo "Variable testResult2 is now set to $testResult2. (expected 0)" 			>> $TESTLOGFILE


## db is ready
#exit


export SMAFEPORT=1212
export SMAFEDUMMYANSWER=helloworld

echo															>> $TESTLOGFILE 2>&1 
echo															>> $TESTLOGFILE 2>&1 
echo  -------------------------- real server / mock up client	>> $TESTLOGFILE 2>&1

# check old daemons running
testOlddaemonrunning=$(ps aux |grep -- "-s --dbconf=../test/resources/localhost-testcasedb-dbconnection.opt" | grep -v "/bin/bash"  | grep -v "/bin/sh"   | wc -l)
if [ $testOlddaemonrunning -gt 1 ]; then
   ps aux |grep -- "-s --dbconf=../test/resources/localhost-testcasedb-dbconnection.opt"  | grep -v "/bin/bash"  | grep -v "/bin/sh"
   echo "Note: daemon from previous make check may still be running. Please kill the process. Aborting this test, no cleaning done." 
   exit 1
fi
# run smafedistd in live mode

$SMAFETESTEXECPREFIX/smafedistd/smafedistd  --live --portion 1  -f 1 --log=testsmafedistd.log -s --dbconf=../test/resources/localhost-testcasedb-dbconnection.opt -v  $SMAFETESTPARAMV  >> $TESTLOGFILE 2>&1
testResult1=$?
echo "Variable testResult1 is now set to $testResult1. (expected 0)" 			>> $TESTLOGFILE

# wait secs for startup
sleep 5 

# check if daemon is running (same line is also used below!)
testDaemonrunning=$(ps aux |grep -- "-s --dbconf=../test/resources/localhost-testcasedb-dbconnection.opt"  |grep -v grep  |  wc -l)
# contains number of lines with that id. 
if [ $testDaemonrunning -eq 0 ]; then
	ps aux |grep -- "-s --dbconf=../test/resources/localhost-testcasedb-dbconnection.opt" |grep -v grep >> $TESTLOGFILE 2>&1 
	echo "WARNING: Daemon seems not be running" >> $TESTLOGFILE 2>&1 
	testResult1=1
	echo "Variable testResult1 is now set to $testResult1. (expected 0)" 			>> $TESTLOGFILE
fi



#		#  Start second server on same port and check if fails (because port is taken)
$SMAFETESTEXECPREFIX/smafedistd/smafedistd  --live -f 1 --log=testsmafedistd-should-fail.log -s --dbconf=../test/resources/localhost-testcasedb-dbconnection.opt -v  $SMAFETESTPARAMV  >> $TESTLOGFILE 2>&1
testSecondServer=$?
echo "This fatal error has been provoked *deliberately*" >> $TESTLOGFILE 2>&1
echo "Variable testSecondServer is now set to $testSecondServer. (expected != 0)" 			>> $TESTLOGFILE  
echo  >> $TESTLOGFILE 2>&1 
echo  >> $TESTLOGFILE 2>&1 



# SAHARA#1    
# $ echo GETOPTS |   nc localhost 1212 > result-GETOPTS.reference.txt
# cleanup, start over
#exit




# emulate client with netcat: send GETOPTS, receive scrambled options, discard it
echo "emulate client with netcat: send GETOPTS, receive scrambled options, discard it" >> $TESTLOGFILE 2>&1 
echo GETOPTS |  nc localhost 1212  >> $TESTLOGFILE 2>&1
echo  >> $TESTLOGFILE 2>&1  


# emulate client with netcat: send fv, receive NN
echo "emulate client with netcat: send fv, receive NN"   >> $TESTLOGFILE 2>&1
cat  ../test/resources/$CANONICALTESTNAME/telnet-input-1.txt |  nc localhost 1212  >> $TESTLOGFILE 2>&1
testResult2=$?
echo "Variable testResult2 is now set to $testResult2. (expected 0)" 			>> $TESTLOGFILE
echo  >> $TESTLOGFILE 2>&1  


# check if input was received by server
# grep "$(cat ../test/resources/$CANONICALTESTNAME/telnet-input-1.txt)" testsmafedistd.log  >> $TESTLOGFILE 2>&1
# This would only be successful if debug output in the log is enabled.
# On deploy target, however, this seems not to be the case. So for now we just set the to true.
# testCheck0=$?
testCheck0=0

# check if result from server =~ reference
# The regexp reference allows the both distance values to be any number (number may also contain
# 'e' and '-', eg, in 9.34e-15 
egrep "$(cat ../test/resources/$CANONICALTESTNAME/result-1.regexp.txt)" $TESTLOGFILE  >> /dev/null 2>&1
testCheck1=$?
echo "Variable testCheck1 is now set to $testCheck1.(expected 0)"				>> $TESTLOGFILE


## kill daemon
	# get PID
	DamonPID=$(grep "PID of" testsmafedistd.log | head -n 1 |cut -d " " -f 10)
	# kill the daemon using sigkill (for now)
	kill -sigkill $DamonPID 
	# check if killed
	sleep 1
	testDaemonrunning=$(ps aux |grep  -- "-s --dbconf=../test/resources/localhost-testcasedb-dbconnection.opt" |grep -v grep | grep -v "check-recursive" | wc -l) >> $TESTLOGFILE 2>&1 
	# contains number of lines with that id.
	if [ $testDaemonrunning -ge 1 ]; then
		ps aux |grep  -- "-s --dbconf=../test/resources/localhost-testcasedb-dbconnection.opt"  |grep -v grep | grep -v "check-recursive" >> $TESTLOGFILE 2>&1 
		echo "Daemon still running !?" >> $TESTLOGFILE 2>&1 
		testDaemon2=1
		echo "Variable testDaemon2 is now set to $testDaemon2.(expected 0)" 			>> $TESTLOGFILE
	else
		echo "Daemon killed" >> $TESTLOGFILE 2>&1
		testDaemon2=0
		echo "Variable testDaemon2 is now set to $testDaemon2.(expected 0)" 			>> $TESTLOGFILE
	fi
	testDaemon1=0
	echo "Variable testDaemon1 is now set to $testDaemon1. (expected 0)" 			>> $TESTLOGFILE
	
	
	# check if daemon was in listening mode
	# that is the case if this is the last line of the log
	# if we have more verbose setting, it need not be the last line!
	echo "---"  >> $TESTLOGFILE 2>&1 	
	tail -n 5 testsmafedistd.log   >> $TESTLOGFILE 2>&1
	echo "---"  >> $TESTLOGFILE 2>&1 
	tail -n 5 testsmafedistd.log | grep "Waiting for incoming connection on port $SMAFEPORT"   >> $TESTLOGFILE 2>&1 
	if [ $? -eq 0 ]; then
		testDaemon3=0
		echo "smafedistd seems to be in listening mode, alright"  >> $TESTLOGFILE 2>&1
		echo "Variable testDaemon3 is now set to $testDaemon3.(expected 0)" 			>> $TESTLOGFILE
	else
		testDaemon3=1
		echo "ERROR: smafedistd seems not to be in listening mode"  >> $TESTLOGFILE 2>&1
		echo "Variable testDaemon3 is now set to $testDaemon3.(expected 0)" 			>> $TESTLOGFILE
	fi

# copy daemon log to our test log
echo "-------------------BEGIN log from daemon (testsmafedistd.log)---------------------"  >> $TESTLOGFILE 2>&1 
cat testsmafedistd.log >> $TESTLOGFILE
echo "-------------------END log from daemon (testsmafedistd.log)---------------------"  >> $TESTLOGFILE 2>&1 
# remove log file
rm -f testsmafedistd.log




echo															>> $TESTLOGFILE 2>&1 
echo															>> $TESTLOGFILE 2>&1 
echo  -------------------------- mock up  server / real client >> $TESTLOGFILE 2>&1 


# USE this section for self-closing server
# Problem was: the dummy answer did not arrive on the client side
# start mockup servers
# first nc answers with options (reference file)
# second nc answers with an arbitrary string $SMAFEDUMMYANSWER
# started in background (nonblocking)
# redirecting input to server to file
# second nc uses "start command after connection" because piping an echo leaves the connection open
# NOTE: the second server MUST close the connection otherweise the client blocks waiting for an EOF
#nc -l -p $SMAFEPORT  < ../test/resources/testDistd_live/result-GETOPTS.reference.txt  > mockup.log &&  nc -l -p $SMAFEPORT -c "echo $SMAFEDUMMYANSWER" >> mockup.log  &

# start mockup servers
# first nc answers with options (reference file)
# second nc answers with an arbitrary string $SMAFEDUMMYANSWER
# started in background (nonblocking)
# redirecting input to server to file


# traditional netcat must get -p parameter
# Since Ubuntu 10.04 (I think), the openbsd variant is standard
# If you use newer Linux, please install package netcat-traditional and make sure this
#		version is used for nc on that system. Otherwise this test fails!
# See trac ticket #349 for details.
if [ -n "$SMAFESETUP_ISLINUX" ]; then
	nc -l -p $SMAFEPORT  < ../test/resources/testDistd_live/result-GETOPTS.reference.txt  > mockup.log && echo "$SMAFEDUMMYANSWER" | nc -l -p $SMAFEPORT  >> mockup.log  &
elif [ -n "$SMAFESETUP_ISMAC" ]; then
	nc -l    $SMAFEPORT  < ../test/resources/testDistd_live/result-GETOPTS.reference.txt  > mockup.log && echo "$SMAFEDUMMYANSWER" | nc -l    $SMAFEPORT  >> mockup.log  &
fi

# start client
# split stdout and stderr to different files
$SMAFETESTEXECPREFIX/smafewrapd/smafewrapd  --live --no-daemon --livefile=../test/resources/testthird.mp3 -v 1 > stdout.log 2> stderr.log  
testResult3=$?
echo "Variable testResult3 is now set to $testResult3. (expected 0)" 			>> $TESTLOGFILE


# check if stdout == dummy answer
# diff returns 0 if no difference, 1 if difference
echo $SMAFEDUMMYANSWER | diff - stdout.log  >> $TESTLOGFILE 2>&1
testCheck3=$?
echo "Variable testCheck3 is now set to $testCheck3.(expected 0)"				>> $TESTLOGFILE

# append log files to main log
echo "-------------------BEGIN mockup server output---------------------"  >> $TESTLOGFILE 2>&1 
cat mockup.log >> $TESTLOGFILE
echo "-------------------END mockup server output---------------------"  >> $TESTLOGFILE 2>&1 
rm -f mockup.log
echo "-------------------BEGIN client std out stream--------------------"  >> $TESTLOGFILE 2>&1 
cat stdout.log >> $TESTLOGFILE
echo "-------------------END client std out stream---------------------"  >> $TESTLOGFILE 2>&1 
rm -f stdout.log
echo "-------------------BEGIN client std err stream (log)--------------------"  >> $TESTLOGFILE 2>&1 
cat stderr.log >> $TESTLOGFILE
echo "-------------------END client std err stream (log)---------------------"  >> $TESTLOGFILE 2>&1 
rm -f stderr.log



echo															>> $TESTLOGFILE 2>&1 
echo															>> $TESTLOGFILE 2>&1 
echo  -------------------------- real server / real client	>> $TESTLOGFILE 2>&1


## START (copied from above, adapted $testXXX vars)
# check old daemons running
testOlddaemonrunning=$(ps aux |grep -- "-s --dbconf=../test/resources/localhost-testcasedb-dbconnection.opt" | grep -v "/bin/bash"  | grep -v "/bin/sh"   | wc -l)
if [ $testOlddaemonrunning -gt 1 ]; then
   ps aux |grep -- "-s --dbconf=../test/resources/localhost-testcasedb-dbconnection.opt"  | grep -v "/bin/bash"  | grep -v "/bin/sh"
   echo "Note: daemon from previous make check may still be running. Please kill the process. Aborting this test, no cleaning done." 
   exit 1
fi
# run smafedistd in live mode

#valgrind --leak-check=yes --trace-children=yes $SMAFETESTEXECPREFIX/smafedistd/smafedistd  --live -f 1 --log=testsmafedistd.log -s --dbconf=../test/resources/localhost-testcasedb-dbconnection.opt -v  $SMAFETESTPARAMV  >> $TESTLOGFILE 2>&1
$SMAFETESTEXECPREFIX/smafedistd/smafedistd  --live -f 1 --log=testsmafedistd.log -s --dbconf=../test/resources/localhost-testcasedb-dbconnection.opt -v  $SMAFETESTPARAMV  >> $TESTLOGFILE 2>&1
testResultRR1=$?
echo "Variable testResultRR1 is now set to $testResultRR1. (expected 0)" 			>> $TESTLOGFILE

# wait secs for startup
sleep 5 

# check if daemon is running (same line is also used below!)
testDaemonrunning=$(ps aux |grep -- "-s --dbconf=../test/resources/localhost-testcasedb-dbconnection.opt"  |grep -v grep  |  wc -l)
# contains number of lines with that id. 
if [ $testDaemonrunning -eq 0 ]; then
	ps aux |grep -- "-s --dbconf=../test/resources/localhost-testcasedb-dbconnection.opt" |grep -v grep >> $TESTLOGFILE 2>&1 
	echo "WARNING: Daemon seems not be running" >> $TESTLOGFILE 2>&1 
	testResultRR1=1
fi
## END (copied from above)


# start client
# split stdout and stderr to different files
# restrict to collection id 3
# only 2 results
$SMAFETESTEXECPREFIX/smafewrapd/smafewrapd  --live --livecollectionid 3 --livetopk 2 --no-daemon --livefile=../test/resources/testthird.mp3 -v 1 > stdout.log 2> stderr.log  
testResultRR3=$?
echo "Variable testResultRR3 is now set to $testResultRR3. (expected 0)" 			>> $TESTLOGFILE


# check if result from server =~ reference
# The reference contains the CSV header, and exactly one line with track id 2 and distance starting with 62
# The check is twofold:  1) check if all lines are matching, 2) check if the reverse is empty (=ONLY the lines we check for are present)
# Info: -w : only whole lines
#		-v : inverse 
egrep -w "$(cat ../test/resources/$CANONICALTESTNAME/result-2.regexp.txt)" stdout.log  >> $TESTLOGFILE 2>&1
testCheckRR1=$?
echo "Variable testCheckRR1 is now set to $testCheckRR1.(expected 0)"				>> $TESTLOGFILE

egrep -wv "$(cat ../test/resources/$CANONICALTESTNAME/result-2.regexp.txt)" stdout.log  >> $TESTLOGFILE 2>&1
testCheckRR1v=$?
echo "Variable testCheckRR1 is now set to $testCheckRR1.(expected 0)"				>> $TESTLOGFILE
echo "Variable testCheckRR1v is now set to $testCheckRR1v.(expected 1)"				>> $TESTLOGFILE


echo "-------------------BEGIN client std out stream--------------------"  >> $TESTLOGFILE 2>&1 
cat stdout.log >> $TESTLOGFILE
echo "-------------------END client std out stream---------------------"  >> $TESTLOGFILE 2>&1 
rm -f stdout.log
echo "-------------------BEGIN client std err stream (log)--------------------"  >> $TESTLOGFILE 2>&1 
cat stderr.log >> $TESTLOGFILE
echo "-------------------END client std err stream (log)---------------------"  >> $TESTLOGFILE 2>&1 
rm -f stderr.log


## START copy from above
## kill daemon
	# get PID
	DamonPID=$(grep "PID of" testsmafedistd.log | head -n 1 |cut -d " " -f 10)
	# kill the daemon using sigkill (for now)
	kill -sigkill $DamonPID 
	# check if killed
	sleep 1
	testDaemonrunning=$(ps aux |grep  -- "-s --dbconf=../test/resources/localhost-testcasedb-dbconnection.opt" |grep -v grep | grep -v "check-recursive" | wc -l) >> $TESTLOGFILE 2>&1 
	# contains number of lines with that id.
	if [ $testDaemonrunning -ge 1 ]; then
		ps aux |grep  -- "-s --dbconf=../test/resources/localhost-testcasedb-dbconnection.opt"  |grep -v grep | grep -v "check-recursive" >> $TESTLOGFILE 2>&1 
		echo "Daemon still running !?" >> $TESTLOGFILE 2>&1 
		testDaemonRR2=1
	else
		echo "Daemon killed" >> $TESTLOGFILE 2>&1
		testDaemonRR2=0	
	fi
	testDaemonRR1=0
	

# copy daemon log to our test log
echo "-------------------BEGIN log from daemon (testsmafedistd.log)---------------------"  >> $TESTLOGFILE 2>&1 
cat testsmafedistd.log >> $TESTLOGFILE
echo "-------------------END log from daemon (testsmafedistd.log)---------------------"  >> $TESTLOGFILE 2>&1 
# remove log file
rm -f testsmafedistd.log
## END copy


# -------------------------- clean up
# -> done afterwards, not in this script!




# also check if no warnings,  errors (EE) and no fatal errors (FF) occured
# Note: grep returns 0 if at least one occurence is found, 1 if not a singel one is found, and 2 for errors
grep " WW " $TESTLOGFILE
testErrors0=$?
grep " EE " $TESTLOGFILE
testErrors1=$?
grep "FF Port 1212 is already in use." -v $TESTLOGFILE | grep " FF " 
testErrors2=$?


echo "Variable testDaemon1 is now set to $testDaemon1. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testDaemon2 is now set to $testDaemon2.(expected 0)" 			>> $TESTLOGFILE
echo "Variable testDaemon3 is now set to $testDaemon3.(expected 0)" 			>> $TESTLOGFILE
#echo "Variable testResultconfig1 is now set to $testResultconfig1. (expected 0)" 			>> $TESTLOGFILE
#echo "Variable testResultconfig2 is now set to $testResultconfig2. (expected 0)" 			>> $TESTLOGFILE
#echo "Variable testResultconfig3 is now set to $testResultconfig3. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testResult1 is now set to $testResult1. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testSecondServer is now set to $testSecondServer. (expected != 0)" 			>> $TESTLOGFILE
echo "Variable testResult2 is now set to $testResult2. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testResult3 is now set to $testResult3. (expected 0)" 			>> $TESTLOGFILE
#echo "Variable testResult4 is now set to $testResult4. (expected 0)" 			>> $TESTLOGFILE
#echo "Variable testResult5 is now set to $testResult5. (expected 0)" 			>> $TESTLOGFILE
#echo "Variable testResult6 is now set to $testResult6. (expected 0)" 			>> $TESTLOGFILE
#echo "Variable testResult7 is now set to $testResult7. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testCheck0 is now set to $testCheck0.(expected 0, NB: override in action)"				>> $TESTLOGFILE
echo "Variable testCheck1 is now set to $testCheck1.(expected 0)"				>> $TESTLOGFILE
#echo "Variable testCheck2 is now set to $testCheck2.(expected 0)"				>> $TESTLOGFILE
echo "Variable testCheck3 is now set to $testCheck3.(expected 0)"				>> $TESTLOGFILE

# third
echo "Variable testResultRR1 is now set to $testResultRR1. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testResultRR3 is now set to $testResultRR3. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testCheckRR1 is now set to $testCheckRR1.(expected 0)"				>> $TESTLOGFILE
echo "Variable testCheckRR1v is now set to $testCheckRR1v.(expected 1)"				>> $TESTLOGFILE
echo "Variable testDaemonRR1 is now set to $testDaemonRR1. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testDaemonRR2 is now set to $testDaemonRR2.(expected 0)" 			>> $TESTLOGFILE

echo "Variable testErrors0 is now set to $testErrors0.(expected 1)"				>> $TESTLOGFILE
echo "Variable testErrors1 is now set to $testErrors1.(expected 1)"				>> $TESTLOGFILE
echo "Variable testErrors2 is now set to $testErrors2.(expected 1)"				>> $TESTLOGFILE





# move logfile to final destination
mv $TESTLOGFILE $SMAFETESTEXECPREFIX/


# exit with success / error code
if [ $testDaemon1 == 0 -a $testDaemon2 == 0 -a $testDaemon3 == 0 -a $testResult1 == 0 -a $testSecondServer != 0 -a $testResult2 == 0 -a $testResult3 == 0 -a $testCheck0 == 0 -a $testCheck1 == 0 -a $testCheck3 == 0 -a $testResultRR1 == 0 -a $testResultRR3 == 0 -a $testCheckRR1 == 0 -a $testCheckRR1v == 1 -a $testDaemonRR1 == 0 -a $testDaemonRR2 == 0 -a $testErrors0 == 1 -a $testErrors1 == 1 -a $testErrors2 == 1 ] 
then
  echo "$CANONICALTESTNAME complete"
  exit 0 
else 
  echo "There was an error while running the $CANONICALTESTNAME test! For details see $SMAFETESTEXECPREFIX/$(basename $TESTLOGFILE) "
  exit 1
fi 
