#!/bin/bash
# online resources: 
# http://steve-parker.org/sh/sh.shtml
# http://www.freeos.com/guides/lsst/
# http://www.ss64.com/bash/

##########################################################################################
#
#	This script tests the "top k" functionality of smafedistd.
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
#		# smafedistd with topk=5 -d 1 -d 2, *bulk* mode				"first run"
#		# query nearest neighbours of track # 23, distt # 1 and compare with reference result (textfile)
#		# query nearest neighbours of track # 46, distt # 2 and compare with reference result (textfile)
#		# ensure that each track feature vector has at least 5 "from" (outgoing) distance records
#		# add additional data (new fv, file, track, distancejob)
#		# smafedistd with topk=5 -d 1 -d 2							"second run"
#		# check that track #2 has now 6 neighbours, one of which is the new track
#		# check that the new track has 5 neighbours
#		# smafedistd in bulk mode									"third run"
#		# Make sure that error was returned (bulk only if distance table is empty)
#		# (database and users are deleted by a following Cleanup test)
#
#-----------------------------------------------------------------------------------------
#
#	History:
#		2009-06-24		initial version						ep
#		2010-06-22		adapted for db config				ep
#		2010-11-04		encrypted contents					ep
#		2011-02-07		second run, checking regular mode 	ep
#
##########################################################################################


# set name of test for logfile
CANONICALTESTNAME=testDistd_extended
TESTLOGFILE=../test/$CANONICALTESTNAME.log
 
echo "Running test $CANONICALTESTNAME.... "



#check env
. ../test/checkEnv.lib
checkEnv >> $TESTLOGFILE 2>&1

# prepare log file
rm -f $TESTLOGFILE
touch $TESTLOGFILE




# set to user
PGUSERSET=user
setSmafePGUser >> $TESTLOGFILE 2>&1


####------------------------------------------------ BEGIN obsolete
#### config
#OPTFILE=../test/resources/$CANONICALTESTNAME/test.opt
#SQLFILE=$OPTFILE.sql

# run smafeconfig
#$SMAFETESTEXECPREFIX/smafeconfig/smafeconfig $OPTFILE  >> $TESTLOGFILE 2>&1
#$SMAFETESTEXECPREFIX/smafewrapd/smafewrapd --config $OPTFILE  >> $TESTLOGFILE 2>&1
#testResultconfig1=$?
# check existance of file
#if [ -f $SQLFILE ]
#then
#	testResultconfig2=0
#else
#	echo "Output SQL script file does not exist."  >> $TESTLOGFILE 2>&1
#	testResultconfig2=1
#fi

# set to admin
#PGUSERSET=admin
#setSmafePGUser >> $TESTLOGFILE 2>&1

# insert config
#cat $SQLFILE  | psql  -d $TESTDATABASE    >> $TESTLOGFILE 2>&1
#testResultconfig3=$?
# delete sql file
#rm $SQLFILE


# set to user
#PGUSERSET=user
#setSmafePGUser >> $TESTLOGFILE 2>&1

#### ---------------------END obsolete (config stuff iscontained in the contents.sql) 


### insert fv, track, file and distancejob data of 94 song corpus
psql  -d $TESTDATABASE -f ../test/resources/$CANONICALTESTNAME/contents.sql >> $TESTLOGFILE 2>&1
testResult2=$?


 
###  run smafedistd, quit when finished
$SMAFETESTEXECPREFIX/smafedistd/smafedistd --jobs --id=testCaseDaemon --interval="-1" --dbconf ../test/resources/localhost-testcasedb-dbconnection-admin.opt --verbosity=1  --no-daemon --bulk   --initial-run >> $TESTLOGFILE 2>&1
# save exit code
testResult3=$?


### check contents of distance table

# query 1: top 5 tracks for track 38, distt 1
echo "select track_b_id from distance where featurevectortype_id =1 and distancetype_id=1 and track_a_id=38 order by value limit 5" | psql  -d $TESTDATABASE  >> $TESTLOGFILE 2>&1
# cmp with reference
# - parameter for diff means stdin
tail -n 9 $TESTLOGFILE | head -n 7 |diff --strip-trailing-cr ../test/resources/$CANONICALTESTNAME/testDistd_extended_query1_reference.txt - >> $TESTLOGFILE 2>&1
testResult4=$?

# query 2: top 5 tracks for track 89, distt 2
echo "select track_b_id from distance where featurevectortype_id =1 and distancetype_id=2 and track_a_id=89 order by value limit 5" | psql  -d $TESTDATABASE  >> $TESTLOGFILE 2>&1
# cmp with reference
# - parameter for diff means stdin
tail -n 9 $TESTLOGFILE | head -n 7 |diff --strip-trailing-cr ../test/resources/$CANONICALTESTNAME/testDistd_extended_query2_reference.txt - >> $TESTLOGFILE 2>&1
testResult5=$?

# query 3: number of tracks with at least 5 "from" distance records (EACH track must have these!) 
echo "select count(*) as number from (SELECT count(*) as number FROM "public"."distance" WHERE "featurevectortype_id" = '1' AND "distancetype_id" = '2' group by track_a_id having count(*) >=5) as subquery;" | psql  -d $TESTDATABASE  >> $TESTLOGFILE 2>&1
# cmp with reference
# - parameter for diff means stdin
tail -n 5 $TESTLOGFILE | head -n 3 |diff --strip-trailing-cr ../test/resources/$CANONICALTESTNAME/testDistd_extended_query3_reference.txt - >> $TESTLOGFILE 2>&1
testResult6=$?


#############################################################
## second run

echo "################################## second run ############################" >> $TESTLOGFILE 2>&1

