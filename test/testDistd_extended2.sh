#!/bin/bash
# online resources: 
# http://steve-parker.org/sh/sh.shtml
# http://www.freeos.com/guides/lsst/
# http://www.ss64.com/bash/

##########################################################################################
#
#	This script tests the transition from bulk to regular mode of smafedistd.
#
#-----------------------------------------------------------------------------------------
#
#	Actions:
#		# expects empty database with official db dump
#		# inserts config sql file into db
#		# inserts data (94 song corpus):
#			# featurevectortype RH
#			# tracks
#			# files
#			# feature vectors
#			# distance jobs for all distance types (1 to 5)
#		# smafedistd with topk=5 -d 1 -d 2, *bulk* mode as daemon				"first run"
#		# add additional data (new fv, file, track, distancejob)
#		# wait until daemon is finished, merge logfiles
#		# query nearest neighbours of track # 23, distt # 1 and compare with reference result (textfile)
#		# query nearest neighbours of track # 46, distt # 2 and compare with reference result (textfile)
#		# ensure that each track feature vector has at least 5 "from" (outgoing) distance records
#		# check that track #2 has now 6 neighbours, one of which is the new track
#		# check that the new track has 5 neighbours
#		# check that the transition from bulk mode to regular mode took place when encountering te later
#			inserted track (grep in log file)
#		# (database and users are deleted by a following Cleanup test)



#
#-----------------------------------------------------------------------------------------
#
#	History:
#		2011-02-14		inital version, as copy of testDistd_extended 	ep
#
##########################################################################################


# set name of test for logfile
CANONICALTESTNAME=testDistd_extended2
RESOURCETESTNAME=testDistd_extended
TESTLOGFILE=../test/$CANONICALTESTNAME.log
 
echo "Running test $CANONICALTESTNAME.... "



#check env
. ../test/checkEnv.lib
checkEnv >> $TESTLOGFILE 2>&1

# prepare log file
rm -f $TESTLOGFILE
touch $TESTLOGFILE





# save timestamp
SMAFETIC=$(date +'%s')


# set to user
PGUSERSET=user
setSmafePGUser >> $TESTLOGFILE 2>&1



### insert fv, track, file and distancejob data of 94 song corpus
psql  -d $TESTDATABASE -f ../test/resources/$RESOURCETESTNAME/contents.sql >> $TESTLOGFILE 2>&1
testResult1=$?



# save elapsed time for the data inserted.
# This magically corresponds ot the sleeping duration after daemon starts before the additional data should be inserted.
SMAFETOC=$(date +'%s')

MEASURED_WAIT_ADD_DATA=$(($SMAFETOC - $SMAFETIC))
DERIVED_WAIT_ADD_DATA=$(($SMAFETOC - $SMAFETIC - 1))
echo Elapsed time measured: $MEASURED_WAIT_ADD_DATA >>  $TESTLOGFILE 2>&1

 
###  run smafedistd
$SMAFETESTEXECPREFIX/smafedistd/smafedistd --jobs --id=id$CANONICALTESTNAME --log testsmafedistd.log --interval 66 --dbconf ../test/resources/localhost-testcasedb-dbconnection-admin.opt --verbosity=1   --bulk   --initial-run >> $TESTLOGFILE 2>&1
# save exit code
testResult2=$?


# daemon is running now

# wait a little but before entering the data
# this should be long enough to have the daemon loaded the first 94 feature vectors
# but not too long such that the daemon goes to sleep mode before encountering the job with track_id 96

# Instruction how to adapt that sleep period
# 1) The timestamp T0 of line
# 			Die Mär 8 14:34:32 CET 2011:Additional data inserted into db.
#	must be between the timestamps T1 and T2 of these two lines:
# 			2011-03-08 14:34:27 dd Executing SQL: select * from distance_indexes_exist();
#	and
#			2011-03-08 14:34:58 ii Finished task: track_id=94, featurevectortype_id=1, distancetype_id=1 in 0 s (usr time 40 ms)
# If T0 is before T1: 	the daemon will load basic+additional data, no break between bulk and regular mode
#						-> increase WAIT_ADD_DATA
# If T0 is after T2:	daemon will enter sleep mode, without using the additional data
#						-> decrease WAIT_ADD_DATA
WAIT_ADD_DATA=$DERIVED_WAIT_ADD_DATA
# On EP dev machine with tunnel to kreta, 4 or 5 was good
echo -n Starting sleep for $WAIT_ADD_DATA seconds... >>  $TESTLOGFILE 2>&1
sleep $WAIT_ADD_DATA
echo finished. 										>>  $TESTLOGFILE 2>&1

