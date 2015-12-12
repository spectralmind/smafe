#!/bin/bash
# online resources: 
# http://steve-parker.org/sh/sh.shtml
# http://www.freeos.com/guides/lsst/
# http://www.ss64.com/bash/


##########################################################################################
#
#	This script tests smuiupdated, if the sql for the schema can be found. Otherwise warning (no error) is returned 
#
#-----------------------------------------------------------------------------------------
#
#	Actions:
#
#		# expects empty database with official smafestore dump
#		# smui sql dump & some bootstrap data
#		# also a generated config sql is inserted
#		# start smuiupdated in no-daemon mode 
#			- processes jobs
#			- check if relevant table contents match reference files 
#		# add delete track job to db (sql file)
#		# start smuiupdated in no-daemon mode
#			- processed jobs
#			- check if database contents matches
#		# start smafewrap with one mp3 file
#			- check if smui add track job has been propagated.
#		# check for errors in log file
#		# (database and users are deleted by a following Cleanup test)
#
#-----------------------------------------------------------------------------------------
#
#	Note:
#		Hardcoded:
#		- password in config must be supersecret because the bootstrap data contains
#			encrypted fvtype and fv
#
##########################################################################################

# set name of test for logfile
CANONICALTESTNAME=testSmuiupdated
TESTLOGFILE=../test/$CANONICALTESTNAME.log

echo "Running test $CANONICALTESTNAME.... "

# check if this test should run actually
# check if env var has been set
if [ -z $SMUIDBSCHEMAFILE ]; then
    #try default file name
    export SMUIDBSCHEMAFILE=db-schema-delta.sql
fi
echo "Using smui sql schema file " $SMUIDBSCHEMAFILE
if [ ! -f  $SMUIDBSCHEMAFILE ]; then
   # explain stuff
   echo WARNING
   echo This test is skipped because the required sql schema file for Smui could not be found.
   echo To specify the location and name you can use the environmen variable SMUIDBSCHEMAFILE.
   echo Example: 
   echo export SMUIDBSCHEMAFILE=/path/to/db-schema-delta.sql
   exit 0;
fi

#check env
. ../test/checkEnv.lib

# prepare log file
rm -f $TESTLOGFILE
touch $TESTLOGFILE

checkEnv >> $TESTLOGFILE 2>&1

# set to user
PGUSERSET=admin
setSmafePGUser >> $TESTLOGFILE 2>&1



echo "Setting up smui db schema"
# apply the smui sql file
cat $SMUIDBSCHEMAFILE | psql  $TESTDATABASE >> $TESTLOGFILE 2>&1
testResultj1=$?


#### config
echo "Setting up config table"
OPTFILE=../test/resources/$CANONICALTESTNAME/test.opt
SQLFILE=$OPTFILE.sql
# run smafeconfig
#$SMAFETESTEXECPREFIX/smafeconfig/smafeconfig  $OPTFILE  >> $TESTLOGFILE 2>&1
$SMAFETESTEXECPREFIX/smafewrapd/smafewrapd --config $OPTFILE  >> $TESTLOGFILE 2>&1
testResultconfig1=$?
# check existance of file
if [ -f $SQLFILE ]
then
	testResultconfig2=0
else
	echo "Output SQL script file does not exist."  >> $TESTLOGFILE 2>&1
	testResultconfig2=1
fi
# insert config
cat $SQLFILE  | psql  -d $TESTDATABASE    >> $TESTLOGFILE 2>&1
testResultconfig3=$?
# delete sql file
rm $SQLFILE

# apply the test settings sql file
echo "Setting up test db schema"
cat ../test/resources/$CANONICALTESTNAME/$CANONICALTESTNAME.sql | psql  $TESTDATABASE >> $TESTLOGFILE 2>&1
testResultj2=$?




#### process jobs
# start not as daemon, should not stay in mem, 
# process one job
$SMAFETESTEXECPREFIX/smuiupdated/smuiupdated --id test  -v $SMAFETESTPARAMV -f 1 --no-daemon --interval -1  --dbconf ../test/resources/localhost-testcasedb-dbconnection.opt >> $TESTLOGFILE 2>&1
testResultj3=$?


### check contents 

# general
# cmp with reference
# - parameter for diff means stdin
# sed command dleetes the last line(s)
# tail -n N where N = (lines of result + 2)

