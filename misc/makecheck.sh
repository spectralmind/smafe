#!/bin/bash


LOGFILE=misc/makecheck.log

echo "This script does configure and make check. It is expected to be run from \$SMAFEROOT/misc."
echo "These two directories are used: build4deploy and build4admin"
echo "Note that they are deleted at the beginning of this script."
echo "Logfile name is $LOGFILE"


# go down
cd ..


rm -f $LOGFILE


# perform cleanup
test/cleanup.sh  >> $LOGFILE 2>&1


echo "first create the configuration scripts"
# # -f to force overwriting the svnversion macro (otherwise we always get the old version of the macro from the SVN which does not reflect the current revision)
autoreconf -f  >> $LOGFILE 2>&1




########## deploy version
BUILDDIR=build4deploy
rm -rf $BUILDDIR
mkdir $BUILDDIR
cd $BUILDDIR
../configure $@ >> ../$LOGFILE 2>&1
# set our build dir
export SMAFETESTEXECPREFIX=../$BUILDDIR/
make check >> ../$LOGFILE 2>&1
cd ..


echo " " >> $LOGFILE 2>&1
echo " ">> $LOGFILE 2>&1
echo " ">> $LOGFILE 2>&1
echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~">> $LOGFILE 2>&1
echo " ">> $LOGFILE 2>&1
echo " ">> $LOGFILE 2>&1
echo " ">> $LOGFILE 2>&1

########## admin version
BUILDDIR=build4admin
rm -rf $BUILDDIR
mkdir $BUILDDIR
cd $BUILDDIR
../configure --disable-deploy $@ >> ../$LOGFILE 2>&1
export SMAFETESTEXECPREFIX=../$BUILDDIR/
make check >> ../$LOGFILE 2>&1
cd ..

### check errors

grep FAIL $LOGFILE -B 2
grep ========== $LOGFILE -B 2

echo -n "Do not forget to check the date: "
date 

## go up again
cd misc