### insert more fv, track, file and distancejob data
psql  -d $TESTDATABASE -f ../test/resources/$RESOURCETESTNAME/more_contents.sql >> $TESTLOGFILE 2>&1
testResult3=$?
echo -n $(date):  >> $TESTLOGFILE 2>&1
echo Additional data inserted into db.  >> $TESTLOGFILE 2>&1





# wait until daemon is in sleeping mode

# check if daemon is running (same line is also used below!)
testDaemonrunning=$(ps aux |grep id$CANONICALTESTNAME  |grep -v grep  |  wc -l)
# contains number of lines with that id. 
if [ $testDaemonrunning -eq 0 ]; then
	ps aux |grep id$CANONICALTESTNAME |grep -v grep >> $TESTLOGFILE 2>&1 
	echo "WARNING: Daemon seems not be running" >> $TESTLOGFILE 2>&1 
	testResult3=1
fi

DAEMONSLEEPING=0
SLEEPFORSECS=15
COUNTER=0
SLEEPLOOPS=15
until [ $DAEMONSLEEPING -eq 1 -o $COUNTER -ge $SLEEPLOOPS ]; do
	# sleep for X seconds at let the daemon do its work
	echo "Sleeping for $SLEEPFORSECS seconds..." >> $TESTLOGFILE 2>&1 
	sleep $SLEEPFORSECS
	let COUNTER=COUNTER+1
	
	# check if daemon is in sleep mode
	# that is the case if we find this line in the log file
	# interval was 66!
	grep "Going to sleep for 66 minutes..." testsmafedistd.log  >> $TESTLOGFILE 2>&1 
	if [ $? -eq 0 ]; then
		DAEMONSLEEPING=1
	else
		DAEMONSLEEPING=0
	fi
done
if [ $DAEMONSLEEPING -eq 0 -a $COUNTER -ge $SLEEPLOOPS ]; then
	ps aux |grep id$CANONICALTESTNAME |grep -v grep  >> $TESTLOGFILE 2>&1 
	echo "Waited $(($COUNTER*$SLEEPFORSECS)) seconds, but daemon still does not seem to be in sleeping mode." >> $TESTLOGFILE 2>&1 
	testDaemon1=1
else
	# get PID
	DamonPID=$(grep "PID of" testsmafedistd.log | head -n 1 |cut -d " " -f 10)
	# kill the daemon
	kill $DamonPID
	# check if killed
	sleep 1
	testDaemonrunning=$(ps aux |grep id$CANONICALTESTNAME |grep -v grep | wc -l) >> $TESTLOGFILE 2>&1 
	# contains number of lines with that id.
	if [ $testDaemonrunning -ge 1 ]; then
		ps aux |grep id$CANONICALTESTNAME |grep -v grep >> $TESTLOGFILE 2>&1 
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
cat testsmafedistd.log >> $TESTLOGFILE
echo "-------------------END log from daemon---------------------"  >> $TESTLOGFILE 2>&1 
# remove log file
rm -f testsmafedistd.log






### check contents of distance table

# query 1: top 5 tracks for track 38, distt 1
echo "select track_b_id from distance where featurevectortype_id =1 and distancetype_id=1 and track_a_id=38 order by value limit 5" | psql  -d $TESTDATABASE  >> $TESTLOGFILE 2>&1
# cmp with reference
# - parameter for diff means stdin
tail -n 9 $TESTLOGFILE | head -n 7 |diff --strip-trailing-cr ../test/resources/$RESOURCETESTNAME/testDistd_extended_query1_reference.txt - >> $TESTLOGFILE 2>&1
testResultQuery1=$?

# query 2: top 5 tracks for track 89, distt 2
echo "select track_b_id from distance where featurevectortype_id =1 and distancetype_id=2 and track_a_id=89 order by value limit 5" | psql  -d $TESTDATABASE  >> $TESTLOGFILE 2>&1
# cmp with reference
# - parameter for diff means stdin
tail -n 9 $TESTLOGFILE | head -n 7 |diff --strip-trailing-cr ../test/resources/$RESOURCETESTNAME/testDistd_extended_query2_reference.txt - >> $TESTLOGFILE 2>&1
testResultQuery2=$?