# query 1: track table
echo "select bubbles1_id, bubbles2_id, ST_X(geom), ST_Y(geom) from track where id=1" | psql  -d $TESTDATABASE  >> $TESTLOGFILE 2>&1
tail -n 5 $TESTLOGFILE | sed '$d' | sed '$d' | diff --strip-trailing-cr ../test/resources/$CANONICALTESTNAME/$CANONICALTESTNAME.reference1.txt - >> $TESTLOGFILE 2>&1
testResultac1=$?

# query 2: job OK?
echo "select status, finished is not null as finished_is_not_null from smuijob_addtrack where id=1;" | psql  -d $TESTDATABASE  >> $TESTLOGFILE 2>&1
tail -n 5 $TESTLOGFILE | sed '$d' | sed '$d' | diff --strip-trailing-cr ../test/resources/$CANONICALTESTNAME/$CANONICALTESTNAME.reference2.txt - >> $TESTLOGFILE 2>&1
testResultac2=$?

# query 3: bubbles1
echo "select id, size, count from bubbles1 order by id;" | psql  -d $TESTDATABASE  >> $TESTLOGFILE 2>&1
tail -n 9 $TESTLOGFILE | sed '$d' | sed '$d' | diff --strip-trailing-cr ../test/resources/$CANONICALTESTNAME/$CANONICALTESTNAME.reference3.txt - >> $TESTLOGFILE 2>&1
testResultac3=$?

# query 4: bubbles2
echo "select id, size, count from bubbles2 order by id;" | psql  -d $TESTDATABASE  >> $TESTLOGFILE 2>&1
tail -n 6 $TESTLOGFILE | sed '$d' | sed '$d' | diff --strip-trailing-cr ../test/resources/$CANONICALTESTNAME/$CANONICALTESTNAME.reference4.txt - >> $TESTLOGFILE 2>&1
testResultac4=$?



### Delete jobs new (having it propagated through smafewrapd)
# apply the test settings sql file
echo "Setting up test db schema"
cat ../test/resources/$CANONICALTESTNAME/${CANONICALTESTNAME}3.sql | psql  $TESTDATABASE >> $TESTLOGFILE 2>&1
testResultd1=$?

# call smafewrap
$SMAFETESTEXECPREFIX/smafewrapd/smafewrapd --no-daemon --id=test --interval -1 -v $SMAFETESTPARAMV --dbconf ../test/resources/localhost-testcasedb-dbconnection.opt   >> $TESTLOGFILE 2>&1
testResultd2=$?

## one file (id 1) should be deleted now (moved from default collection to removed).
# Make sure that no smuideletetrack job is here
# query 10: job not there
echo "select status, finished is not null as finished_is_not_null from smuijob_deletetrack;" | psql  -d $TESTDATABASE  >> $TESTLOGFILE 2>&1
tail -n 4 $TESTLOGFILE | sed '$d' | sed '$d' | diff --strip-trailing-cr ../test/resources/$CANONICALTESTNAME/$CANONICALTESTNAME.reference10.txt - >> $TESTLOGFILE 2>&1
testResultd3=$?



# add second delete file job
cat ../test/resources/$CANONICALTESTNAME/${CANONICALTESTNAME}4.sql | psql  $TESTDATABASE >> $TESTLOGFILE 2>&1
testResultd4=$?

# call smafewrap
$SMAFETESTEXECPREFIX/smafewrapd/smafewrapd --no-daemon --id=test --interval -1 -v $SMAFETESTPARAMV --dbconf ../test/resources/localhost-testcasedb-dbconnection.opt >> $TESTLOGFILE 2>&1
testResultd5=$?

## both files should be deleted now (moved from default collection to removed).
# Make sure that ONE smuideletetrack job is here
# query 11: job is  there
echo "select status, finished is not null as finished_is_not_null from smuijob_deletetrack;" | psql  -d $TESTDATABASE  >> $TESTLOGFILE 2>&1
tail -n 5 $TESTLOGFILE | sed '$d' | sed '$d' | diff --strip-trailing-cr ../test/resources/$CANONICALTESTNAME/$CANONICALTESTNAME.reference11.txt - >> $TESTLOGFILE 2>&1
testResultd6=$?

# process jobs (one delete job)
$SMAFETESTEXECPREFIX/smuiupdated/smuiupdated --id test  -v $SMAFETESTPARAMV -f 1 --no-daemon --interval -1  --dbconf ../test/resources/localhost-testcasedb-dbconnection.opt >> $TESTLOGFILE 2>&1
testResultd7=$?

### check

