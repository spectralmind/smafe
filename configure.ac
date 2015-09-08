###########################################################################
## Copyright (c) 2009 spectralmind.com
## All rights reserved.
## ------------------------------------------------------------------------
## configure.ac
##  Created on: 03/2009
##      Author: wolfgang
## ------------------------------------------------------------------------
## SVN Version: $Id$
###########################################################################

# configure.ac is used by autoconf & automake
# see http://www.gnu.org/software/autoconf/manual/autoconf.html#Writing-Autoconf-Input for Details

# First thing to do is call AC_INIT (package, version, [bug-report], [tarname])
AC_INIT([smafe], [1.3], [bugs@spectralmind.com])

# automake macro for cppunit integration
AM_PATH_CPPUNIT(1.12, , AC_MSG_NOTICE([Can not find CPPUNIT. CPPUNIT tests will be skipped. To test CPPUNIT tests intall CPPUNIT!]))
AM_CONDITIONAL(ENABLE_CPPUNIT, test "x$no_cppunit" = x)

# get svn version into package version of autotools 
# you might need to run autoreconf --force --install to update the version, since it may be cached in existing files
define([gitcommithash], esyscmd([sh -c "git rev-parse HEAD"])) 
PACKAGE_VERSION=$PACKAGE_VERSION.gitcommithash
PACKAGE_STRING=$PACKAGE_STRING.gitcommithash
AC_DEFINE_UNQUOTED([PACKAGE_VERSION],["$PACKAGE_VERSION"],"Define to the version of this package - including the commit hash.")
AC_DEFINE_UNQUOTED([PACKAGE_STRING],["$PACKAGE_STRING"],"Define to the full name and version of this package - including the commit hash.")
# This variable must be numeric
#AC_DEFINE(COMMITHASH, "gitcommithash", [GIT commit hash])
echo "Added git commit hash to package version: $PACKAGE_STRING"

# Check the version of Autoconf
AC_PREREQ([2.61])

# Check the version of Libtool
LT_PREREQ([2.2.4])

# set the directory where auxiliary scripts such as install-sh and depcomp should be in 
AC_CONFIG_AUX_DIR([build-aux])
# set the directory where additional local Autoconf macros shall go 
AC_CONFIG_MACRO_DIR([m4])

# Next init AUTOMAKE 
# see http://www.gnu.org/software/automake/manual/automake.html
AM_INIT_AUTOMAKE([-Wall -Werror foreign])   

# Then check what system we are working on: sets host_cpu, host_vendor, host_os
AC_CANONICAL_HOST

# checks for programs
# the check will try to set reasonable compiler options if no other settings exist
# since we do this manually later in the debug section we have to ensure, that CXX / C flags are present 
# thx to http://www.bioinf.uni-freiburg.de/~mmann/HowTo/automake.html#noAutoDebug
# store current user given compiler flags to avoid default setup via AC_PROG_CXX
OLD_CXXFLAGS=$CXXFLAGS
OLD_CFLAGS=$CFLAGS
AC_PROG_CXX # C++ compiler
AC_PROG_CC  # C compiler
# reset compiler flags to initial flags
CXXFLAGS=$OLD_CXXFLAGS
CFLAGS=$OLD_CFLAGS

# set default compiler
AC_LANG([C++])

# check compiler version 
# g++ >= 4.2
WANT_CXX_MAJOR_VERSION=4
WANT_CXX_MINOR_VERSION=2
AX_GXX_VERSION()
AX_SPLIT_VERSION([$GXX_VERSION])
if ( test "$AX_MAJOR_VERSION" -gt $WANT_CXX_MAJOR_VERSION ) || \
   ( test "$AX_MAJOR_VERSION" -ge $WANT_CXX_MAJOR_VERSION &&  test "$AX_MINOR_VERSION" -ge $WANT_CXX_MINOR_VERSION ); then
  AC_MSG_NOTICE([C++ compiler version is ${GXX_VERSION}. Need at least version ${WANT_CXX_MAJOR_VERSION}.${WANT_CXX_MINOR_VERSION}])
