#!/bin/bash

echo "This script builds smafe and creates a deploy package. It is expected to be run from directory misc"


#### check system
# (this section occurs also in test/checkEnv.lib)
SMAFESETUP_UNAME_S=$(uname -s)
if [ x"$SMAFESETUP_UNAME_S" == xLinux ]; then
	echo "Linux detected."
	#echo "Linux detected. uname -s was $SMAFESETUP_UNAME_S" >> $TESTLOGFILE 2>&1  
	SMAFESETUP_ISLINUX=1
	SMAFESETUP_ISMAC=
	SMAFESETUP_ISWIN=
elif [ x"$SMAFESETUP_UNAME_S" == xDarwin ]; then
	echo "Mac detected."
	#echo "Mac detected. uname -s was $SMAFESETUP_UNAME_S" >> $TESTLOGFILE 2>&1  
	SMAFESETUP_ISLINUX=
	SMAFESETUP_ISMAC=1
	SMAFESETUP_ISWIN=
else
	echo "Unknown system: $SMAFESETUP_UNAME_S"
	exit 1
fi


# check os version and set some variables 
unamestr=`uname`
dynlibextension='so'

if [ -n "$SMAFESETUP_ISLINUX" ]; then
	dynlibextension='so'
elif [ -n "$SMAFESETUP_ISMAC" ]; then
   dynlibextension='dylib'
fi



if test -n "$LIB_FOLDER"; then
	echo
	# use the provided directory
else
	echo
	# use default directory
	  if [ -n "$SMAFESETUP_ISMAC" ]; then
			LIB_FOLDER=/opt/local/lib/
	  else
			LIB_FOLDER=/usr/lib/
	  fi
fi
echo "Using $LIB_FOLDER as standard lib folder (set \$LIB_FOLDER to change)" 


if test -n "$LIBTOOL_FOLDER"; then
	echo
	# use the provided directory
else
	echo
	# use default directory
	LIBTOOL_FOLDER=$LIB_FOLDER
fi
echo "Using $LIBTOOL_FOLDER as libtool folder (set \$LIBTOOL_FOLDER to change)" 



if test -n "$BOOST_LIB_FOLDER"; then
	echo
	# use the provided directory
else
	echo
	# use default directory
	BOOST_LIB_FOLDER=/usr/local/lib/
fi
echo "Using $BOOST_LIB_FOLDER as Boost folder (set \$BOOST_LIB_FOLDER to change)" 


if test -n "$MPG123_LIB_FOLDER"; then
	echo
	# use the provided directory
else
	echo
	# use default directory
	MPG123_LIB_FOLDER=/usr/local/lib/
fi
echo "Using $MPG123_LIB_FOLDER as mpg123 lib folder (set \$MPG123_LIB_FOLDER to change)" 

echo "Using $INTEL_IPP_LIB_FOLDER as IPP lib folder (set \$INTEL_IPP_LIB_FOLDER to change)"

if test -n "$INTEL_IPP_COMPILERLIB_FOLDER"; then
	echo
	# use the provided directory
else
	echo
	# use default directory
	INTEL_IPP_COMPILERLIB_FOLDER=$INTEL_IPP_LIB_FOLDER
fi
echo "Using $INTEL_IPP_COMPILERLIB_FOLDER as IPP lib folder (set \$INTEL_IPP_COMPILERLIB_FOLDER to change)"
echo
echo "All your arguments after the <config file> are passed to configure." 




