# this script helps you build smafe

# first create the configuration scripts
# -f to force overwriting the svnversion macro (otherwise we always get the old version of the macro from the SVN which does not reflect the current revision)
autoreconf -f

# enter the build folder
cd build 


# now set the options of the smafe compile environment
# either add your settings to this shell script or append the settings to your .bash_profile 
# after the settings are in your permanent environment you just need to run ../configure from the build folder


# postgre path settings, might be unnecessary if you are working on linux, since they are in the default include/lib path
# export env POSTGRES_INCLUDE_FOLDER=/c/Programme/PostgreSQL/8.3/include
# export env POSTGRES_SERVER_INCLUDE_FOLDER=/c/Programme/PostgreSQL/8.3/include/server
# export env POSTGRES_LIB_FOLDER=/c/Programme/PostgreSQL/8.3/lib

# intel ipp settings
# export env INTEL_IPP_FOLDER=/c/Programme/Intel/IPP/6.0.2.074/ia32

# boost settings, might be unnecessary if you are working on linux, since they might be in the default include/lib path
# export env BOOST_LIB_FOLDER=/local/src/boost_1_35_0/stage/lib
# export env BOOST_INCLUDE_FOLDER=/local/src/boost_1_35_0/

# argtable 2 settings, , might be unnecessary if you are working on linux, since they might be in the default include/lib path
# export env ARGTABLE2_LIB_FOLDER=/home/nig/smafe/lib/argtable/win32/
# export env ARGTABLE2_INCLUDE_FOLDER=/home/nig/smafe/lib/argtable/include/

# set path where to find dynamic libraries 
# export env LD_LIBRARY_PATH=/opt/intel/ipp/6.0.1.071/em64t/sharedlib/:/home/wolfgang/smafe/boost_1_35_0/stage/lib/


# now after all settings were made run configure
../configure
# and make the project
make 