else
# try to find newer compiler depending on host system
    AC_MSG_NOTICE([C++ compiler version ${GXX_VERSION} is too old, trying to find newer version .... ])
    case "${host_os}" in
      *linux*)
        ;;
      *mingw32* | *win32*)
        ;;
      *darwin*)
	    AC_MSG_NOTICE([setting CXX to g++-4.2 ])
		CXX=g++-4.2
        ;;
    esac
    AX_GXX_VERSION()
	AX_SPLIT_VERSION([$GXX_VERSION])
	if ( test "$AX_MAJOR_VERSION" -gt $WANT_CXX_MAJOR_VERSION ) || \
	   ( test "$AX_MAJOR_VERSION" -ge $WANT_CXX_MAJOR_VERSION &&  test "$AX_MINOR_VERSION" -ge $WANT_CXX_MINOR_VERSION ); then
	  AC_MSG_NOTICE([C++ compiler version is ${GXX_VERSION}. Need at least version ${WANT_CXX_MAJOR_VERSION}.${WANT_CXX_MINOR_VERSION}])
	else
		AC_MSG_ERROR([C++ compiler version is ${GXX_VERSION}. 
                  Need at least version ${WANT_CXX_MAJOR_VERSION}.${WANT_CXX_MINOR_VERSION}.

                  You can try to 
                  - tell configure what compiler to use e.g. configure CXX=g++-4.2
                  - do the same thing in your enviroment before launching configure export env CXX=g++-4.2
                  - update your compiler
                

])
	fi
fi


##########################################################################
# static linking support
# some libraries like IPP need special treatment when linking as static lib 
# in order to provide propper support use this option instead of just adding -static to the linker options
##########################################################################
#AC_MSG_CHECKING([whether to link as static objects])
#AC_ARG_ENABLE([static],
#    [AS_HELP_STRING([--enable-static],
#        [enable static linking (default=no)])],
#    [staticlinking="$enableval"],
#    [staticlinking=no])
#AC_MSG_RESULT([$staticlinking])
#if test x"$staticlinking" = x"yes"; then
#    AC_DEFINE([STATICLINKED], 1, [static linking status])
#    LDFLAGS="-static $LDFLAGS"
#else
#    AC_DEFINE([STATICLINKED], 0, [static linking status])
#fi
##########################################################################



# add doxygen support
DX_INIT_DOXYGEN($PACKAGE_NAME, doxygen.cfg)

# init Libtool 
LT_INIT

# option to enable building of optional components like the postgres distance functions /smafedist
#AC_ARG_ENABLE(postgres_smafedist,
#  AS_HELP_STRING([--enable-postgres_smafedist], [Also build smafedist (function for postgres to calculate distance.)]),
#  [enable_postgres_smafedist=$enableval], 
#  [enable_postgres_smafedist=no])
#AM_CONDITIONAL(ENABLE_POSTGRES_SMAFEDIST, test x"$enable_postgres_smafedist" = xyes)

# set preprocessor settings 
AC_MSG_NOTICE([Setting compile options.])
  # database driver settings
  DEFAULT_SMAFE_DBDRIVER=SMAFE_PQ
  AC_ARG_VAR([SMAFE_DBDRIVER], [Set database driver for smafe [SMAFE_PQ|SMAFE_SL]. Default is SMAFE_PQ])
    if test "$SMAFE_DBDRIVER" = "SMAFE_PQ" || \
       test "$SMAFE_DBDRIVER" = "SMAFE_SL"; then
      SMAFE_DBDRIVER=$SMAFE_DBDRIVER
    else
      SMAFE_DBDRIVER=$DEFAULT_SMAFE_DBDRIVER
    fi
    CPPFLAGS="-D$SMAFE_DBDRIVER $CPPFLAGS"
    AC_MSG_NOTICE([using SMAFE_DBDRIVER: $CPPFLAGS])


  # passphrase
#  DEFAULT_SMAFE_PASSPHRASE=
#  AC_ARG_VAR([SMAFE_PASSPHRASE], [Set passphrase for encryption. Default is no passphrase])
#    if test x"$SMAFE_PASSPHRASE" != x"";   then
#      AC_DEFINE_UNQUOTED([SMAFE_PASSPHRASE], ["$SMAFE_PASSPHRASE"], [passphrase])
#      AC_MSG_NOTICE([using SMAFE_PASSPHRASE: $SMAFE_PASSPHRASE])
#    else
#      AC_MSG_NOTICE([not using any passphrase: no encryption])
#    fi
  

###############################################################################
###############################################################################
#
# check for library and header path names
#
###############################################################################
###############################################################################

