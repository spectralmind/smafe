#!/bin/bash
# online resources: 
# http://steve-parker.org/sh/sh.shtml
# http://www.freeos.com/guides/lsst/
# http://www.ss64.com/bash/


##########################################################################################
#
#	This script tests smafewrapd in jobs mode: standard feature extraction, addfile job 
#			management, re-use of feature vector types, re-use of tracks, delete jobs, 
#			default collection handling (_removed_)
#
#-----------------------------------------------------------------------------------------
#
#	Actions:
#		# remove any possibly left over files from earlier runs
#
#		(first run)
#		# iterate through test resources and take files that start with test (3 files)
#			- create copy named with suffix fortesting
#			- insert smafejob_addfile job in db for the copy of this file
#		# start smafewrapd 
#			- using standard db connection params
#			- stored config params
#			- daemon mode
#			- 66 minutes intveral
#			- set id and logfile
#			- also store segments' features
#		# for each of the files, check if they are moved (must not exist in original location and must
#				exist in new location. The new location is assumed to be /tmp/ (must correspond to example.opt!)
#		# clean up: remove files that might have been created (fortesting copies) IF no problems occured
#				That means: if the "has file been moved?" checks do not end successfully, the files are not
#				deleted to give the developer a chance to check manually.
#
#		(second run)
#		analogous to first run, but use file testthird*.mp3 			(1 file but it is a copy, 
#				so we still have 3 tracks at the end!)
#		This checks if earlier stored featurevector types are found and can be used again as well as
#				if tracks are re-used for files that have the same fingerprint.
#		Does not run as daemon
##
#		# inserts deletefile jobs
#			- delete from all collections
#			- delete from one collection
#		# start smafewrapd using standard db connection and stored config params, no daemon mode
#		# check contents of collection_file table and compare with reference
#		# check contents of featurevectorsegment table and compare with reference
#
#
#		# addfile jobs again
#		("fourth" run)
#		analogous to second run, but use the second file from the first run from test resource folder
#		Because this file was deleted in a deletefilejob it should after that run be in the global and specific collection again
#			and not in the removed collection!
#		Compare collection-file Zuordnung with referance (note that file 2 must not be in collectoin 1 (_removed) anymore and
#			that there is not file 5 (since the file from this job is the same as file 2)
##
#		(third run)
#		analogous to second run, but use files limit_too_many*.mp3 from test resource folder
#		Due to the limit, the second job and file should not be processed
#		Check if warning (limit reached) is in log file.
#		Check if exactly one failed job in addfile job table
#
#
#		# check for EE and FF in log file and do var checking if error occured 
#
#
#-----------------------------------------------------------------------------------------
#
#	Note:
#		hardcoded stuff:
#			- file destination is /tmp
#			- files are named test*.mp3 and testthird*.mp3
#			- reference sql output in subdir test/testWrap
#
##########################################################################################

# set name of test for logfile
CANONICALTESTNAME=testWrap
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
	# set to user user
	PGUSERSET=user
	setSmafePGUser >> $TESTLOGFILE 2>&1
fi
checkEnv >> $TESTLOGFILE 2>&1


