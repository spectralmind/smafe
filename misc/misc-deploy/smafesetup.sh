#!/bin/bash
# $Id$

echo
echo
echo "This script installs and performs initial setup of Smafe Core System and SMINT Api on this server."
echo "(c) 2009-2012 by Spectralmind GmbH"
echo
echo




# if less than the mandatory number of arguments is provided give usage
if [ $# -lt 6 ]; then
  echo "USAGE: $0 <postgres-host> <postgres-admin-user> <postgres-admin-user-password> <database-name> <pg-password-smafeadmin> <pg-password-smafeuser> [<postgres port>  [<tablespace name>]]"
  echo
  echo "with"
  echo
  echo "- <postgres-host> 			the host where postgres is running (localhost if local database server is used)"
  echo "- <postgres-admin-user> 	a user that has the right to create roles"
  echo "- <postgres-admin-password> the password for that admin user"
  echo "- <database-name> 			the name for the (new) database where the similarity model is stored"
  echo "- <pg-password-smafeadmin> 	the new password for the user for read-write-access"
  echo "- <pg-password-smafeuser> 	the new password for the user for read-access"
  echo "- <postgres port>	 		(optional) port for postgres server (default 5432)"
  echo "- <tablespace name>	 		(optional) postgres tablespace name where working database will be created. Note that that temporary test database will always be created in the default tablespace.  (default pg_default)"
  echo
  exit 2
else



  echo "Press key to continue..."; read -n1 -r
  

  
#####################################################################
# tests
#####################################################################
  
  
  
  
#### store parameters
  
  # store PGPASSWORD
  if [ -n $PGPASSWORD ]; then
    SAVEDPGPASSWORD=$PGPASSWORD
  fi
  # store PGUSER
  if [ -n $PGUSER ]; then
    SAVEDPGUSER=$PGUSER
  fi
  # store PGHOST
  if [ -n $PGHOST ]; then
    SAVEDPGHOST=$PGHOST
  fi
  OURPGHOST=$1
  OURPGUSER=$2
  OURPGPASSWORD=$3
  OURDBNAME=$4
  SMAFEADMINPWD=$5
  SMURFPWD=$6
  
  if [ $# -ge 7 ]; then
  	OURPGPORT=$7
  	if [ $OURPGPORT -ne 5432 ]; then
  		# not implemented yet for the tests
  		echo "Sorry: a non-standard port is currently not supported ($OURPGPORT)."
  		exit 1 
  	fi
  else
    OURPGPORT=5432
  fi
  if [ $# -ge 8 ]; then
  	OURPGTB=$8
  else
    OURPGTB=pg_default
  fi  
fi


#### prepare log file
  	TESTLOGFILE=$0.$$.log
	rm -f $TESTLOGFILE
	touch $TESTLOGFILE 
echo "Using logfile $TESTLOGFILE"

#### error flag
SMAFESETUP_ERROR_OCCURED=


#### check system
SMAFESETUP_UNAME_S=$(uname -s)
if [ x"$SMAFESETUP_UNAME_S" == xLinux ]; then
	echo "Linux detected."
	echo "Linux detected. uname -s was $SMAFESETUP_UNAME_S" >> $TESTLOGFILE 2>&1  
	SMAFESETUP_ISLINUX=1
	SMAFESETUP_ISMAC=
	SMAFESETUP_ISWIN=
fi
if [ x"$SMAFESETUP_UNAME_S" == xDarwin ]; then
	echo "Mac detected."
	echo "Mac detected. uname -s was $SMAFESETUP_UNAME_S" >> $TESTLOGFILE 2>&1  
	SMAFESETUP_ISLINUX=
	SMAFESETUP_ISMAC=1
	SMAFESETUP_ISWIN=
fi


#### check if already installed (config files are there)
# | tr -d '\t ' strips whitespace (POSIX compliant wc seems to format (pad) the output with leading whitespace
files=$(ls config/smafedbconn* 2> /dev/null | wc -l | tr -d '\t ')
if [ "$files" != "0" ]; then
	echo Found config files: >> $TESTLOGFILE 2>&1 
	ls config/smafedbconn* >> $TESTLOGFILE 2>&1 
	
	echo
	echo "It looks as if you have already installed Smafe (config files are present)."
	echo "You have these options:"
	echo "	1 - Re-install program files, test installation and (re-)create database (possibly overwriting manually changed files and database config files), OR"
	echo "	2 - Test installation, (re-)create database and database config files, as well as re-install SMINT API, OR"
	echo "	3 - Quit setup [default]"
	echo -n "Please choose an option.   (1/2/[3]) "
	read answer
	
	echo User option was: $answer >> $TESTLOGFILE 2>&1 
else
	answer=1
fi




[ "$answer" != "1" -a "$answer" != "2" ] && echo Quit setup && exit 0

  
if [ "$answer" == "1" ]; then
	#### extract archives
	echo
	echo "Extracting tar archives and installing program files..."
	echo "Extracting tar archives and installing program files..." >> $TESTLOGFILE 2>&1 
	tar xfj  smafedeploy.tar.bz2
	tar xfj smafedeploy-libs.tar.bz2
else
	#### move archived stuff from archive
	echo move archived stuff from archive >> $TESTLOGFILE 2>&1
	mv -f archive/* . 
fi



  
#####################################################################
# packages
#####################################################################


############### core #####################

# for linux we can install via apt-get
# for mac we download a package and unpack it

if [ -n "$SMAFESETUP_ISLINUX" ]; then
echo
echo "smafesetup can install all packages required for core Smafe via apt-get. You will need sudo rights for this."
echo "The following packages will be installed:"
echo " - ffmpeg and dependencies"

echo -n "Do you want smafesetup to install the packages?   (YES/no) "
read answer


if [ "$answer" != "no"  -a "$answer" != "n" -a "$answer" != "NO" -a "$answer" != "N"  ]; then

	echo "SMINT Core packages installation requested by user (answer was $answer)" >> $TESTLOGFILE 2>&1 
	echo Please enter sudo password:
	
	
	sudo apt-get install ffmpeg
	if [ $? -ne 0 ]; then
		SMAFESETUP_ERROR_OCCURED=yes
		echo "ERROR when installing packages" >> $TESTLOGFILE 2>&1 
		exit 1
		SMAFESETUP_CORE_PKGS_INSTALLED=
	else	
		SMAFESETUP_CORE_PKGS_INSTALLED=true
	fi
	
else
	echo "Core apt-get packages not installed. (user answer was $answer)" >> $TESTLOGFILE 2>&1 
	SMAFESETUP_CORE_PKGS_INSTALLED=
fi 

fi # [ -n "$SMAFESETUP_ISLINUX" ]; 


############## MAC install Core Dependencies
if [ -n "$SMAFESETUP_ISMAC" ]; then
	
	echo "ffmpeg can be found in subdirectory bin" >> $TESTLOGFILE 2>&1 
	
	# ffmpeg binary is in folder bin
	# how to set path to this file?

	
fi # -n "$SMAFESETUP_ISMAC" ]; 


############### API #####################
echo
echo "smafesetup can install all packages required for Smint API via apt-get. You will need sudo rights for this."
echo "The following packages will be installed:"
echo " - apache2"
echo " - php5"
echo " - php5-cgi" 
echo " - libapache2-mod-php5" 
echo " - php5-pgsql"
echo " - php5-curl"

echo -n "Do you want smafesetup to install the packages?   (YES/no) "
read answer


if [ "$answer" != "no"  -a "$answer" != "n" -a "$answer" != "NO" -a "$answer" != "N"  ]; then

	echo "SMINT API installation requested by user (answer was $answer)" >> $TESTLOGFILE 2>&1 
	echo Please enter sudo password:
	
	
	sudo apt-get install apache2 php5 php5-cgi libapache2-mod-php5 php5-pgsql php5-curl
	if [ $? -ne 0 ]; then
		SMAFESETUP_ERROR_OCCURED=yes
		echo "ERROR when installing packages" >> $TESTLOGFILE 2>&1 
		exit 1
		SMAFESETUP_API_PKGS_INSTALLED=
	else	
		SMAFESETUP_API_PKGS_INSTALLED=true
	fi
	
else
	echo "SMINT API packages not installed. (user answer was $answer)" >> $TESTLOGFILE 2>&1 
	SMAFESETUP_API_PKGS_INSTALLED=
fi 



PGUSER=$OURPGUSER
PGPASSWORD=$OURPGPASSWORD
PGHOST=$OURPGHOST
export PGUSER PGPASSWORD PGHOST




################# do testing 

echo
echo Testing server software installation...

# prerequisites: create build dir 
# -p: don't be angry if dir exists already
mkdir -p build


# test db
export TESTDATABASE=smafetestcasedb
# set to normal verbosity
export SMAFETESTPARAMV=3

# assume that all is ok
TESTOK=1

# build conn files for tests
DBCONNFILENAME=test/resources/localhost-testcasedb-dbconnection.opt
echo "creating db connection file $DBCONNFILENAME"  >> $TESTLOGFILE 2>&1 
rm -f $DBCONNFILENAME
touch $DBCONNFILENAME
echo "--dbhost=$OURPGHOST" >>  $DBCONNFILENAME
echo "--dbuser=testsmurf" >>  $DBCONNFILENAME
echo "--dbpwd=papa" >>  $DBCONNFILENAME
echo "--dbname=$TESTDATABASE" >>  $DBCONNFILENAME

DBCONNFILENAME=test/resources/localhost-testcasedb-dbconnection-admin.opt
echo "creating db connection file $DBCONNFILENAME"  >> $TESTLOGFILE 2>&1 
rm -f $DBCONNFILENAME
touch $DBCONNFILENAME
echo "--dbhost=$OURPGHOST" >>  $DBCONNFILENAME
echo "--dbuser=testsmafeadmin" >>  $DBCONNFILENAME
echo "--dbpwd=FEP13pipers" >>  $DBCONNFILENAME
echo "--dbname=$TESTDATABASE" >>  $DBCONNFILENAME




# drop database if database already exists
z=$(psql -U $OURPGUSER -h $OURPGHOST postgres -t -c "select * from pg_database d where datname='$TESTDATABASE'" | wc -l | tr -d '\t ' | cut -d" " -f 1);
if [ $z -ge 2 ]
then 

echo Database $TESTDATABASE already exists. Normally you can savely delete it since it is a temporary test database.
# --interactive to ask before dropping existing db (set by parmeter, see above) 
dropdb --interactive -h $OURPGHOST $TESTDATABASE  
# no need to check errors, since the database might not exist 

else

	echo "(Database $TESTDATABASE does not yet exist (=expected))"  >> $TESTLOGFILE 2>&1 

fi




# go into test
cd test

# set this var so that the test scripts do not change the PGUSER and PGPASSWORD vars
#PGDOUSERSET=noset
#export PGDOUSERSET

## the tests

./testCreateDB.sh  >> ../$TESTLOGFILE 2>&1 
if [ $? -ne 0 ]; then
  TESTOK=0
  echo "Error occured with testCreateDB.sh"
  SMAFESETUP_ERROR_OCCURED=yes
 # exit 1
fi

./testConfig.sh  >> ../$TESTLOGFILE 2>&1 
if [ $? -ne 0 ]; then
  TESTOK=0
    echo "Error occured with testConfig.sh"
    SMAFESETUP_ERROR_OCCURED=yes
  #exit 1
fi

./testWrap.sh  >> ../$TESTLOGFILE 2>&1 
if [ $? -ne 0 ]; then
  TESTOK=0
    echo "Error occured with testWrap.sh"
    SMAFESETUP_ERROR_OCCURED=yes
  #exit 1
fi

./testDistd.sh  >> ../$TESTLOGFILE 2>&1 
if [ $? -ne 0 ]; then
  TESTOK=0
    echo "Error occured with testDistd.sh"
    SMAFESETUP_ERROR_OCCURED=yes
  #exit 1
fi

./testTicket75_twobadfiles.sh  >> ../$TESTLOGFILE 2>&1 
if [ $? -ne 0 ]; then
  TESTOK=0
    echo "Error occured with testTicket75_twobadfiles.sh"
    SMAFESETUP_ERROR_OCCURED=yes
  #exit 1
fi

./testWrapNoDoubleFree.sh  >> ../$TESTLOGFILE 2>&1 
if [ $? -ne 0 ]; then
  TESTOK=0
    echo "Error occured with testWrapNoDoubleFree.sh"
    SMAFESETUP_ERROR_OCCURED=yes
  #exit 1
fi

./testTimbralExtractorNoFrames.sh  >> ../$TESTLOGFILE 2>&1 
if [ $? -ne 0 ]; then
  TESTOK=0
    echo "Error occured with testTimbralExtractorNoFrames.sh"
    SMAFESETUP_ERROR_OCCURED=yes
  #exit 1
fi

./testCleanupRemoveDB.sh  >> ../$TESTLOGFILE 2>&1 
if [ $? -ne 0 ]; then
  TESTOK=0
    echo "Error occured with testCleanupRemoveDB.sh"
    SMAFESETUP_ERROR_OCCURED=yes
  #exit 1
fi

./testCreateDB.sh  >> ../$TESTLOGFILE 2>&1 
if [ $? -ne 0 ]; then
  TESTOK=0
  echo "Error occured with testCreateDB.sh (2)"
  SMAFESETUP_ERROR_OCCURED=yes
 # exit 1
fi

./testConfig.sh  >> ../$TESTLOGFILE 2>&1 
if [ $? -ne 0 ]; then
  TESTOK=0
    echo "Error occured with testConfig.sh (2)"
    SMAFESETUP_ERROR_OCCURED=yes
  #exit 1
fi

# external file types test
./testFiletypes.sh  >> ../$TESTLOGFILE 2>&1 
if [ $? -ne 0 ]; then
  TESTOK=0
    echo "Error occured with testFiletypes.sh"
    SMAFESETUP_ERROR_OCCURED=yes
  #exit 1
fi


./testCleanupRemoveDB.sh  >> ../$TESTLOGFILE 2>&1 
if [ $? -ne 0 ]; then
  TESTOK=0
    echo "Error occured with testCleanupRemoveDB.sh (2)"
    SMAFESETUP_ERROR_OCCURED=yes
  #exit 1
fi

./testCreateDB.sh  >> ../$TESTLOGFILE 2>&1 
if [ $? -ne 0 ]; then
  TESTOK=0
  echo "Error occured with testCreateDB.sh (2)"
  SMAFESETUP_ERROR_OCCURED=yes
 # exit 1
fi

./testDistd_extended.sh  >> ../$TESTLOGFILE 2>&1 
if [ $? -ne 0 ]; then
  TESTOK=0
    echo "Error occured with testDistd_extended.sh"
  #exit 1
fi

./testCleanupRemoveDB.sh  >> ../$TESTLOGFILE 2>&1 
if [ $? -ne 0 ]; then
  TESTOK=0
    echo "Error occured with testCleanupRemoveDB.sh"
  #exit 1
fi

./testCreateDB.sh  >> ../$TESTLOGFILE 2>&1 
if [ $? -ne 0 ]; then
  TESTOK=0
  echo "Error occured with testCreateDB.sh (2)"
  SMAFESETUP_ERROR_OCCURED=yes
 # exit 1
fi

./testDistd_live.sh  >> ../$TESTLOGFILE 2>&1 
if [ $? -ne 0 ]; then
  TESTOK=0
    echo "Error occured with testDistd_live.sh"
  #exit 1
fi

./testCleanupRemoveDB.sh  >> ../$TESTLOGFILE 2>&1 
if [ $? -ne 0 ]; then
  TESTOK=0
    echo "Error occured with testCleanupRemoveDB.sh"
  #exit 1
fi


# only if fv is encrypted  >> ../$TESTLOGFILE 2>&1 
#./testDistd_live.sh
#if [ $? -ne 0 ]; then
#  TESTOK=0
# echo "Error occured with testDistd_live.sh"
#  exit 1
#fi


cd ..


#restore PGPASSWORD
	if [ -n $SAVEDPGPASSWORD ]; then
	  PGPASSWORD=$SAVEDPGPASSWORD
	fi
	#restore 
	if [ -n $SAVEDPGUSER ]; then
  		PGUSER=$SAVEDPGUSER
	fi
	#restore 
	if [ -n $SAVEDPGHOST ]; then
  		PGHOST=$SAVEDPGHOST
	fi


##### clean up on failed tests

if [ $TESTOK -eq 0 ]; then

	# try to clean up the mess
	test/testCleanupRemoveDB.sh  >> $TESTLOGFILE 2>&1 

	# make debug package
	SMAFEDEBUGPACKNAME=testsfailed.tgz
	
	# get ldd infos 
	rm -f build/ldd-info.log
	touch build/ldd-info.log
	echo ldd bin/bin-internal/smafewrapd >> build/ldd-info.log
	ldd bin/bin-internal/smafewrapd >> build/ldd-info.log
	echo ldd bin/bin-internal/smafedistd >> build/ldd-info.log
	ldd bin/bin-internal/smafedistd >> build/ldd-info.log
	echo ldd bin/bin-internal/smuiupdated >> build/ldd-info.log
	ldd bin/bin-internal/smuiupdated >> build/ldd-info.log
	
	# copy log file to buil dir
	cp $TESTLOGFILE build
	
	# zip build dir
	tar cfz $SMAFEDEBUGPACKNAME build
	
	SMAFESETUP_ERROR_OCCURED=yes

	echo Test of installation was not successful. $SMAFEDEBUGPACKNAME contains relevant log files.
	echo Please contact support@spectralmind.com and attach $SMAFEDEBUGPACKNAME.
	echo Exiting
	
	

	exit 1
fi 




echo "Tests successful."
echo
echo -n "Create working database?   [YES/no] "
read answer
	
echo Question: create db? User option was: $answer >> $TESTLOGFILE 2>&1 

if [ "$answer" != "no"  -a "$answer" != "n" -a "$answer" != "NO" -a "$answer" != "N"  ]; then


#####################################################################
# setup
#####################################################################



################# setup real working database


PGUSER=$OURPGUSER
PGPASSWORD=$OURPGPASSWORD
PGHOST=$OURPGHOST
export PGUSER PGPASSWORD PGHOST

echo creating roles in postgres and setting passwords >> $TESTLOGFILE 2>&1 
#psql -U $OURPGUSER -h $OURPGHOST postgres -f res/smafestore.dump.groups.sql  >> $TESTLOGFILE 2>&1 
psql -U $OURPGUSER -h $OURPGHOST postgres -f res/smafestore.dump.roles.sql  >> $TESTLOGFILE 2>&1 
psql -U $OURPGUSER -h $OURPGHOST postgres -c "alter role smafeadmin with password '$SMAFEADMINPWD';"  >> $TESTLOGFILE 2>&1 
psql -U $OURPGUSER -h $OURPGHOST postgres -c "alter role smurf with password '$SMURFPWD';"  >> $TESTLOGFILE 2>&1 


dberror=0

# drop database if database already exists
z=$(psql -U $OURPGUSER -h $OURPGHOST postgres -t -c "select * from pg_database d where datname='$OURDBNAME'" | wc -l | tr -d '\t ' | cut -d" " -f 1);
if [ $z -ge 2 ]
then 

echo Database $OURDBNAME already exists.
# --interactive to ask before dropping existing db (set by parmeter, see above) 
dropdb --interactive -h $OURPGHOST $OURDBNAME  
# no need to check errors, since the database might not exist 

else

	echo "(Database does not yet exist)"  >> $TESTLOGFILE 2>&1 

fi


# create db 
createdb  -h $OURPGHOST --tablespace $OURPGTB --port $OURPGPORT $OURDBNAME  >> $TESTLOGFILE 2>&1 
if [ $? -ne 0 ]; then
 dberror=1
fi 


if [ $dberror -eq 0 ]; then
# change owner
echo "ALTER DATABASE \"$OURDBNAME\" OWNER TO smafeadmins" | psql  -h  $OURPGHOST --port $OURPGPORT postgres   >> $TESTLOGFILE 2>&1 
if [ $? -ne 0 ]; then
 dberror=1
fi 
fi

if [ $dberror -eq 0 ]; then
# the meat
psql --dbname $OURDBNAME --port $OURPGPORT  -h $OURPGHOST < res/smafestore.dump.sql   >> $TESTLOGFILE 2>&1 
if [ $? -ne 0 ]; then
 dberror=1
fi 
fi 

if [ $dberror -eq 0 ]; then
# bootstrap data
psql --dbname $OURDBNAME --port $OURPGPORT  -h $OURPGHOST < res/smafestore.bootstrap.data.sql   >> $TESTLOGFILE 2>&1 
if [ $? -ne 0 ]; then
 dberror=1
fi 
fi 


PGUSER=smafeadmin
PGPASSWORD=$SMAFEADMINPWD
PGHOST=$OURPGHOST
export PGUSER PGPASSWORD PGHOST


if [ $dberror -eq 0 ]; then
# config
psql --dbname $OURDBNAME --port $OURPGPORT -h $OURPGHOST < res/config.opt.sql   >> $TESTLOGFILE 2>&1 
if [ $? -ne 0 ]; then
 dberror=1
fi 
fi 

#restore PGPASSWORD
if [ -n $SAVEDPGPASSWORD ]; then
  PGPASSWORD=$SAVEDPGPASSWORD
fi
#restore 
if [ -n $SAVEDPGUSER ]; then
  PGUSER=$SAVEDPGUSER
fi
#restore 
if [ -n $SAVEDPGHOST ]; then
	PGHOST=$SAVEDPGHOST
fi


PGDOUSERSET=

export PGUSER PGPASSWORD PGHOST PGDOUSERSET


if [ $dberror -ne 0 ]; then
 echo ERROR: database $OURDBNAME could not be created. Exiting.
 SMAFESETUP_ERROR_OCCURED=yes
 exit 1
fi 

DB_INSTALLED=true

fi # create db?


## connection files

mkdir -p config

# build filename (change also below!)
DBCONNFILENAME1=config/smafedbconn-$OURPGHOST-$OURDBNAME-smafeuser.opt
echo "creating db connection file $DBCONNFILENAME1"  >> $TESTLOGFILE 2>&1 
rm -f $DBCONNFILENAME1
touch $DBCONNFILENAME1
echo "--dbhost=$OURPGHOST" >>  $DBCONNFILENAME1
echo "--dbuser=smurf" >>  $DBCONNFILENAME1
echo "--dbpwd=$SMURFPWD" >>  $DBCONNFILENAME1
echo "--dbname=$OURDBNAME" >>  $DBCONNFILENAME1


# build filename (change also below!)
DBCONNFILENAME2=config/smafedbconn-$OURPGHOST-$OURDBNAME-smafeadmin.opt
echo "creating db connection file $DBCONNFILENAME2"  >> $TESTLOGFILE 2>&1 
rm -f $DBCONNFILENAME2
touch $DBCONNFILENAME2
echo "--dbhost=$OURPGHOST" >>  $DBCONNFILENAME2
echo "--dbuser=smafeadmin" >>  $DBCONNFILENAME2
echo "--dbpwd=$SMAFEADMINPWD" >>  $DBCONNFILENAME2
echo "--dbname=$OURDBNAME" >>  $DBCONNFILENAME2



#####################################################################
# create smafed.conf
#####################################################################

# build filename (change also above!)
DBCONNFILENAME1=config/smafedbconn-$OURPGHOST-$OURDBNAME-smafeuser.opt
DBCONNFILENAME2=config/smafedbconn-$OURPGHOST-$OURDBNAME-smafeadmin.opt

# read daemon params from res file
RESFILE=res/smafed.conf-input.txt
# This magic sed command means:
#	find the part after a non-space-containing string (starting with ###_) and a space and save it in group \1, and output group \1
#	Should be the same as 
#		cut -d " " -f 1 --complement
#	but --complement does not work on Mac os x
DAEMON1_ARGS_IN=$(egrep "^###_smafewrapd-params" $RESFILE | sed  "s/^###_[^ ]* \(.*\)$/\1/" )
DAEMON2_ARGS_IN=$(egrep "^###_smafedistd-params" $RESFILE | sed  "s/^###_[^ ]* \(.*\)$/\1/" )
echo smafewrapd params as read from file: $DAEMON1_ARGS_IN >> $TESTLOGFILE
echo smafedistd params as read from file: $DAEMON2_ARGS_IN >> $TESTLOGFILE

# create smafed.conf
echo \# created by smafesetup 		> system/smafed.conf
echo -n \#							>> system/smafed.conf
date 								>> system/smafed.conf
echo DAEMONPATH=$(pwd)				>> system/smafed.conf
echo DAEMON1_ARGS=\"--dbconf=$DBCONNFILENAME1 $DAEMON1_ARGS_IN\"			>> system/smafed.conf
echo DAEMON2_ARGS=\"--jobs --dbconf=$DBCONNFILENAME2  $DAEMON2_ARGS_IN\"			>> system/smafed.conf
echo DAEMON_CHUID=$(whoami)			>> system/smafed.conf
echo \# end of file					>> system/smafed.conf

# make executable
chmod +x system/smafed.conf

echo smafed.conf created.
echo smafed.conf created.  >> $TESTLOGFILE 2>&1






#####################################################################
# init.d script
#####################################################################


# uses $DBCONNFILENAME1 and $DBCONNFILENAME2 from previous section
echo
echo "smafesetup can install the init.d script and runlevel symlinks to have the daemons started automatically on boot time."
echo You will need sudo rights for this step.
echo "Do you want smafesetup to install the init.d script and create runlevel symlinks?"
echo -n "NOTE: Custom changes in /etc/init.d/smafed* will be LOST.                              (YES/no) "
read answer

if [ "$answer" != "no"  -a "$answer" != "n" -a "$answer" != "NO" -a "$answer" != "N"  ]; then

	echo installing script into /etc/init.d/ >> $TESTLOGFILE 2>&1 
	echo Please enter sudo password:
	sudo cp system/smafed* /etc/init.d/
	[ $? -ne 0 ] && SMAFESETUP_ERROR_OCCURED=yes && echo "ERROR when installing script into /etc/init.d/" >> $TESTLOGFILE 2>&1 
	
	echo creating symlinks >> $TESTLOGFILE 2>&1 
	sudo update-rc.d smafed defaults
	[ $? -ne 0 ]  && SMAFESETUP_ERROR_OCCURED=yes && echo "ERROR when creating symlinks" >> $TESTLOGFILE 2>&1 
	
	echo
	echo "smafesetup can start the daemons right away."
	echo You will need sudo rights for this step.
	echo -n "Do you want smafesetup to start the daemons right away?   (yes/NO) "
	read answer
	
	if [ "$answer" == "yes" ]; then
		sudo /etc/init.d/smafed start 
		[ $? -ne 0 ]  && SMAFESETUP_ERROR_OCCURED=yes && echo "ERROR when starting daemons" >> $TESTLOGFILE 2>&1 
	fi
	
	INITD_INSTALLED=true
	
else
	echo init.d script NOT installed. >> $TESTLOGFILE 2>&1 
	INITD_INSTALLED=
fi 

  


#####################################################################
# SMINT API
#####################################################################




# uses these variables from previous sections:
# OURDBNAME
# OURPGPORT
# OURPGHOst
# SMURFPWD

echo
echo
echo "Smafesetup can install SMINT API on localhost in the default location, prepared to be used with a standard installation of Apache 2."
echo
echo "Prerequisites and assumptions: "
echo
echo " -) Apache 2 config files are to be installed in standard location (/etc/apache2/sites-enabled and /etc/apache2/sites-available)"
echo
echo " -) Public alias for accessing SMINT API is 'smintapi'"
echo
echo " -) Files that have been created by smafesetup during an previous setup run are *overwritten* without notice even if they have been modified later."
echo

echo You will need sudo rights for this step.
echo -n "Do you want smafesetup to install SMINT API and configure Apache 2 to use it?   (YES/no) "
read answer

if [ "$answer" != "no"  -a "$answer" != "n" -a "$answer" != "NO" -a "$answer" != "N"  ]; then

	echo "SMINT API installation requested by user (answer was $answer)" >> $TESTLOGFILE 2>&1 

	### PDO
	echo PDO installation - should have been done already when installing the packages. >> $TESTLOGFILE 2>&1 
	
	### untar (done already)
	
	### make log directory world writeable
	chmod o+w smintapi/log
	
	### create apache config
	echo apache config file creation  >> $TESTLOGFILE 2>&1
	
	# get absolute path 
	CURRENTDIR=`pwd`
	ABSPATH=`cd smintapi/web; pwd`
	SMINTAPI_PUBLICDIR=$ABSPATH/
	cd $CURRENTDIR
	
	# write apache config file 
	SMINTAPI_APACHECONF_NAMEONLY=smintapi-apache.conf
	SMINTAPI_APACHECONF=system/$SMINTAPI_APACHECONF_NAMEONLY
	
	rm -f $SMINTAPI_APACHECONF
	
	# create $SMINTAPI_APACHECONF
	echo \# created by smafesetup 				> $SMINTAPI_APACHECONF
	echo -n \#									>> $SMINTAPI_APACHECONF
	date 										>> $SMINTAPI_APACHECONF
	echo "Alias /smintapi $SMINTAPI_PUBLICDIR" 	>> $SMINTAPI_APACHECONF
	echo "	<Directory  $SMINTAPI_PUBLICDIR>" 	>> $SMINTAPI_APACHECONF
	echo "        AllowOverride All" 			>> $SMINTAPI_APACHECONF
	echo "        Allow from All" 				>> $SMINTAPI_APACHECONF
	echo "        Options +FollowSymLinks" 		>> $SMINTAPI_APACHECONF
	echo "	</Directory> " 						>> $SMINTAPI_APACHECONF
	echo \# end of file							>> $SMINTAPI_APACHECONF

	echo "$SMINTAPI_APACHECONF created."
	echo "$SMINTAPI_APACHECONF created" >> $TESTLOGFILE 2>&1
	
	### copy apache config file to apache etc section, and create symlink
	echo copy apache config file to apache etc section, and create symlink  >> $TESTLOGFILE 2>&1
	sudo mv $SMINTAPI_APACHECONF /etc/apache2/sites-available/  >> $TESTLOGFILE 2>&1
	[ $? -ne 0 ] && SMAFESETUP_ERROR_OCCURED=yes  && echo "ERROR moving apache config to sites-available" >> $TESTLOGFILE 2>&1
	# use force
	sudo ln -s -f /etc/apache2/sites-available/$SMINTAPI_APACHECONF_NAMEONLY /etc/apache2/sites-enabled/$SMINTAPI_APACHECONF_NAMEONLY  >> $TESTLOGFILE 2>&1
	[ $? -ne 0 ]  && SMAFESETUP_ERROR_OCCURED=yes && echo "ERROR creating symlink from  /etc/apache2/sites-available/$SMINTAPI_APACHECONF_NAMEONLY to /etc/apache2/sites-enabled/$SMINTAPI_APACHECONF_NAMEONLY " >> $TESTLOGFILE 2>&1
		
		
	### create smintapi config file
	echo create smintapi config file  >> $TESTLOGFILE 2>&1
	
	SMINTAPICONF_NAMEONLY=smintapi.ini
	SMINTAPICONF=smintapi/config/$SMINTAPICONF_NAMEONLY
	SMINTAPICONF_TEMPLATE=$SMINTAPICONF.template
	
	# copy template to real file
	cp $SMINTAPICONF_TEMPLATE $SMINTAPICONF
	[ $? -ne 0 ] && SMAFESETUP_ERROR_OCCURED=yes  && echo "ERROR copying smintapi config template to smintapi config file instance" >> $TESTLOGFILE 2>&1
	
	# replace template blindtext with real data
	# (all in-place)
	# Syntax for sed queries:
	# (beginning of line)keyword(whitespace optional)=(whitespace optional)"(dummy value)"
	sed -i "s|^dbname\s*=\s*//|dbname=\"$OURDBNAME\"; 			//edit by smafesetup   |g"    $SMINTAPICONF
	sed -i "s|^dbport\s*=\s*//|dbport=\"$OURPGPORT\"; 			//edit by smafesetup   |g"    $SMINTAPICONF
	sed -i "s|^dbhost\s*=\s*//|dbhost=\"$OURPGHOST\"; 			//edit by smafesetup   |g"    $SMINTAPICONF
	sed -i "s|^dbuser\s*=\s*//|dbuser=\"smurf\"; 			//edit by smafesetup   |g"    $SMINTAPICONF
	sed -i "s|^dbpassword\s*=\s*//|dbpassword=\"$SMURFPWD\"; 			//edit by smafesetup   |g"    $SMINTAPICONF

	echo "; template edited by smafesetup" 				>> $SMINTAPICONF
	echo -n ";"										>> $SMINTAPICONF
	date 											>> $SMINTAPICONF
	echo $SMINTAPICONF created.
	echo $SMINTAPICONF created. >> $TESTLOGFILE 2>&1		
		
		
	### only if liveapi.php controller is present

	if [ -f  smintapi/web/liveapi.php ]; then
		echo "liveapi controller found, installing live api"  >> $TESTLOGFILE 2>&1

		### create liveapi config file
		echo create liveapi config file  >> $TESTLOGFILE 2>&1
	
		SMINTAPICONF_NAMEONLY=liveapi.ini
		SMINTAPICONF=smintapi/config/$SMINTAPICONF_NAMEONLY
		SMINTAPICONF_TEMPLATE=$SMINTAPICONF.template
	
		# copy template to real file
		cp $SMINTAPICONF_TEMPLATE $SMINTAPICONF
		[ $? -ne 0 ] && SMAFESETUP_ERROR_OCCURED=yes  && echo "ERROR copying liveapi config template to liveapi config file instance" >> $TESTLOGFILE 2>&1
	
		# replace template blindtext with real data
		# (all in-place)
		# Syntax for sed queries:
		# (beginning of line)keyword(whitespace optional)=(whitespace optional)"(dummy value)"
		sed -i "s|^dbname\s*=\s*//|dbname=\"$OURDBNAME\"; 			//edit by smafesetup   |g"    $SMINTAPICONF
		sed -i "s|^dbport\s*=\s*//|dbport=\"$OURPGPORT\"; 			//edit by smafesetup   |g"    $SMINTAPICONF
		sed -i "s|^dbhost\s*=\s*//|dbhost=\"$OURPGHOST\"; 			//edit by smafesetup   |g"    $SMINTAPICONF
		sed -i "s|^dbuser\s*=\s*//|dbuser=\"smurf\"; 			//edit by smafesetup   |g"    $SMINTAPICONF
		sed -i "s|^dbpassword\s*=\s*//|dbpassword=\"$SMURFPWD\"; 			//edit by smafesetup   |g"    $SMINTAPICONF

		echo "; template edited by smafesetup" 				>> $SMINTAPICONF
		echo -n ";"										>> $SMINTAPICONF
		date 											>> $SMINTAPICONF
		echo $SMINTAPICONF created.
		echo $SMINTAPICONF created. >> $TESTLOGFILE 2>&1	


		echo "Please review section liveapi in $SMINTAPICONF "
		echo "Please review section liveapi in $SMINTAPICONF " >> $TESTLOGFILE 2>&1
	
	else
		echo "liveapi controller found, installing live api"  >> $TESTLOGFILE 2>&1

	fi ### END liveapi


		
	### restart apache
	echo restart apache  >> $TESTLOGFILE 2>&1
	sudo /etc/init.d/apache2 restart   >> $TESTLOGFILE 2>&1
	[ $? -ne 0 ] && SMAFESETUP_ERROR_OCCURED=yes  && echo "ERROR restarting apache" >> $TESTLOGFILE 2>&1
	
	### do a simple test
	echo testing API  >> $TESTLOGFILE 2>&1
	smintapi/test/script-tests/testAccessibility.sh http://localhost/smintapi/
	if [ $? -ne 0 ]; then
		SMAFESETUP_ERROR_OCCURED=yes  && echo "ERROR @ testAccessibility.sh" >> $TESTLOGFILE 2>&1
		SMINTAPI_INSTALLED=
	else
		SMINTAPI_INSTALLED=true
	fi
		
	
	
else
	echo "SMINT API not installed. (user answer was $answer)" >> $TESTLOGFILE 2>&1 
	SMINTAPI_INSTALLED=
fi 

  




#####################################################################
# aftermath
#####################################################################


# shuffle the files a bit
# move install stuff to archive
rm -rf archive
mkdir -p archive
mv -f test archive
mv -f build archive
mv -f res archive
mv -f smafestore archive
mv -f system archive

# create symlink for doc/README in root dir
[ -e README ] || ln -s doc/README 


if test x"$SMAFESETUP_ERROR_OCCURED" = x"yes"; then

echo
echo "At least one error occured."
echo "Please contact our support team at support@spectralmind.com and attach the log files $TESTLOGFILE and testAccessibility.log."
echo "Sorry for the inconvenience and thank you for your patience!"
echo

else


echo
echo Smafe setup successfully finished.
echo
echo 
[ "$DB_INSTALLED" ] && echo Database $OURDBNAME has been set up with a configuration tailered specifically by Spectralmind to your needs.
[ "$DB_INSTALLED" ] && echo

[ "$INITD_INSTALLED" ] && echo You can check the status of Smafe daemons with
[ "$INITD_INSTALLED" ] && echo -e "\t/etc/init.d/smafed status"
[ "$INITD_INSTALLED" ] && echo
 
[ "$SMINTAPI_INSTALLED" ] && echo "SMINT API has been installed and is accessible at"
[ "$SMINTAPI_INSTALLED" ] && echo -e "\thttp://localhost/smintapi/smintapi.php/"
[ "$SMINTAPI_INSTALLED" ] && echo "Example: To query SMINT API version, use"
[ "$SMINTAPI_INSTALLED" ] && echo -e "\twget --header='Accept:application/xml' http://localhost/smintapi/smintapi.php/version"
[ "$SMINTAPI_INSTALLED" ] && echo 

[ "$INITD_INSTALLED" ] && echo You can start the core system daemons with
[ "$INITD_INSTALLED" ] && echo -e "\t/etc/init.d/smafed start"
[ "$INITD_INSTALLED" ] && echo

echo 
echo Thank you for using Smafe.
echo

fi