AC_MSG_NOTICE([Setting folders for inlude & library path folders.])
  # set default search path
  DEFAULT_POSTGRES_INCLUDE_FOLDER="/usr/include/postgresql"
  DEFAULT_POSTGRES_SERVER_INCLUDE_FOLDER="/usr/include/postgresql/8.3/server"
  DEFAULT_INTEL_IPP_FOLDER="/opt/intel/ipp/5.3.4.080/em64t"
  DEFAULT_INTEL_IPP_LIB_FOLDER="$DEFAULT_INTEL_IPP_FOLDER/lib"
  DEFAULT_CRYPTOPP_INCLUDE_FOLDER="/usr/include/cryptopp"
  
  # set configure args
  AC_ARG_VAR([POSTGRES_INCLUDE_FOLDER], [Set folder where smafe finds the postgres headers.])
  AC_ARG_VAR([POSTGRES_SERVER_INCLUDE_FOLDER], [Set folder where smafe finds the postgres server headers.])
  AC_ARG_VAR([INTEL_IPP_FOLDER], [Set folder where smafe finds INTEL IPP.])
  AC_ARG_VAR([INTEL_IPP_LIB_FOLDER], [(Optional. Mostly for non linux systems.) Set folder where smafe finds the IPP libs.])
  # the following args are needed if the libs are not in the default locations 
  # where they would be found from the linker without additional library path
  AC_ARG_VAR([BOOST_LIB_FOLDER], [(Optional. Mostly for non linux systems.) Set folder where smafe finds the boost libs.])
  AC_ARG_VAR([BOOST_INCLUDE_FOLDER], [(Optional. Mostly for non linux systems.) Set folder where smafe finds the boost headers.])
  AC_ARG_VAR([ARGTABLE2_LIB_FOLDER], [(Optional. Mostly for non linux systems.) Set folder where smafe finds the ARGTABLE2 libs.])
  AC_ARG_VAR([ARGTABLE2_INCLUDE_FOLDER], [(Optional. Mostly for non linux systems.) Set folder where smafe finds the ARGTABLE2 headers.])
  AC_ARG_VAR([POSTGRES_LIB_FOLDER], [(Optional. Mostly for non linux systems.) Set folder where smafe finds the postgres libs.])
  AC_ARG_VAR([CRYPTOPP_INCLUDE_FOLDER], [(Optional. Mostly for non linux systems.) Set folder where smafe finds the cryptopp headers. ])
  AC_ARG_VAR([CRYPTOPP_LIB_FOLDER], [(Optional. Mostly for non linux systems.) Set folder where smafe finds the cryptopp libs. ])


    
  
  # set default path if no args where given 
  # postgres headers
    if test -z "$POSTGRES_INCLUDE_FOLDER"; then
      POSTGRES_INCLUDE_FOLDER=$DEFAULT_POSTGRES_INCLUDE_FOLDER
    fi
  # postgres server headers
    if test -z "$POSTGRES_SERVER_INCLUDE_FOLDER"; then
      POSTGRES_SERVER_INCLUDE_FOLDER=$DEFAULT_POSTGRES_SERVER_INCLUDE_FOLDER
    fi
  # INTEL IPP folder
    if test -z "$INTEL_IPP_FOLDER"; then
      INTEL_IPP_FOLDER=$DEFAULT_INTEL_IPP_FOLDER
    fi
  # IPP LIB folder
    if test -z "$INTEL_IPP_LIB_FOLDER"; then
      INTEL_IPP_LIB_FOLDER=$DEFAULT_INTEL_IPP_LIB_FOLDER
    fi
    if test -n "$INTEL_IPP_LIB_FOLDER"; then
      LDFLAGS="-L$INTEL_IPP_LIB_FOLDER $LDFLAGS"  
      LD_LIBRARY_PATH="$INTEL_IPP_LIB_FOLDER:$LD_LIBRARY_PATH"
      AC_MSG_NOTICE([using INTEL_IPP_LIB_FOLDER: $INTEL_IPP_LIB_FOLDER])
    fi

  # CRYPTOPP_INCLUDE_FOLDER
    if test -z "$CRYPTOPP_INCLUDE_FOLDER"; then
      CRYPTOPP_INCLUDE_FOLDER=$DEFAULT_CRYPTOPP_INCLUDE_FOLDER
    fi
  # the following libs are added to the linker options if they are not in the default location
  # BOOST LIB folder
    if test -n "$BOOST_LIB_FOLDER"; then
      LDFLAGS="-L$BOOST_LIB_FOLDER $LDFLAGS"  
      LD_LIBRARY_PATH="$BOOST_LIB_FOLDER:$LD_LIBRARY_PATH"
      AC_MSG_NOTICE([using BOOST_LIB_FOLDER: $BOOST_LIB_FOLDER])
    fi
  # BOOST INCLUDE folder
    if test -n "$BOOST_INCLUDE_FOLDER"; then
      CPPFLAGS="-I$BOOST_INCLUDE_FOLDER $CPPFLAGS"  
      AC_MSG_NOTICE([using BOOST_INCLUDE_FOLDER: $BOOST_INCLUDE_FOLDER])
    fi
  # ARGTABLE2 LIB folder
    if test -n "$ARGTABLE2_LIB_FOLDER"; then
      LDFLAGS="-L$ARGTABLE2_LIB_FOLDER $LDFLAGS"  
      AC_MSG_NOTICE([using ARGTABLE2_LIB_FOLDER: $ARGTABLE2_LIB_FOLDER])
    fi
  # ARGTABLE2 INCLUDE folder
    if test -n "$ARGTABLE2_INCLUDE_FOLDER"; then
      CPPFLAGS="-I$ARGTABLE2_INCLUDE_FOLDER $CPPFLAGS"  
      AC_MSG_NOTICE([using ARGTABLE2_INCLUDE_FOLDER: $ARGTABLE2_INCLUDE_FOLDER])
    fi
  # POSTGRES LIB folder
    if test -n "$POSTGRES_LIB_FOLDER"; then
      LDFLAGS="-L$POSTGRES_LIB_FOLDER $LDFLAGS" 
      AC_MSG_NOTICE([using POSTGRES_LIB_FOLDER: $POSTGRES_LIB_FOLDER])
    fi  
  # CRYPTOPP_INCLUDE_FOLDER 
  	if test -n "$CRYPTOPP_INCLUDE_FOLDER"; then
      CPPFLAGS="-I$CRYPTOPP_INCLUDE_FOLDER $CPPFLAGS"  
      AC_MSG_NOTICE([using CRYPTOPP_INCLUDE_FOLDER: $CRYPTOPP_INCLUDE_FOLDER])
    fi  
  # CRYPTOPP LIB folder
    if test -n "$CRYPTOPP_LIB_FOLDER"; then
      LDFLAGS="-L$CRYPTOPP_LIB_FOLDER $LDFLAGS" 
      AC_MSG_NOTICE([using CRYPTOPP_LIB_FOLDER: $CRYPTOPP_LIB_FOLDER])
    fi  
    
  # print info what folders are used
    AC_MSG_NOTICE([using POSTGRES_INCLUDE_FOLDER: $POSTGRES_INCLUDE_FOLDER])
    AC_MSG_NOTICE([using POSTGRES_SERVER_INCLUDE_FOLDER: $POSTGRES_SERVER_INCLUDE_FOLDER])
    AC_MSG_NOTICE([using INTEL_IPP_FOLDER: $INTEL_IPP_FOLDER])
    AC_MSG_NOTICE([using CRYPTOPP_INCLUDE_FOLDER: $CRYPTOPP_INCLUDE_FOLDER])
  