### insert more fv, track, file and distancejob data
psql  -d $TESTDATABASE -f ../test/resources/$CANONICALTESTNAME/more_contents.sql >> $TESTLOGFILE 2>&1
testResult2nd2=$?


 
###  run smafedistd, quit when finished
$SMAFETESTEXECPREFIX/smafedistd/smafedistd --jobs --id=testCaseDaemon --interval="-1" --dbconf ../test/resources/localhost-testcasedb-dbconnection.opt --verbosity=1  --no-daemon  >> $TESTLOGFILE 2>&1
# save exit code
testResult2nd3=$?

### check contents of distance table

# query 1: top 6 tracks for track 2, distt 1
echo "select track_b_id from distance where featurevectortype_id =1 and distancetype_id=1 and track_a_id=2 order by value limit 6" | psql  -d $TESTDATABASE  >> $TESTLOGFILE 2>&1
# cmp with reference
# - parameter for diff means stdin
tail -n 10 $TESTLOGFILE | head -n 8 |diff --strip-trailing-cr ../test/resources/$CANONICALTESTNAME/testDistd_extended_query4_reference.txt - >> $TESTLOGFILE 2>&1
testResult2nd4=$?

# query 2: top 6 tracks for track 96, distt 2
# but we expect only 5 rows in the result
echo "select track_b_id from distance where featurevectortype_id =1 and distancetype_id=2 and track_a_id=96 order by value limit 6" | psql  -d $TESTDATABASE  >> $TESTLOGFILE 2>&1
# cmp with reference
# - parameter for diff means stdin
tail -n 9 $TESTLOGFILE | head -n 7 |diff --strip-trailing-cr ../test/resources/$CANONICALTESTNAME/testDistd_extended_query5_reference.txt - >> $TESTLOGFILE 2>&1
testResult2nd5=$?


######################## end of second run






#############################################################
## 3rd run

echo "################################## third run ############################" >> $TESTLOGFILE 2>&1

###  run smafedistd, quit when finished
# with bulk, but since distance table is not empty, it should exit with an error.
$SMAFETESTEXECPREFIX/smafedistd/smafedistd --jobs --id=testCaseDaemon --interval="-1" --dbconf ../test/resources/localhost-testcasedb-dbconnection.opt --verbosity=1  --bulk --no-daemon  >> $TESTLOGFILE 2>&1
# save exit code
testResult3rd3=$?
echo NOTE: This fatal error was provoked DELIBERATELY.     >> $TESTLOGFILE 2>&1
# we expect error here



######################## end of 3rd run






# drop testdb
# set to admin
#PGUSERSET=admin
#setSmafePGUser >> $TESTLOGFILE 2>&1
#dropdb  $TESTDATABASE  >> $TESTLOGFILE 2>&1
#testResult7=$?

# delete test roles
#psql  postgres -c "drop role testsmafeadmin;"  >> $TESTLOGFILE 2>&1
#psql  postgres -c "drop role testsmurf;" >> $TESTLOGFILE 2>&1




# also check if no warnings,  errors (EE) and no fatal errors (FF) occured
# Note: grep returns 0 if at least one occurence is found, 1 if not a singel one is found, and 2 for errors
grep " WW " $TESTLOGFILE
testErrors0=$?
grep " EE " $TESTLOGFILE
testErrors1=$?
# We have one deliberate error
grep "Bulk modes expects distance" -v  $TESTLOGFILE  | grep " FF " 
testErrors2=$?


echo -------------------------------------------------------------------------------------- >> $TESTLOGFILE
#echo "Variable testResultconfig1 is now set to $testResultconfig1. (expected 0)" 			>> $TESTLOGFILE
#echo "Variable testResultconfig2 is now set to $testResultconfig2. (expected 0)" 			>> $TESTLOGFILE
#echo "Variable testResultconfig3 is now set to $testResultconfig3. (expected 0)" 			>> $TESTLOGFILE
#echo "Variable testResult1 is now set to $testResult1. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testResult2 is now set to $testResult2. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testResult3 is now set to $testResult3. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testResult4 is now set to $testResult4. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testResult5 is now set to $testResult5. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testResult2nd2 is now set to $testResult2nd2. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testResult2nd3 is now set to $testResult2nd3. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testResult2nd4 is now set to $testResult2nd4. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testResult2nd5 is now set to $testResult2nd5. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testResult3rd3 is now set to $testResult3rd3. (expected != 0)" 			>> $TESTLOGFILE
echo "Variable testResult6 is now set to $testResult6. (expected 0)" 			>> $TESTLOGFILE
#echo "Variable testResult7 is now set to $testResult7. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testErrors0 is now set to $testErrors0.(expected 1)"				>> $TESTLOGFILE
echo "Variable testErrors1 is now set to $testErrors1.(expected 1)"				>> $TESTLOGFILE
echo "Variable testErrors2 is now set to $testErrors2.(expected 1)"				>> $TESTLOGFILE



# move logfile to final destination
mv $TESTLOGFILE $SMAFETESTEXECPREFIX/


# exit with success / error code
if [ $testResult2 == 0 -a $testResult3 == 0 -a $testResult4 == 0 -a $testResult5 == 0 -a $testResult2nd2 == 0 -a $testResult2nd3 == 0 -a $testResult2nd4 == 0 -a $testResult2nd5 == 0 -a $testResult3rd3 != 0 -a $testResult6	 == 0  -a $testErrors0 == 1 -a $testErrors1 == 1 -a $testErrors2 == 1 ] 
then
  echo "$CANONICALTESTNAME complete"
  exit 0 
else 
  echo "There was an error while running the $CANONICALTESTNAME test! For details see $SMAFETESTEXECPREFIX/$(basename $TESTLOGFILE) "
  exit 1
fi 