# query 6: track table
echo "select id, bubbles1_id, bubbles2_id, ST_X(geom), ST_Y(geom) from track where id=1" | psql  -d $TESTDATABASE  >> $TESTLOGFILE 2>&1
tail -n 5 $TESTLOGFILE | sed '$d' | sed '$d' | diff --strip-trailing-cr ../test/resources/$CANONICALTESTNAME/$CANONICALTESTNAME.reference6.txt - >> $TESTLOGFILE 2>&1
testResultd8=$?

# query 7: job OK?
echo "select status, finished is not null as finished_is_not_null from smuijob_deletetrack where id=1;" | psql  -d $TESTDATABASE  >> $TESTLOGFILE 2>&1
tail -n 5 $TESTLOGFILE | sed '$d' | sed '$d' | diff --strip-trailing-cr ../test/resources/$CANONICALTESTNAME/$CANONICALTESTNAME.reference7.txt - >> $TESTLOGFILE 2>&1
testResultd9=$?

# query 8: bubbles1
echo "select id, size, count from bubbles1 order by id;" | psql  -d $TESTDATABASE  >> $TESTLOGFILE 2>&1
tail -n 9 $TESTLOGFILE | sed '$d' | sed '$d' | diff --strip-trailing-cr ../test/resources/$CANONICALTESTNAME/$CANONICALTESTNAME.reference8.txt - >> $TESTLOGFILE 2>&1
testResultd10=$?

# query 9: bubbles2
echo "select id, size, count from bubbles2 order by id;" | psql  -d $TESTDATABASE  >> $TESTLOGFILE 2>&1
tail -n 6 $TESTLOGFILE | sed '$d' | sed '$d' | diff --strip-trailing-cr ../test/resources/$CANONICALTESTNAME/$CANONICALTESTNAME.reference9.txt - >> $TESTLOGFILE 2>&1
testResultd11=$?