# Only works with config.h included in smafequery
# (OLD just left here to see how to define preprocessor variable) set SMAFE_PQ for smafequery 
# AC_DEFINE([SMAFE_PQ],[1],[Defines database driver for smafequery.])

# checks for libraries 
# cppunit is not included yet
# AC_CHECK_LIB([cppunit],[exit],,[echo "this program needs libcppunit in order to work"; exit 1])
AC_CHECK_LIB([qttestrunner],[exit], [LIBS="-lqttestrunner  $LIBS"; AC_DEFINE([QT_TEST_RUNNER],[1],[Set 1 if you want to use the Qt Test Runner of CppUnit.]) ] )

AC_CHECK_LIB([pq],[exit],[echo "found"],[echo "this program needs postrges library pq in order to work"; exit 1])
AC_CHECK_LIB([mpg123],[main],[echo "found"],[echo "this program needs mpg123 library in order to work"; exit 1])
AC_CHECK_LIB([argtable2],[main],[echo "found"],[echo "this program needs argtable2 library in order to work"; exit 1])

# check for boost libs and if the -mt suffix for the multi threaded versions should be added
# note that these checks actually also would be satisfied with the static lib .a file!
# Also the first two lines are quite misleading as the second one is successful even if we have NO suffix!
AC_CHECK_LIB([boost_system],[main],BOOSTSUFFIX=; [echo "found BOOST libs without -mt suffix ;-)"], BOOSTSUFFIX=-mt)
AC_CHECK_LIB([boost_system$BOOSTSUFFIX],[main],[echo "found BOOST libs with -mt suffix ;-)"], [echo "Can not find BOOST libraries! This program needs boost library boost_system in order to work. Install libraries and set path."; exit 1])
AC_CHECK_LIB([boost_serialization$BOOSTSUFFIX],[main],[echo "found"], [echo "Can not find BOOST libraries! This program needs boost library boost_serialization in order to work. Install libraries and set path."; exit 1])
AC_CHECK_LIB([boost_filesystem$BOOSTSUFFIX],[exit],[echo "found"],[echo "this program needs boost library boost_filesystem in order to work"; exit 1])
AC_SUBST(BOOSTSUFFIX)

