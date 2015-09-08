#!/bin/sh

# Assumptions
#	- Compiled executables are in subfolder bin-internal
#	- Libs folder is ../lib

# the name of this script must be the same as the real binary, appended with .sh
appname=bin-internal/`basename $0 | sed s,\.sh$,,`

# appname is first argument
#appname=$1

dirname=`dirname $0`
tmp="${dirname#?}"

if [ "${dirname%$tmp}" != "/" ]; then
dirname=$PWD/$dirname
fi

# add the lib path for ld to look for
LD_LIBRARY_PATH=$dirname/../lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH

# start app
$dirname/$appname "$@"