# if less than 1 argument is provided give usage
if [ $# -lt 1 ]; then
  echo "USAGE: $0 <config file> [configure parameters ... ]"
  exit 2
else
    # readlink -f is not supported on non linux platforms like macos
     
    CURRENTDIR=`pwd`
    FILEPATH=`dirname $1`
    ABSPATH=`cd $FILEPATH; pwd`
    FILENAME=`basename $1`
    OPTFILE=$ABSPATH/$FILENAME
    cd $CURRENTDIR

    # linux systems 
    # readlink gets the full path name -f canonicalizes the path
    #OPTFILE=$(readlink -f $1)
 
  echo "using $OPTFILE as config file"
  # for later: sql file
  SQLFILE=$OPTFILE.sql
fi

echo
echo "Press key to continue..."; read -n1 -r
echo "Started"


# go down
cd ..


echo "first create the configuration scripts"
# -f to force overwriting the svnversion macro (otherwise we always get the old version of the macro from the SVN which does not reflect the current revision)
# why do we need --install? #--install
# in order to have the newest revision number reported by svnversion (in configure)
# --install would recreate build-aux stuff that would create modifications shown by "svn status"
autoreconf --force --install
# !! with force and install it does not work with libtool 1.5 (various error messages with X-...) !!
# so: for eg Debian Lenny: use the following line
#autoreconf 


BUILDDIR=build4deploy
BUILDDIR2=build4admin


INSTALLDIR=$(pwd)/deploy-packages/$(basename $OPTFILE)-TBD

# check if dir exists, if yes append a random string
while [  -d $INSTALLDIR  ]; do
	echo "$INSTALLDIR already exists"
	INSTALLDIR=$INSTALLDIR--$$
done
echo "Using $INSTALLDIR"
mkdir $INSTALLDIR

## get deploy mode
# grep for variable (must be at beginning of line), cut at space and take rest of line, trim whitespace (eg, if you have more than one space between parameter keyword and value)
OPT_deployscenario=$(grep "^###_deployscenario" $OPTFILE | cut -d " " -f 2- | sed "s/\s//g")
# if empty, use default
if [ x"$OPT_deployscenario" == x"" ]; then
	echo "###_deployscenario was not found in config file, assuming default deploy scenario"
	OPT_deployscenario=default
fi
# ## default
if [ x"$OPT_deployscenario" == x"default" ]; then
	OPT_libs=1		# not implemented
	OPT_smafe=1		# not implemented
	OPT_smintapi=1		# not implemented
	OPT_liveapi=0
fi
# ## liveapi
if [ x"$OPT_deployscenario" == x"liveapi" ]; then
	OPT_libs=1		# not implemented
	OPT_smafe=1		# not implemented
	OPT_smintapi=1		# not implemented
	OPT_liveapi=1
fi



# delete anyhting left over from last time
rm -rf $BUILDDIR
rm -rf $BUILDDIR2
rm -rf $INSTALLDIR

# remove the first argument value
shift 1
echo "using additional configure parameters: $@"


mkdir $BUILDDIR2
cd $BUILDDIR2
echo "configure..."
../configure --disable-deploy $@
make
# create sql script with config
smafewrapd/smafewrapd --config $OPTFILE
if [ $? == 0 ]; then
  echo "config file successfully created"
else
  echo "ERROR creating config file"
  cd ../misc
  exit 1
fi
cd ..



mkdir $BUILDDIR
cd $BUILDDIR
echo "configure..."
../configure --prefix=$INSTALLDIR $@

echo "make and make install ..."
# make 
make
# make install
make install
cd ..


# Move binaries to subdir in bin
mkdir $INSTALLDIR/bin/bin-internal
mv $INSTALLDIR/bin/sm* $INSTALLDIR/bin/bin-internal


echo "copy wrapper scripts for each executable"
# inspired by http://doc.qt.nokia.com/4.6/deployment-x11.html#creating-the-application-package
# the script is always the same, but depending on the name it launches different binaries 
if [ -n "$SMAFESETUP_ISMAC" ]; then
  smwrapperscriptfile='misc/misc-deploy/sm-wrapper-script-macosx.sh'
else
  # linux systems 
  smwrapperscriptfile='misc/misc-deploy/sm-wrapper-script.sh'
fi


cp $smwrapperscriptfile $INSTALLDIR/bin/smuiupdated.sh
cp $smwrapperscriptfile $INSTALLDIR/bin/smafewrapd.sh
cp $smwrapperscriptfile $INSTALLDIR/bin/smafedistd.sh
# run script to start/stop/restart/status daemons
cp misc/misc-deploy/run.sh $INSTALLDIR/bin/
# script for live segmentation query
cp misc/smafewrapd-live-segmented-file.sh $INSTALLDIR/bin/



echo "copy resource files"
mkdir $INSTALLDIR/res
cp smafestore/smafestore.bootstrap.data.sql $INSTALLDIR/res/
cp smafestore/smafestore.dump* $INSTALLDIR/res/
# copy generated sql file
cp $SQLFILE $INSTALLDIR/res/config.opt.sql

echo "copy doc files"
mkdir $INSTALLDIR/doc
cp doc/doc-deploy/* $INSTALLDIR/doc/

echo "copy system files"
mkdir $INSTALLDIR/system
cp misc/misc-deploy/smafed $INSTALLDIR/system/

echo "copy test files"
mkdir $INSTALLDIR/test
cp -r test/* $INSTALLDIR/test/
# delete those that we do not want to copy
rm $INSTALLDIR/test/testDecrypt.sh			# component not deployed
rm -rf $INSTALLDIR/test/resources/testDecrypt
rm $INSTALLDIR/test/testNorm.sh				# component not deployed
rm $INSTALLDIR/test/testQuery.sh			# component not deployed
rm $INSTALLDIR/test/TestRunner.cpp			# ?
#rm $INSTALLDIR/test/testSMAFE_NODB.sh		# deployment: only db # obsolete
rm $INSTALLDIR/test/testSmuiupdated.sh		# component not deployed
rm -rf $INSTALLDIR/test/resources/testSmuiupdated
rm $INSTALLDIR/test/testWrap_extended*.sh	#
rm -rf $INSTALLDIR/test/resources/testWrap_extended	# config file
rm -rf $INSTALLDIR/test/resources/testWrap_extended_nopass	# config file
rm $INSTALLDIR/test/resources/testDistd_extended/test.opt*   # no optionsfile in plaintext. Generated .sql file not used in test
rm $INSTALLDIR/test/resources/test.opt		# no optoins file in plaintext!
#rm $INSTALLDIR/test/testDistd_live.sh		# no plaintext database dump!
#rm -rf $INSTALLDIR/test/resources/testDistd_live # no plaintext database dump!

# make sure the test.opt.sql exists
if [ ! -f $INSTALLDIR/test/resources/test.opt.sql ]
then
    echo ERROR: $INSTALLDIR/test/resources/test.opt.sql does not exist but is needed for tests on the target machine. Please create test.opt.sql and run the script again. HINT: make check creates the file
    exit 1
fi

# remove .svn
# this provokes some errors, not sure why
find $INSTALLDIR/test -name ".svn" -type d | xargs -0 rm -rf


# hack the test scripts so that they use our wapper scripts to start the
# binaries instead of the (not available) libtool wrapper scripts in build dir
# this command performs in place substition of calls like
# $SMAFETESTEXECPREFIX/smafewrapd/smafewrapd
# to
# ../bin/smafewrapd.sh
# this is done for alle test*.sh scripts in test dir 
sed -i -e 's|\$SMAFETESTEXECPREFIX/\(sm[a-z]*\)/\1|../bin/\1.sh|g' $INSTALLDIR/test/test*.sh
if [ $? != 0 ]; then
  echo "ERROR adapting test scripts"
  exit 1
fi


# not required?
mkdir $INSTALLDIR/smafestore
cp smafestore/smafestore*.sql $INSTALLDIR/smafestore/


echo "copy lib files"
# this is quite system specific - we might want to do it more general at some time!
# 20100721 WJ removed boost, crypto++ extension
# for these we also copy the static compiled lib

# libcrypto++
cp $LIB_FOLDER/libcrypto++* $INSTALLDIR/lib/
# check if the last copy succeeded.
if [ $? != 0 ]; then
  echo "ERROR copying crypto++ libs. Expected in directory $LIB_FOLDER (\$LIB_FOLDER)"
  exit 1
fi

# boost 
cp $BOOST_LIB_FOLDER/libboost_filesystem* $INSTALLDIR/lib/
cp $BOOST_LIB_FOLDER/libboost_serialization* $INSTALLDIR/lib/
cp $BOOST_LIB_FOLDER/libboost_system* $INSTALLDIR/lib/
cp $BOOST_LIB_FOLDER/libboost_wserialization* $INSTALLDIR/lib/
cp $BOOST_LIB_FOLDER/libboost_date_time* $INSTALLDIR/lib/
# check if the last copy succeeded.
# if not, we assume that $BOOST_LIB_FOLDER might not be set correctly
if [ $? != 0 ]; then
  echo "ERROR copying Boost libs. Please check variable \$BOOST_LIB_FOLDER"
  exit 1
fi

cp $MPG123_LIB_FOLDER/libmpg123*$dynlibextension* $INSTALLDIR/lib/
# check if the last copy succeeded.
if [ $? != 0 ]; then
  echo "ERROR copying mpg123 libs. Please check variable \$MPG123_LIB_FOLDER"
  exit 1
fi

# should be installed with the separate postgres package cp /usr/lib/libpq*$dynlibextension* $INSTALLDIR/lib/

cp $LIB_FOLDER/libargtable2*$dynlibextension* $INSTALLDIR/lib/
# check if the last copy succeeded.
if [ $? != 0 ]; then
  echo "ERROR copying argtable libs. Expected in directory $LIB_FOLDER (\$LIB_FOLDER)"
  exit 1
fi

cp $LIBTOOL_FOLDER/libltdl*$dynlibextension* $INSTALLDIR/lib/ 	# libtool
# check if the last copy succeeded.
if [ $? != 0 ]; then
  echo "ERROR copying libtool libs. Expected in directory $LIBTOOL_FOLDER (\$LIBTOOL_FOLDER)"
  exit 1
fi

# ipp is special: we take ipps and ippcore, dispatcher and processor specific libs
cp  $INTEL_IPP_LIB_FOLDER/libipps*$dynlibextension* $INSTALLDIR/lib/
# check if the last copy succeeded.
if [ $? != 0 ]; then
  echo "ERROR copying IPP libs. Please check variable \$INTEL_IPP_LIB_FOLDER"
  exit 1
fi
# removes speech recognition files
rm -f $INSTALLDIR/lib/libippsr* 
cp  $INTEL_IPP_COMPILERLIB_FOLDER/libippcore*$dynlibextension* $INSTALLDIR/lib/

# Actually only one of libguide and libiomp5 is used but
# we do not know at this time which one (eg ubuntu 9.10 systems use
# libguide, Debian 5 system (Hetzner) uses libiomp5)
# So, we copy both.
cp  $INTEL_IPP_COMPILERLIB_FOLDER/libguide*$dynlibextension* $INSTALLDIR/lib/
if [ $? != 0 ]; then
  echo "WARNING: copying libguide failed. If you think this is an error please check \$INTEL_IPP_COMPILERLIB_FOLDER"
  echo "Since this file is not required mkdeploy.sh continues."
fi
# libiomp5
SMAFEDEPLOY_TO_COPY=$INTEL_IPP_COMPILERLIB_FOLDER/libiomp5*$dynlibextension*
if [ -f $SMAFEDEPLOY_TO_COPY ]; then
	cp  $SMAFEDEPLOY_TO_COPY $INSTALLDIR/lib/
	if [ $? != 0 ]; then
  		echo "ERROR copying libiomp5. Please check \$INTEL_IPP_COMPILERLIB_FOLDER"
  		exit 1
	fi
fi
# libsvml.so
SMAFEDEPLOY_TO_COPY=$INTEL_IPP_COMPILERLIB_FOLDER/libsvml*$dynlibextension*
if [ -f $SMAFEDEPLOY_TO_COPY ]; then
	cp  $SMAFEDEPLOY_TO_COPY $INSTALLDIR/lib/
	if [ $? != 0 ]; then
  		echo "ERROR copying libsvml. Please check \$INTEL_IPP_COMPILERLIB_FOLDER"
  		exit 1
	fi
fi
# libimf
SMAFEDEPLOY_TO_COPY=$INTEL_IPP_COMPILERLIB_FOLDER/libimf*$dynlibextension*
if [ -f $SMAFEDEPLOY_TO_COPY ]; then
	cp  $SMAFEDEPLOY_TO_COPY $INSTALLDIR/lib/
	if [ $? != 0 ]; then
  		echo "ERROR copying libimf. Please check \$INTEL_IPP_COMPILERLIB_FOLDER"
  		exit 1
	fi
fi
# libirc
SMAFEDEPLOY_TO_COPY=$INTEL_IPP_COMPILERLIB_FOLDER/libirc*$dynlibextension*
if [ -f $SMAFEDEPLOY_TO_COPY ]; then
	cp  $SMAFEDEPLOY_TO_COPY $INSTALLDIR/lib/
	if [ $? != 0 ]; then
  		echo "ERROR copying libirc. Please check \$INTEL_IPP_COMPILERLIB_FOLDER"
  		exit 1
	fi
fi

# libgomp
# new approach: not a fixed hardcoded file. Instead, we rather check what 
# library is acutally used at runtime. This on is going to be copied. 
SMAFEDEPLOY_TO_COPY=$(ldd $INSTALLDIR/bin/bin-internal/smafewrapd  |grep libgomp |cut -d " " -f 3)
echo "libgomp lib file detected: $SMAFEDEPLOY_TO_COPY"
if [ -f $SMAFEDEPLOY_TO_COPY ]; then
	cp  $SMAFEDEPLOY_TO_COPY $INSTALLDIR/lib/
	if [ $? != 0 ]; then
  		echo "ERROR copying libgomp. Detection of file to copy is based on ldd output of smafewrapd"
  		ldd $INSTALLDIR/bin/bin-internal/smafewrapd
  		exit 1
	fi
fi

# not exactly a lib but external program ffmpeg
# on mac we assume that ffmpeg is one binary (downloaded from http://ffmpegmac.net/)
# on linux system we have smafesetup pull the deb packages on the client side
# since there is a bunch of deps

# check if mac
if [ -n "$SMAFESETUP_ISMAC" ]; then
	echo "copy ffmpeg"
	# get location of ffmpeg
	MKDEPLOY_FFMPEG_BINARY=$(which ffmpeg)
	# check if found
	if [ -n "$MKDEPLOY_FFMPEG_BINARY" ]; then
		# yes, so copy
		SMAFEDEPLOY_TO_COPY="$MKDEPLOY_FFMPEG_BINARY"
		if [ -f "$SMAFEDEPLOY_TO_COPY" ]; then
			# to BIN subfolder!
			cp  "$SMAFEDEPLOY_TO_COPY" "$INSTALLDIR/bin/"
			if [ $? != 0 ]; then
  				echo "ERROR copying ffmpeg"
	  			exit 1
			fi
			echo "ffmpeg copied"
		fi
	else
		echo "WARNING: ffmpeg binary not found"
	fi
fi #[ -n "$SMAFESETUP_ISMAC" ]; then



## drag along the smafe daemons params
# check if these param lines are in the options file 
egrep "^###_smafewrapd-params"  $OPTFILE > /dev/null
EXCODE1=$?
# rmember: grep returns 0 if found
if [ $EXCODE1 -eq 0 ]; then
	echo Extracting daemon params from options file
	# copy these lines into a new file for target machine use
	# pipe through fromdos: remove windows line endings
	egrep "###_smafe.*-params" $OPTFILE | tr -d '\r'  > $INSTALLDIR/res/smafed.conf-input.txt
else
	echo create standard parameters
	echo "###_smafewrapd-params --id=smafedbwrap" > $INSTALLDIR/res/smafed.conf-input.txt
	echo "###_smafedistd-params --id=smafedbdist" >> $INSTALLDIR/res/smafed.conf-input.txt
fi
	

### SMINT API
echo "preparing SMINT API files."
echo "NOTE: The standard configuration file template 'smintapi/config/smintapi.ini.template' is used."

mkdir -p $INSTALLDIR/smintapi/lib
cp smintapi/lib/* $INSTALLDIR/smintapi/lib/
mkdir $INSTALLDIR/smintapi/log
mkdir $INSTALLDIR/smintapi/web
# copy only smintapi frontend controller (not liveapi)
cp smintapi/web/smintapi.php $INSTALLDIR/smintapi/web/
mkdir $INSTALLDIR/smintapi/config
cp smintapi/config/smintapi.ini.template $INSTALLDIR/smintapi/config
mkdir -p $INSTALLDIR/smintapi/test/script-tests
# copy only script tests, not phpunit tests
cp smintapi/test/script-tests/* $INSTALLDIR/smintapi/test/script-tests/

### live api
# Precondition: SMINT API must be packaged.
if [ $OPT_liveapi -ne 0 ]; then
	echo "Including live API..."
	cp smintapi/web/liveapi.php $INSTALLDIR/smintapi/web/
	cp smintapi/config/liveapi.ini.template $INSTALLDIR/smintapi/config
else
	echo "Live API not included in deploy package."
fi


### version
echo Creating VERSION file
echo > $INSTALLDIR/VERSION
echo $(grep PACKAGE_VERSION $BUILDDIR/config.h |cut -d " " -f 3)  >> $INSTALLDIR/VERSION
echo $(uname -a) 				>> $INSTALLDIR/VERSION
if [ -f /etc/issue ]; then
    cat /etc/issue >> $INSTALLDIR/VERSION
else
	echo "/etc/issue not found"
fi



echo "create tar with necessary files"
# save pwd
PWDSAVE=$(pwd)
cd $INSTALLDIR
rm -f smafedeploy-libs.tar.bz2
rm -f smafedeploy.tar.bz2
tar -cj --remove-files --exclude=lib/libsmafe* --exclude=lib/libipp* -f smafedeploy-libs.tar.bz2 lib doc/* 
tar -cj --remove-files --exclude=smafedeploy-libs.tar.bz2 -f smafedeploy.tar.bz2 * 
cd $PWDSAVE


# copy setup script
cp misc/misc-deploy/smafesetup.sh $INSTALLDIR
# copy install texst
cp misc/misc-deploy/INSTALL $INSTALLDIR


### rename the install folder to reflect options file and revision
# build name: svn revision from config.h, the optfile. Then sanitize the string.
SVNREV=$(grep PACKAGE_VERSION $BUILDDIR/config.h |cut -d " " -f 3)
SVNREV=${SVNREV//[^a-zA-Z0-9\.\-]/}
INSTALLDIRFINAL=$(pwd)/deploy-packages/$(basename $OPTFILE)-$SVNREV

if [ -d "$INSTALLDIRFINAL" ]; then
	INSTALLDIRFINAL=${INSTALLDIRFINAL}.$$
fi


mv $INSTALLDIR $INSTALLDIRFINAL


### make one archive
#tar -cj --remove-files -f $INSTALLDIR/../smafe.tar.bz2  $INSTALLDIR # not working

# go up
cd misc


echo
echo
echo "Finished: Deploy package has been created in "
echo "      $INSTALLDIRFINAL"
echo "Next, copy the contents of this directory to the target machine and do the setup."