# The default installation directory is: 
# LINUX:
# /opt/intel/ipp/6.0.x.xxx/ia32 ( IA-32 based system) 
# /opt/intel/ipp/6.0.x.xxx/em64t (Intel 64 based system) 
# /opt/intel/ipp/6.0.x.xxx/itanium (Intel Itanium based system) 
# /opt/intel/ipp/6.0.x.xxx/lp32 (Intel Atom Processor based system) 
# WINDOWS:
# C:\Program Files\Intel\IPP\6.0.x.xxx\ia32 ( IA-32 based system)  
# C:\Program Files\Intel\IPP\6.0.x.xxx\em64t (Intel 64 based system) 
# C:\Program Files\Intel\IPP\6.0.x.xxx\itanium (Intel Itanium based system) 
# MAC OS-X:
# /Library/Frameworks/Intel_IPP.framework/Versions/6.0.x.xxx/ia32
# /Library/Frameworks/Intel_IPP.framework/Versions/6.0.x.xxx/em64t

  #store current LDFLAGS
    _savedldflags=${LDFLAGS}
  # INTEL IPP
    # for more infos on ipp take a look at: 
     # linking: http://software.intel.com/en-us/articles/intel-integrated-performance-primitives-intel-ipp-intel-ipp-linkage-models-quick-reference-guide/
     # win:http://software.intel.com/en-us/articles/intel-integrated-performance-primitives-intel-ipp-for-windows-deploying-applications-with-intel-ipp-dlls/
     # linux: http://software.intel.com/en-us/articles/how-to-build-ipp-application-in-linux-environment/
     # mac: http://software.intel.com/en-us/articles/intel-integrated-performance-primitives-for-mac-os-how-to-link-to-the-intel-ipp-for-mac-os-x-libraries-in-application/
      # Add additional search path for ipp libraries 
      # first check for 32 bit Version then for 64 bit Version
      # win/linux/mac have different path to shared libs and some different dependencies; e.g. -pthread is only needed on linux/POSIX systems
      
    case "${host_os}" in
      *linux*)
#        IPP_SHAREDLIB_PATH=sharedlib/ # obsolete since INTEL_IPP_LIB_FOLDER
        IPP_STATICLIB_PATH=lib/
#        IPP_LDFLAGS="-pthread"
		CPPFLAGS="$CPPFLAGS -pthread"
        # to find shared libs you have to define the LD_LIBRARY_PATH see: http://www.gnu.org/software/gsl/manual/html_node/Shared-Libraries.html
#        LD_LIBRARY_PATH="$INTEL_IPP_FOLDER/$IPP_SHAREDLIB_PATH:$LD_LIBRARY_PATH" # has alerady been added before
        echo "LD_LIBRARY_PATH: $LD_LIBRARY_PATH"
        ;;
      *darwin*)
#        IPP_SHAREDLIB_PATH=Libraries/
        IPP_STATICLIB_PATH=lib/
        # include intel compiler libs otherwise the compiler misses libiomp5
        IPP_LDFLAGS=-L$INTEL_IPP_FOLDER/../../lib/
        ;;
      *mingw32* | *win32*)
#        IPP_SHAREDLIB_PATH=stublib/
        IPP_STATICLIB_PATH=lib/
        IPP_LDFLAGS= 
        ;;
      *)
        AC_MSG_ERROR([Can not decide what system you are running on! Supported are linux, darwin, mingw32, win32 you have: ${host_os}]); 
        ;;        
    esac
    
    echo [Supported Host System found: ${host_os}]

#	if test x"$staticlinking" = x"yes"; then
#	    LDFLAGS=" -L$INTEL_IPP_FOLDER/$IPP_STATICLIB_PATH $IPP_LDFLAGS $LDFLAGS"  
#	    echo [Checking IPP 4 static linking ...]
#	    AC_CHECK_LIB([ippcore],[main],IPPSUFFIX=; [echo "found IPP IA32 Version ;-)"], IPPSUFFIX=em64t)
#	    AC_CHECK_LIB([ippcore$IPPSUFFIX],[main], [echo "found IPP em64t Version ;-)"], [echo "Can not find IPP libraries! This program needs the INTEL IPP libraries in order to work. Install libraries and set path."; exit 1])
#	    AC_CHECK_LIB([ippsemerged$IPPSUFFIX],[main],[echo "found IPPs ;-)"],[echo "Can not find IPPs libraries! This program needs the INTEL IPP libraries in order to work. Install libraries and set path."; exit 1])
#	    AC_CHECK_LIB([ippsmerged$IPPSUFFIX],[main],[echo "found IPPs ;-)"],[echo "Can not find IPPs libraries! This program needs the INTEL IPP libraries in order to work. Install libraries and set path."; exit 1])
#	    
#	    IPP_IPPS_LIBS=" -lippsemerged$IPPSUFFIX -lippsmerged$IPPSUFFIX"
#	    IPP_CORE_LIBS=" -lippcore$IPPSUFFIX" 
#	    IPP_LIB_PATH=$IPP_STATICLIB_PATH
#	else
	    echo [Checking IPP 4 dynamic linking ...]
	    # since macro does not know about shared libs, add -shared -fPIC and folders to linker flags
	    #  -shared -fPIC
	    # -R$INTEL_IPP_FOLDER/$IPP_SHAREDLIB_PATH