################ init 
# remove files left over
rm -f ../test/resources/$CANONICALTESTNAME/*fortesting* >> $TESTLOGFILE 2>&1
rm -f ../test/resources/*fortesting* >> $TESTLOGFILE 2>&1
rm -f testsmafewrapd.log
rm -f /tmp/*fortesting* >> $TESTLOGFILE 2>&1


#################################### first run

### insert files
# if you change something here, 
# have a look at the forth run because there we use the second file from here again!
#INPATH=$(pwd)/../test/resources/$CANONICALTESTNAME/test*.mp3



# Save and change IFS
# After some experiments, it seems that the IFS variable must be set to only line break
# - for ls command (storing files in INPATH), AND
# - for loop
OLDIFS=$IFS
IFS=$'\n'
INPATH=($( ls $(pwd)/../test/resources/$CANONICALTESTNAME/test*.mp3 | sort -d))
#for i in  ${INPATH[*]}; do 
#	echo $i
#done
# do the file add
doFileAdd
# Restore it
IFS=$OLDIFS




echo "NOTE: 3 files should have been processed!"  >> $TESTLOGFILE 2>&1 


# run smafewrapd (output will be written to $TESTLOGFILE 2>&1 redirects errors to stdout) 
#$SMAFETESTEXECPREFIX/smafewrapd/smafewrapd --no-daemon --id=test --interval -1 -v $SMAFETESTPARAMV --dbconf ../test/resources/localhost-testcasedb-dbconnection.opt  >> $TESTLOGFILE 2>&1
#testResult1=$?

# check old daemons running
testOlddaemonrunning=$(ps aux |grep testsmafewrapd | wc -l)
if [ $testOlddaemonrunning -gt 1 ]; then
   ps aux |grep testsmafewrapd 
   echo "Note: daemon from previous make check still running. Pease kill the process. Aborting this test, no cleaning done." 
   exit 1
fi
# run smafewrapd as daemon
# interval 66
$SMAFETESTEXECPREFIX/smafewrapd/smafewrapd  --id=testsmafewrapd --log testsmafewrapd.log --interval 66 -v $SMAFETESTPARAMV --dbconf ../test/resources/localhost-testcasedb-dbconnection.opt  >> $TESTLOGFILE 2>&1
testResult1=$?

# wait 2 secs for startup
sleep 2 

# check if daemon is running (same line is also used below!)
testDaemonrunning=$(ps aux |grep testsmafewrapd  |grep -v grep  |  wc -l)
# contains number of lines with that id. 
if [ $testDaemonrunning -eq 0 ]; then
	ps aux |grep testsmafewrapd |grep -v grep >> $TESTLOGFILE 2>&1 
	echo "WARNING: Daemon seems not be running" >> $TESTLOGFILE 2>&1 
	testResult1=1
fi

DAEMONSLEEPING=0
SLEEPFORSECS=5
COUNTER=0
SLEEPLOOPS=15
until [ $DAEMONSLEEPING -eq 1 -o $COUNTER -ge $SLEEPLOOPS ]; do
	# sleep for 5 seconds at let the daemon do its work
	echo "Sleeping for $SLEEPFORSECS seconds..." >> $TESTLOGFILE 2>&1 
	sleep $SLEEPFORSECS
	let COUNTER=COUNTER+1
	
	# check if daemon is in sleep mode
	# that is the case if we find this line in the log file
	# interval was 66!
	grep "Going to sleep for 66 minutes..." testsmafewrapd.log  >> $TESTLOGFILE 2>&1 
	if [ $? -eq 0 ]; then
		DAEMONSLEEPING=1
	else
		DAEMONSLEEPING=0
	fi
done
if [ $DAEMONSLEEPING -eq 0 -a $COUNTER -ge $SLEEPLOOPS ]; then
	ps aux |grep testsmafewrapd |grep -v grep  >> $TESTLOGFILE 2>&1 
	echo "Waited $(($COUNTER*$SLEEPFORSECS)) seconds, but daemon still does not seem to be in sleeping mode." >> $TESTLOGFILE 2>&1 
	testDaemon1=1
else
	# get PID
	DamonPID=$(grep "PID of" testsmafewrapd.log | head -n 1 |cut -d " " -f 10)
	# kill the daemon
	kill $DamonPID
	# check if killed
	sleep 1
	testDaemonrunning=$(ps aux |grep testsmafewrapd |grep -v grep | wc -l) >> $TESTLOGFILE 2>&1 
	# contains number of lines with that id.
	if [ $testDaemonrunning -ge 1 ]; then
		ps aux |grep testsmafewrapd |grep -v grep >> $TESTLOGFILE 2>&1 
		echo "Daemon still running !?" >> $TESTLOGFILE 2>&1 
		testDaemon2=1
	else
		echo "Daemon killed" >> $TESTLOGFILE 2>&1
		testDaemon2=0	
	fi
	testDaemon1=0
fi
# copy daemon log to our test log
echo "-------------------BEGIN log from daemon---------------------"  >> $TESTLOGFILE 2>&1 
cat testsmafewrapd.log >> $TESTLOGFILE
echo "-------------------END log from daemon---------------------"  >> $TESTLOGFILE 2>&1 
# remove log file
rm -f testsmafewrapd.log

# post processing: check if files are moved
MAKECLEAN=no
doFileCleanup
testResultc1=$testResultcheck



################ second run

### insert files
# Save and change IFS
# After some experiments, it seems that the IFS variable must be set to only line break
# - for ls command (storing files in INPATH), AND
# - for loop
OLDIFS=$IFS
IFS=$'\n'
INPATH=($( ls $(pwd)/../test/resources/testthird*.mp3 | sort -d))
#for i in  ${INPATH[*]}; do 
#	echo $i
#done
# do the file add
doFileAdd
# Restore it
IFS=$OLDIFS

echo "NOTE: one file (1) should have been processed!"  >> $TESTLOGFILE 2>&1



$SMAFETESTEXECPREFIX/smafewrapd/smafewrapd --no-daemon --id=test --interval -1 -v $SMAFETESTPARAMV --dbconf ../test/resources/localhost-testcasedb-dbconnection.opt  >> $TESTLOGFILE 2>&1
testResult2=$?


# post processing: check if files are moved
MAKECLEAN=no
doFileCleanup
testResultc2=$testResultcheck







############################## delete jobs

# insert jobs
psql  -d $TESTDATABASE -c "insert into smafejob_deletefile(collection_name, file_id) VALUES('testcollection', 1);"   >> $TESTLOGFILE 2>&1
testResultjdf1=$?
# if job is done, file_id 1 should only be in default collection
psql  -d $TESTDATABASE -c "insert into smafejob_deletefile(priority, file_id) VALUES(2, 2);"   >> $TESTLOGFILE 2>&1
testResultjdf2=$?
# if job is done, file_id 2 should only be in removed collection


# run smafewrap
$SMAFETESTEXECPREFIX/smafewrapd/smafewrapd --no-daemon --id=test --interval -1 -v $SMAFETESTPARAMV --dbconf ../test/resources/localhost-testcasedb-dbconnection.opt  >> $TESTLOGFILE 2>&1
testResultjdf3=$?


### check contents 

# query 1: collection_file
# note: * must be escaped otherwise it is expanded by the shell
SQLQUERY="select collection_id, file_id from collection_file order by collection_id, file_id;"
echo $SQLQUERY | psql  -d $TESTDATABASE  >> $TESTLOGFILE 2>&1
# uncomment the following line to re-generate the reference output. Be CAREFUL
#echo $SQLQUERY | psql  -d $TESTDATABASE  | sed '$d' | sed '$d'  > ../test/resources/$CANONICALTESTNAME/query1.txt 2>&1

# cmp with reference
# - parameter for diff means stdin
# sed command dleetes the last line(s)
tail -n 10 $TESTLOGFILE | sed '$d' | sed '$d' | diff --strip-trailing-cr ../test/resources/$CANONICALTESTNAME/query1.txt - >> $TESTLOGFILE 2>&1
testResultjdf4=$?


# query 2: featurevectorsegment
# note: * must be escaped otherwise it is expanded by the shell
SQLQUERY="select segmentnr, track_id, featurevectortype_id, file_id, startsample, length from featurevectorsegment order by segmentnr, track_id, featurevectortype_id, file_id, startsample, length;"
echo $SQLQUERY | psql  -d $TESTDATABASE  >> $TESTLOGFILE 2>&1
# uncomment the following line to re-generate the reference output. Be CAREFUL
#echo $SQLQUERY | psql  -d $TESTDATABASE  | sed '$d' | sed '$d'  > ../test/resources/$CANONICALTESTNAME/query2.txt 2>&1

# cmp with reference
# - parameter for diff means stdin
# sed command dleetes the last line(s)
tail -n 19 $TESTLOGFILE | sed '$d' | sed '$d' | diff --strip-trailing-cr ../test/resources/$CANONICALTESTNAME/query2.txt - >> $TESTLOGFILE 2>&1
testResultjdf5=$?



################ forth run with addfile jobs

### insert files
# This must be the 2nd file from the first run!
# Save and change IFS
# After some experiments, it seems that the IFS variable must be set to only line break
# - for ls command (storing files in INPATH), AND
# - for loop
OLDIFS=$IFS
IFS=$'\n'
INPATH=($( ls $(pwd)/../test/resources/$CANONICALTESTNAME/testsmall10percent\ faster.mp3 | sort -d))
#for i in  ${INPATH[*]}; do 
#	echo $i
#done
# do the file add
doFileAdd
# Restore it
IFS=$OLDIFS

echo "NOTE: one file (1) should have been processed!"  >> $TESTLOGFILE 2>&1


$SMAFETESTEXECPREFIX/smafewrapd/smafewrapd --no-daemon --id=test --interval -1 -v $SMAFETESTPARAMV --dbconf ../test/resources/localhost-testcasedb-dbconnection.opt  >> $TESTLOGFILE 2>&1
testResult4=$?


# post processing: check if files are moved
MAKECLEAN=no
doFileCleanup
testResult4c=$testResultcheck



# query 4: collection_file
# note: * must be escaped otherwise it is expanded by the shell
SQLQUERY="select collection_id, file_id from collection_file order by collection_id, file_id;"
echo $SQLQUERY | psql  -d $TESTDATABASE  >> $TESTLOGFILE 2>&1
# uncomment the following line to re-generate the reference output. Be CAREFUL
#echo $SQLQUERY | psql  -d $TESTDATABASE  | sed '$d' | sed '$d'  > ../test/resources/$CANONICALTESTNAME/query4.txt 2>&1

# cmp with reference
# - parameter for diff means stdin
# sed command dleetes the last line(s)
tail -n 11 $TESTLOGFILE | sed '$d' | sed '$d' | diff --strip-trailing-cr ../test/resources/$CANONICALTESTNAME/query4.txt - >> $TESTLOGFILE 2>&1
testResult4c1=$?




################ third run


### insert files
# Save and change IFS
# After some experiments, it seems that the IFS variable must be set to only line break
# - for ls command (storing files in INPATH), AND
# - for loop
OLDIFS=$IFS
IFS=$'\n'
INPATH=($( ls $(pwd)/../test/resources/$CANONICALTESTNAME/limit_too_many*.mp3 | sort -d))
#for i in  ${INPATH[*]}; do 
#	echo $i
#done
# do the file add
doFileAdd
# Restore it
IFS=$OLDIFS
echo "NOTE: two files (2) should have been processed!"  >> $TESTLOGFILE 2>&1



$SMAFETESTEXECPREFIX/smafewrapd/smafewrapd --no-daemon --id=test --interval -1 -v $SMAFETESTPARAMV --dbconf ../test/resources/localhost-testcasedb-dbconnection.opt  >> $TESTLOGFILE 2>&1
testResult3=$?


# post processing: do not check anything
MAKECLEAN=yes
doFileCleanup
testResultc3=$testResultcheck

# check if limit reached (error in logfile)
grep "EE File limit of 5 reached" $TESTLOGFILE  >> /dev/null 2>&1 
if [ $? -eq 0 ]; then
	testResultc3b=0
	echo "testResultc3b=0"  >> $TESTLOGFILE 2>&1
else
	testResultc3b=1
	echo "testResultc3b=1"  >> $TESTLOGFILE 2>&1
fi


### check contents : one failed job
echo "select count(*) from smafejob_addfile where status like 'FAILED %';" | psql  -d $TESTDATABASE  >> $TESTLOGFILE 2>&1
# cmp with reference
# - parameter for diff means stdin
# tail -n X: X = lines of reference files + 2
# sed command dleetes the last line(s)
tail -n 5 $TESTLOGFILE | sed '$d' | sed '$d' | diff --strip-trailing-cr ../test/resources/$CANONICALTESTNAME/query3.txt - >> $TESTLOGFILE 2>&1
testResultc3c=$?









# also check if no errors (EE) and no fatal errors (FF) occured
# Note: grep returns 0 if at least one occurence is found, 1 if not a singel one is found, and 2 for errors
grep " File limit" -v  $TESTLOGFILE  | grep "Exception occured" -v | grep " EE "
testErrors1=$?
grep " FF " $TESTLOGFILE
testErrors2=$?


echo "Variable testDaemon1 is now set to $testDaemon1. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testDaemon2 is now set to $testDaemon2.(expected 0)" 			>> $TESTLOGFILE
echo "Variable testResult1 is now set to $testResult1. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testResult2 is now set to $testResult2.(expected 0)" 			>> $TESTLOGFILE
echo "Variable testResult3 is now set to $testResult3.(expected 0)"				>> $TESTLOGFILE
echo "Variable testResultc1 is now set to $testResultc1. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testResultc2 is now set to $testResultc2. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testResultc3 is now set to $testResultc3. (expected 1)" 			>> $TESTLOGFILE
echo "Variable testResultc3b is now set to $testResultc3b. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testResultc3c is now set to $testResultc3c. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testResultjdf1 is now set to $testResultjdf1.(expected 0)"			>> $TESTLOGFILE
echo "Variable testResultjdf2 is now set to $testResultjdf2.(expected 0)"			>> $TESTLOGFILE
echo "Variable testResultjdf3 is now set to $testResultjdf3.(expected 0)"			>> $TESTLOGFILE
echo "Variable testResultjdf4 is now set to $testResultjdf4.(expected 0)"			>> $TESTLOGFILE
echo "Variable testResultjdf5 is now set to $testResultjdf5.(expected 0)"			>> $TESTLOGFILE
echo "Variable testResult4 is now set to $testResult4.(expected 0)"				>> $TESTLOGFILE
echo "Variable testResult4c is now set to $testResult4c.(expected 0)"				>> $TESTLOGFILE
echo "Variable testResult4c1 is now set to $testResult4c1.(expected 0)"				>> $TESTLOGFILE
echo "Variable testErrors1 is now set to $testErrors1.(expected 1)"				>> $TESTLOGFILE
echo "Variable testErrors2 is now set to $testErrors2.(expected 1)"				>> $TESTLOGFILE



# move logfile to final destination
mv $TESTLOGFILE $SMAFETESTEXECPREFIX/


# exit with success / error code
if [ $testDaemon1 == 0 -a $testDaemon2 == 0 -a $testResult1 == 0 -a  $testResult2 == 0  -a  $testResult3 == 0 -a $testResultc1 == 0 -a $testResultc2 == 0 -a $testResultc3 == 1 -a $testResultc3b == 0 -a $testResultc3c == 0 -a $testResultjdf1 == 0 -a $testResultjdf2 == 0 -a $testResultjdf3 == 0 -a $testResultjdf4 == 0 -a $testResultjdf5 == 0 -a $testResult4 == 0 -a $testResult4c == 0 -a $testResult4c1 == 0 -a $testErrors1 == 1 -a $testErrors2 == 1 ] 
then
  echo "$CANONICALTESTNAME completed successfully."
  exit 0 
else 
  echo "There was an error while running test $CANONICALTESTNAME! For details see $SMAFETESTEXECPREFIX/$(basename $TESTLOGFILE)!"
  exit 1
fi 