# query 3: number of tracks with at least 5 "from" distance records (EACH track must have these!) , and now we have 95 tracks
echo "select count(*) as number from (SELECT count(*) as number FROM "public"."distance" WHERE "featurevectortype_id" = '1' AND "distancetype_id" = '2' group by track_a_id having count(*) >=5) as subquery;" | psql  -d $TESTDATABASE  >> $TESTLOGFILE 2>&1
# cmp with reference
# - parameter for diff means stdin
tail -n 5 $TESTLOGFILE | head -n 3 |diff --strip-trailing-cr ../test/resources/$RESOURCETESTNAME/testDistd_extended_query3b_reference.txt - >> $TESTLOGFILE 2>&1
testResultQuery3=$?


### check contents of distance table

# query 1: top 6 tracks for track 2, distt 1
echo "select track_b_id from distance where featurevectortype_id =1 and distancetype_id=1 and track_a_id=2 order by value limit 6" | psql  -d $TESTDATABASE  >> $TESTLOGFILE 2>&1
# cmp with reference
# - parameter for diff means stdin
tail -n 10 $TESTLOGFILE | head -n 8 |diff --strip-trailing-cr ../test/resources/$RESOURCETESTNAME/testDistd_extended_query4_reference.txt - >> $TESTLOGFILE 2>&1
testResultQuery4=$?

# query 2: top 6 tracks for track 96, distt 2
# but we expect only 5 rows in the result
echo "select track_b_id from distance where featurevectortype_id =1 and distancetype_id=2 and track_a_id=96 order by value limit 6" | psql  -d $TESTDATABASE  >> $TESTLOGFILE 2>&1
# cmp with reference
# - parameter for diff means stdin
tail -n 9 $TESTLOGFILE | head -n 7 |diff --strip-trailing-cr ../test/resources/$RESOURCETESTNAME/testDistd_extended_query5_reference.txt - >> $TESTLOGFILE 2>&1
testResultQuery5=$?



### check that bulk mode was disabled
# Note: grep returns 0 if at least one occurence is found, 1 if not a singel one is found, and 2 for errors
grep "Newly added tracks encountered. Stopping bulk operation and switching to regular mode." $TESTLOGFILE  >> /dev/null 2>&1
testLog1=$?
echo "testLog1=$testLog1" >> $TESTLOGFILE


grep "Bulk mode, but current track_id 96 not found in vTracks." $TESTLOGFILE  >> /dev/null 2>&1
testLog2=$?
echo "testLog2=$testLog2" >> $TESTLOGFILE






# also check if no warnings,  errors (EE) and no fatal errors (FF) occured
# Note: grep returns 0 if at least one occurence is found, 1 if not a singel one is found, and 2 for errors
grep " WW " $TESTLOGFILE
testErrors0=$?
grep " EE " $TESTLOGFILE
testErrors1=$?
grep " FF " $TESTLOGFILE 
testErrors2=$?


echo -------------------------------------------------------------------------------------- >> $TESTLOGFILE
echo "Variable testResult1 is now set to $testResult1. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testResult2 is now set to $testResult2. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testResult3 is now set to $testResult3. (expected 0)" 			>> $TESTLOGFILE
#echo "Variable testResult4 is now set to $testResult4. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testDaemon1 is now set to $testDaemon1. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testDaemon2 is now set to $testDaemon2. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testResultQuery1 is now set to $testResultQuery1. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testResultQuery2 is now set to $testResultQuery2. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testResultQuery3 is now set to $testResultQuery3. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testResultQuery4 is now set to $testResultQuery4. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testResultQuery5 is now set to $testResultQuery5. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testLog1 is now set to $testLog1. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testLog2 is now set to $testLog2. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testErrors0 is now set to $testErrors0.(expected 1)"				>> $TESTLOGFILE
echo "Variable testErrors1 is now set to $testErrors1.(expected 1)"				>> $TESTLOGFILE
echo "Variable testErrors2 is now set to $testErrors2.(expected 1)"				>> $TESTLOGFILE



# move logfile to final destination
mv $TESTLOGFILE $SMAFETESTEXECPREFIX/


# exit with success / error code
if [ $testResult1 == 0 -a $testResult2 == 0 -a $testResult3 == 0 -a $testResultQuery1 == 0 -a $testResultQuery2 == 0 -a $testResultQuery3 == 0 -a $testResultQuery4 == 0 -a $testResultQuery5 == 0 -a $testLog1 == 0 -a $testLog2 == 0 -a $testErrors0 == 1 -a $testErrors1 == 1 -a $testErrors2 == 1 ] 
then
  echo "$CANONICALTESTNAME complete"
  exit 0 
else 
  echo "There was an error while running the $CANONICALTESTNAME test! For details see $SMAFETESTEXECPREFIX/$(basename $TESTLOGFILE) "
  exit 1
fi 
