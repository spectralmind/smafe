#!/bin/bash
# this test will try to create a database for smafe 
echo "Testing database creation .... "
#echo "To change postgres user / password use the environment settings: export PGUSER= ; export PGPASSWORD= "

# online resources: 
# http://steve-parker.org/sh/sh.shtml
# http://www.freeos.com/guides/lsst/
# http://www.ss64.com/bash/


# set name of test for logfile
CANONICALTESTNAME=testCreateDB
TESTLOGFILE=../test/$CANONICALTESTNAME.log
 
echo "Running test $CANONICALTESTNAME.... "
echo "Note that this script uses PGUSER and PGPASSWORD environment variables"
echo "Please make sure that those are set to a postgres role that is allowed to create databases"


#check env
. ../test/checkEnv.lib

# prepare log file
rm -f $TESTLOGFILE
touch $TESTLOGFILE




# save PG stuff - this has to be in the first test shell script
#savePGEnv >> $TESTLOGFILE 2>&1 


dberror=0

echo "using postgres user: " $PGUSER >> $TESTLOGFILE 2>&1 
echo "using postgres pwd:  " $PGPASSWORD >> $TESTLOGFILE 2>&1
echo "using postgres host: " $PGHOST >> $TESTLOGFILE 2>&1

checkEnv >> $TESTLOGFILE 2>&1

# check if db exists
psql  -d $TESTDATABASE -c "select * from distancetype" >>  $TESTLOGFILE 2>&1
if [ $? -eq 0 ]; then
  # the db exists
  echo "The database used by the test case exists. The test case will fail. In order to run this test delete/rename the database: " $TESTDATABASE
  
  
  # move logfile to final destination
  mv $TESTLOGFILE $SMAFETESTEXECPREFIX/

  
  exit 1
fi

# if no db exists create a testdb
# save pwd
PWDSAVE=$(pwd)
cd ../smafestore


# groups
psql  postgres -f smafestore.dump.groups.sql  >> $TESTLOGFILE 2>&1
# add test roles
psql --dbname postgres  < smafestore.dump.testroles.sql  >> $TESTLOGFILE 2>&1


# create db 
createdb   $TESTDATABASE   >> $TESTLOGFILE 2>&1
if [ $? -ne 0 ]; then
 dberror=1
 echo Error with createdb
fi 

# change owner
echo "ALTER DATABASE $TESTDATABASE OWNER TO smafeadmins" | psql   postgres  >> $TESTLOGFILE 2>&1


# the meat
psql --dbname $TESTDATABASE    < smafestore.dump.sql   >> $TESTLOGFILE 2>&1
if [ $? -ne 0 ]; then
 dberror=1
 echo Error with smafestore.dump.sql
fi 

# bootstrap data
psql --dbname $TESTDATABASE    < smafestore.bootstrap.data.sql   >> $TESTLOGFILE 2>&1
if [ $? -ne 0 ]; then
 dberror=1
  echo Error with smafestore.bootstrap.data.sql
fi 


createDBresult=$dberror

cd $PWDSAVE



# move logfile to final destination
mv $TESTLOGFILE $SMAFETESTEXECPREFIX/


# exit with success / error code
if [ $createDBresult -eq 0 ] 
then
  echo "Database $TESTDATABASE created successfully."
  exit 0 
else 
  echo "There was an error while creating the database $TESTDATABASE! For details see $SMAFETESTEXECPREFIX/$(basename $TESTLOGFILE)!"
  exit 1
fi 