#	    LDFLAGS=" -L$INTEL_IPP_FOLDER/$IPP_SHAREDLIB_PATH $IPP_LDFLAGS $LDFLAGS"  
	    LDFLAGS=" -L$IPP_LDFLAGS $LDFLAGS"  
	      
	    AC_CHECK_LIB([ippcore],[main],IPPSUFFIX=; [echo "found IPP IA32 Version ;-)"], IPPSUFFIX=em64t)
	    AC_CHECK_LIB([ippcore$IPPSUFFIX],[main], [echo "found IPP em64t Version ;-)"], [echo "Can not find IPP libraries! This program needs the INTEL IPP libraries in order to work. Install libraries and set path."; exit 1])
	    AC_CHECK_LIB([ipps$IPPSUFFIX],[main],[echo "found IPPs ;-)"],[echo "Can not find IPPs libraries! This program needs the INTEL IPP libraries in order to work. Install libraries and set path."; exit 1])

	    IPP_IPPS_LIBS="-lipps$IPPSUFFIX"
	    IPP_CORE_LIBS=" -lippcore$IPPSUFFIX" 
	    IPP_LIB_PATH=$IPP_SHAREDLIB_PATH
#	fi
      
#    AC_SUBST(IPP_SHAREDLIB_PATH)
#    AC_SUBST(IPP_STATICLIB_PATH)
#    AC_SUBST(IPP_LIB_PATH) # todo alternative
	AC_SUBST(IPP_IPPS_LIBS)
	AC_SUBST(IPP_CORE_LIBS)
    AC_SUBST(IPP_LDFLAGS)
#    AC_SUBST(IPPSUFFIX)
  # restore original LDFLAGS
    LDFLAGS=$_savedldflags

###############################################################################
###############################################################################
#
# check for specific library versions
#
###############################################################################
###############################################################################

##########################################################################
# version check 
# some libraries are checked for their version using a package manager, 
# if the library was not installed using a package manager this will result in a version conflict
# this options allows disabling the version check that depends on the package manager
##########################################################################
AC_ARG_ENABLE([checkpkgversion],
    [AS_HELP_STRING([--disable-checkpkgversion],
        [disable check of versions via package manager])],
    [checkpkgversion="$enableval"],
    [checkpkgversion=yes])
##########################################################################

if test x"$checkpkgversion" = x"yes"; then
  # Check for libmpg123 - package pkg-config needed 
  PKG_CHECK_MODULES(LIBMPG123, libmpg123 >= 1.10)  
fi

# Check for postgres / pq 
# no direct way known -> checking via pg_config --version would be possible if needed

# Check for argtable version
# no way known to check version 

# Boost check for version >= 1.47 (see version.hpp for format) 
WANT_BOOST_VERSION=104700
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
@%:@include <boost/version.hpp>
]], [[
#if BOOST_VERSION >= $WANT_BOOST_VERSION
// Everything is okay
#else
#  error Boost version is too old
#endif
]])],[
    AC_MSG_RESULT([checking for Boost Version >= 1.47... yes])
   	],[
	AC_MSG_ERROR([Boost Version >= 1.47 needed ! The wrong version was found on this system ! ]) 
   	])
   	
# IPP check for version > 5.3.4 (see ippversion.h for format) 
WANT_IPP_VERSION_MAJOR=5
WANT_IPP_VERSION_MINOR=3
WANT_IPP_VERSION_BUILD=4

_cppflags=${CPPFLAGS}
# Add additional search path for ipp headers 
CPPFLAGS="-I$INTEL_IPP_FOLDER/include $CPPFLAGS"  
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
@%:@include <ippversion.h>
]], [[
#if IPP_VERSION_MAJOR > $WANT_IPP_VERSION_MAJOR || \
IPP_VERSION_MAJOR >= $WANT_IPP_VERSION_MAJOR && IPP_VERSION_MINOR > $WANT_IPP_VERSION_MINOR || \
IPP_VERSION_MAJOR >= $WANT_IPP_VERSION_MAJOR && IPP_VERSION_MINOR >= $WANT_IPP_VERSION_MINOR && IPP_VERSION_BUILD >= $WANT_IPP_VERSION_BUILD
// Everything is okay
#else
#  error IPP version is too old
#endif
]])],[
    AC_MSG_RESULT([checking for IPP Version >= ${WANT_IPP_VERSION_MAJOR}.${WANT_IPP_VERSION_MINOR}.${WANT_IPP_VERSION_BUILD}... yes])
   	],[
	AC_MSG_ERROR([IPP Version >= ${WANT_IPP_VERSION_MAJOR}.${WANT_IPP_VERSION_MINOR}.${WANT_IPP_VERSION_BUILD} needed ! The wrong version was found on this system ! ]) 
   	])
