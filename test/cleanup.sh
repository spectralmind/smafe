#!/bin/bash
# online resources: 
# http://steve-parker.org/sh/sh.shtml
# http://www.freeos.com/guides/lsst/
# http://www.ss64.com/bash/


##########################################################################################
#
#	This script cleans up everything in case something should be left over
#	from other tests or debugging
#
#-----------------------------------------------------------------------------------------
#
#	Actions:
#
#		# execute testCleanupRemoveDB.sh (removes testcasedb)
#		# kills smafewrapd daemons with certain id
#		# kills smafedistd daemons with certain id
#		# kills process that is listening on port 1212
#		# deletes all copies of sound files that should have been processed, and moved
#			but weren't (*fortesting)
#
#-----------------------------------------------------------------------------------------
#
#	Note:
#
##########################################################################################


echo Trying to delete test db

# drop testdb
dropdb  testcasedb 

# delete test roles
psql  postgres -c "drop role testsmafeadmin;"
psql  postgres -c "drop role testsmurf;"




echo Trying to  kill smafewrapd daemons with id testsmafewrapd

for signal in "-15" "-1" "-9"
do
  pids=$(ps ux |grep testsmafewrapd | grep -v grep | awk '{ print $2 }')
  kill $signal $pids 2> /dev/null
done



echo Trying to  kill smafedistd daemons with id idtestDistd_extended2

for signal in "-15" "-1" "-9"
do
  pids=$(ps ux |grep idtestDistd_extended2 | grep -v grep | awk '{ print $2 }')
  kill $signal $pids 2> /dev/null
done



echo Trying to  kill process that is listening on port 1212

for signal in "-15" "-1" "-9"
do
  pids=$(lsof -i :1212 -t)
  kill $signal $pids 2> /dev/null
done

echo "Deleting all *fortesting* files"

find . -name "*fortesting*" -exec rm {} \; 