# insert add job
TESTWRAPFILE=../test/resources/testsmall.mp3
# this is advanced bash magic: basically it extracts the basename of a path+file expression
TESTWRAPFILE_NOPATH=${TESTWRAPFILE##*/}
TESTWRAPFILE_CP=${TESTWRAPFILE}.fortesting.mp3
# copy file so that it can be deleted later
cp ${TESTWRAPFILE} ${TESTWRAPFILE_CP} >> $TESTLOGFILE 2>&1
psql  -d $TESTDATABASE -c "insert into smafejob_addfile (collection_name, file_uri, external_key) VALUES('testcollection', '${TESTWRAPFILE_CP}', NULL);"   >> $TESTLOGFILE 2>&1


### start smafewrapd and check for propagation
$SMAFETESTEXECPREFIX/smafewrapd/smafewrapd --no-daemon --id=test --interval -1 -v $SMAFETESTPARAMV --dbconf ../test/resources/localhost-testcasedb-dbconnection.opt  >> $TESTLOGFILE 2>&1
testResultac5=$?


# query 5: smuijob_addtrack
echo "select id, priority, track_id from smuijob_addtrack where status is  null;" | psql  -d $TESTDATABASE  >> $TESTLOGFILE 2>&1
tail -n 5 $TESTLOGFILE | sed '$d' | sed '$d' | diff --strip-trailing-cr ../test/resources/$CANONICALTESTNAME/$CANONICALTESTNAME.reference5.txt - >> $TESTLOGFILE 2>&1
testResultac6=$?




# also check if no errors (EE) and no fatal errors (FF) occured
# Note: grep returns 0 if at least one occurence is found, 1 if not a singel one is found, and 2 for errors
grep " EE " $TESTLOGFILE
testErrors1=$?
grep " FF " $TESTLOGFILE
testErrors2=$?


# drop testdb
#PGUSERSET=admin
#setSmafePGUser >> $TESTLOGFILE 2>&1
#dropdb  $TESTDATABASE  >> $TESTLOGFILE 2>&1
#testResultje=$?
#testResultje=0


echo "Variable testResultconfig1 is now set to $testResultconfig1. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testResultconfig2 is now set to $testResultconfig2. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testResultconfig3 is now set to $testResultconfig3. (expected 0)" 			>> $TESTLOGFILE
echo "Variable testResultj1 is now set to $testResultj1.(expected 0)"			>> $TESTLOGFILE
echo "Variable testResultj2 is now set to $testResultj2.(expected 0)"			>> $TESTLOGFILE
echo "Variable testResultj3 is now set to $testResultj3.(expected 0)"			>> $TESTLOGFILE
#echo "Variable testResultj4 is now set to $testResultj4.(expected 0)"			>> $TESTLOGFILE
#echo "Variable testResultj5 is now set to $testResultj5.(expected 0)"			>> $TESTLOGFILE
echo "Variable testResultac1 is now set to $testResultac1.(expected 0)"			>> $TESTLOGFILE
echo "Variable testResultac2 is now set to $testResultac2.(expected 0)"			>> $TESTLOGFILE
echo "Variable testResultac3 is now set to $testResultac3.(expected 0)"			>> $TESTLOGFILE
echo "Variable testResultac4 is now set to $testResultac4.(expected 0)"			>> $TESTLOGFILE
echo "Variable testResultac5 is now set to $testResultac5.(expected 0)"			>> $TESTLOGFILE
echo "Variable testResultac6 is now set to $testResultac6.(expected 0)"			>> $TESTLOGFILE
echo "Variable testResultd1 is now set to $testResultd1.(expected 0)"			>> $TESTLOGFILE
echo "Variable testResultd2 is now set to $testResultd2.(expected 0)"			>> $TESTLOGFILE
echo "Variable testResultd3 is now set to $testResultd3.(expected 0)"			>> $TESTLOGFILE
echo "Variable testResultd4 is now set to $testResultd4.(expected 0)"			>> $TESTLOGFILE
echo "Variable testResultd5 is now set to $testResultd5.(expected 0)"			>> $TESTLOGFILE
echo "Variable testResultd6 is now set to $testResultd6.(expected 0)"			>> $TESTLOGFILE
echo "Variable testResultd7 is now set to $testResultd7.(expected 0)"			>> $TESTLOGFILE
echo "Variable testResultd8 is now set to $testResultd8.(expected 0)"			>> $TESTLOGFILE
echo "Variable testResultd9 is now set to $testResultd9.(expected 0)"			>> $TESTLOGFILE
echo "Variable testResultd10 is now set to $testResultd10.(expected 0)"			>> $TESTLOGFILE
echo "Variable testResultd11 is now set to $testResultd11.(expected 0)"			>> $TESTLOGFILE


#echo "Variable testResultjdf1 is now set to $testResultjdf1.(expected 0)"			>> $TESTLOGFILE
#echo "Variable testResultjdf2 is now set to $testResultjdf2.(expected 0)"			>> $TESTLOGFILE
#echo "Variable testResultjdf3 is now set to $testResultjdf3.(expected 0)"			>> $TESTLOGFILE
#echo "Variable testResultjdf4 is now set to $testResultjdf4.(expected 0)"			>> $TESTLOGFILE
#echo "Variable testResultjdf5 is now set to $testResultjdf5.(expected 0)"			>> $TESTLOGFILE
#echo "Variable testResultjdf6 is now set to $testResultjdf6.(expected 0)"			>> $TESTLOGFILE
#echo "Variable testResultje is now set to $testResultje.(expected 0)"			>> $TESTLOGFILE
echo "Variable testErrors1 is now set to $testErrors1.(expected 1)"				>> $TESTLOGFILE
echo "Variable testErrors2 is now set to $testErrors2.(expected 1)"				>> $TESTLOGFILE


# move logfile to final destination
mv $TESTLOGFILE $SMAFETESTEXECPREFIX/

# exit with success / error code
if [ $testResultconfig1 == 0 -a $testResultconfig2 == 0 -a $testResultconfig3 == 0 -a  $testResultj1 == 0 -a $testResultj2 == 0 -a $testResultj3 == 0 -a $testResultac1 == 0 -a $testResultac2 == 0 -a $testResultac3 == 0 -a $testResultac4 == 0 -a $testResultac5 == 0 -a $testResultac6 == 0 -a $testResultd1 == 0 -a $testResultd2 == 0 -a $testResultd3 == 0 -a $testResultd4 == 0 -a $testResultd5 == 0 -a $testResultd6 == 0 -a $testResultd7 == 0 -a $testResultd8 == 0 -a $testResultd9 == 0 -a $testResultd10 == 0 -a $testResultd11 == 0 -a $testErrors1 == 1 -a $testErrors2 == 1 ] 
then
  echo "$CANONICALTESTNAME completed successfully."
  exit 0 
else 
  echo "There was an error while running the $CANONICALTESTNAME test! For details see $SMAFETESTEXECPREFIX/$(basename $TESTLOGFILE)"
  exit 1
fi 