# restore original CPPFLAGS
CPPFLAGS=$_cppflags


###############################################################################
###############################################################################
#
# checks for headers
#
###############################################################################
###############################################################################

  #store current CPPFLAGS
    _cppflags=${CPPFLAGS}
  # POSTGRES
      # Add additional search path for postgres headers 
      CPPFLAGS="-I$POSTGRES_INCLUDE_FOLDER $CPPFLAGS" 
    # check postgres headers 
    AC_CHECK_HEADER(libpq-fe.h,,AC_MSG_ERROR(postgres headers can't be found. install postgres headers and set path with 'configure POSTGRES_INCLUDE_FOLDER=/usr/include/postgresql' . ) )  
    
  # BOOST (just check for one, the others should be in place)
    AC_CHECK_HEADER(boost/shared_ptr.hpp,,AC_MSG_ERROR(boost headers can't be found. install boost headers and set path with 'configure BOOST_INCLUDE_FOLDER=/local/src/boost_1_35_0/'. ) ) 
    
  # ARGTABLE 2 
    AC_CHECK_HEADER(argtable2.h,,AC_MSG_ERROR(ARGTABLE 2 headers can't be found. install ARGTABLE 2 headers and set path with 'configure ARGTABLE2_INCLUDE_FOLDER=/argtable/lib/'. ) )  

  # MPG123
  # check mpg123 headers 
  #store current CPPFLAGS
   _cppflags=${CPPFLAGS}
  # Add large file support for check 
  CPPFLAGS="-D_FILE_OFFSET_BITS=64 $CPPFLAGS" 
  AC_CHECK_HEADER(mpg123.h,,AC_MSG_ERROR(mpg123 headers can't be found. install mpg123 headers. ) ) 
  # restore original CPPFLAGS
  CPPFLAGS=$_cppflags
    
  # INTEL IPP
      # Add additional search path for ipp headers 
      CPPFLAGS="-I$INTEL_IPP_FOLDER/include $CPPFLAGS"  
    # check ipp headers 
    AC_CHECK_HEADERS(ipp.h ipps.h,,AC_MSG_ERROR(INTEL IPP headers can't be found. install INTEL IPP headers and set path with . ) ) 
  # restore original CPPFLAGS
    CPPFLAGS=$_cppflags
    
  # CRYPTOPP_INCLUDE_FOLDER
  # check CRYPTOPP_INCLUDE_FOLDER headers 
  #store current CPPFLAGS
   _cppflags=${CPPFLAGS}
  # Add large file support for check 
  CPPFLAGS="-I$CRYPTOPP_INCLUDE_FOLDER $CPPFLAGS" 
  AC_CHECK_HEADER(default.h,,AC_MSG_ERROR(Crypto++ headers ie default.h cannot be found. ) ) 
  # restore original CPPFLAGS
  CPPFLAGS=$_cppflags    


# checks for types
# checks for structures
# checks for compiler characteristics
# checks for library functions

# do we have the fork function?
AC_FUNC_FORK



# checks for system services


##########################################################################
# debug compilation support
# Thx to http://www.bioinf.uni-freiburg.de/~mmann/HowTo/automake.html#debug
##########################################################################
AC_MSG_CHECKING([whether to build with debug information])
AC_ARG_ENABLE([debug],
    [AS_HELP_STRING([--enable-debug],
        [enable debug data generation (default=no)])],
    [debugit="$enableval"],
    [debugit=no])
AC_MSG_RESULT([$debugit])
if test x"$debugit" = x"yes"; then
    AC_DEFINE([DEBUG], 1, [DEBUG Status])
    AM_CXXFLAGS="$AM_CXXFLAGS -g -Wall -Wno-uninitialized -O0"
else
    AC_DEFINE([DEBUG], 0, [DEBUG Status])
    AM_CXXFLAGS="$AM_CXXFLAGS -Wall -O3"
fi
##########################################################################
AC_SUBST([AM_CXXFLAGS])



###########################################################################
# check for deploy version

AC_MSG_CHECKING([whether to build deploy version])
AC_ARG_ENABLE([deploy],
    [AS_HELP_STRING([--disable-deploy],
        [do NOT build version ready for deployment (default ... deploy version built)])],
    [deployit="$enableval"],
    [deployit=yes])
AC_MSG_RESULT([$deployit])
if test x"$deployit" = x"yes"; then
    AC_DEFINE([DEPLOY], 1, [deploy Status])
fi
AM_CONDITIONAL(ENABLE_DEPLOY, test x"$deployit" = x"yes")


# Create Config Files AC_CONFIG_FILES (file..., [cmds], [init-cmds])
# all optional files can be processed, since this will only produce the Makefile.in, but will not compile the projects
    AC_CONFIG_FILES([
	Makefile
	smafeutil/Makefile
	smafedec/Makefile
	smafestore/Makefile
	smafeext/Makefile
	smafedist/Makefile
	smafedistd/Makefile
	smafewrapd/Makefile
	smuiupdated/Makefile
	smafequery/Makefile
	smafenorm/Makefile
	smafedecrypt/Makefile
	])
#
# note that this macro does not work with Ubuntu 8.04's automake 1.10.1!
#AM_COND_IF([ENABLE_DEPLOY], [],
#	[
#	# admin version (not deploy)
#	AC_CONFIG_FILES([
#	smafequery/Makefile
#	smafenorm/Makefile
#	])])

# Create HEADER
AC_CONFIG_HEADERS([config.h])

# Last thing to do is create the ouput (config.status)
AC_OUTPUT



##########################################################################
# Display Message
##########################################################################
echo "
****************************************************************
  $PACKAGE_NAME $PACKAGE_VERSION
****************************************************************

  Install path ...................... $prefix
  Supported Host System found........ $host_os
  C++ compiler version .............. ${GXX_VERSION}

  Using Smafe DB Driver ............. $SMAFE_DBDRIVER

  Debugging ......................... $debugit - enable with configure option --enable-debug

  Deployment version  ............... $deployit - disable with configure option --disable-deploy
  
"

#  Static Linking .................... $staticlinking - enable with configure option --enable-static
 
 
if test "x$no_cppunit" = x ; then
  echo "  CPP UNIT Tests enabled ............ yes"
else
  echo "  CPP UNIT Tests enabled ............ no - Install CPP UNIT to run CPP UNIT tests. "
fi


echo "  
  using Folders:
  POSTGRES_INCLUDE_FOLDER ........... $POSTGRES_INCLUDE_FOLDER
  POSTGRES_SERVER_INCLUDE_FOLDER .... $POSTGRES_SERVER_INCLUDE_FOLDER
  
  INTEL_IPP_FOLDER .................. $INTEL_IPP_FOLDER
  INTEL_IPP_LIB_FOLDER .............. $INTEL_IPP_LIB_FOLDER
#  IPP_SHAREDLIB_PATH ................ $IPP_SHAREDLIB_PATH
  IPP_LDFLAGS ....................... '$IPP_LDFLAGS'
  IPPSUFFIX ......................... '$IPPSUFFIX'  
  IPP_IPPS_LIBS ..................... '$IPP_IPPS_LIBS'
  IPP_LIB_PATH ...................... '$IPP_LIB_PATH'
  
  BOOSTSUFFIX ....................... '$BOOSTSUFFIX'
"

# the following libs are added to the linker options if they are not in the default location
if test -n "$BOOST_LIB_FOLDER"; then
  echo "  BOOST_LIB_FOLDER .................. $BOOST_LIB_FOLDER"
fi
if test -n "$BOOST_INCLUDE_FOLDER"; then
  echo "  BOOST_INCLUDE_FOLDER .............. $BOOST_INCLUDE_FOLDER"
fi
if test -n "$ARGTABLE2_LIB_FOLDER"; then
  echo "  ARGTABLE2_LIB_FOLDER .............. $ARGTABLE2_LIB_FOLDER"
fi 
if test -n "$ARGTABLE2_INCLUDE_FOLDER"; then
  echo "  ARGTABLE2_INCLUDE_FOLDER .......... $ARGTABLE2_INCLUDE_FOLDER"
fi
if test -n "$POSTGRES_LIB_FOLDER"; then
  echo "  POSTGRES_LIB_FOLDER ............... $POSTGRES_LIB_FOLDER"
fi  


# just an empty line
echo

if test x"$debugit" = x"yes"; then
	echo "  CFLAGS='$CFLAGS'"
	echo "  CPPFLAGS='$CPPFLAGS'"
	echo "  CXXFLAGS='$CXXFLAGS'"
	echo "  LDFLAGS='$LDFLAGS'"
	echo "  LIBS='$LIBS'"
	echo "  AM_CXXFLAGS='$AM_CXXFLAGS'"
else
    echo "  Next type 'make' and then 'make install'."
fi

# just an empty line
echo




