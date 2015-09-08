#!/bin/bash

#///////////////////////////////////////////////////////////////////////////
#//
#// Copyright (c) 2011 spectralmind
#// All rights reserved.
#//
#// ------------------------------------------------------------------------
#// smafewrapd-live-segmented-file.sh
#//
#// SpectralMind Audio Feature Extraction Wrapper
#// Helper script for segmented input files for live scenario
#// ------------------------------------------------------------------------
#//
#//
#// Version $Id$
#//
#//
#//
#//
#//
#//
#///////////////////////////////////////////////////////////////////////////


# //////////////////////// 
# // USAGE / Parameters

# if less than x argument is provided give usage
if [ $# -lt 3 ]; then
  echo "USAGE: $0 --livebegintime <BEGIN_TIME> --liveendtime <END_TIME> --livefile <audio file> <other smafewrapd parameters>"
  echo "	where TIMEs are in the format minutes.seconds[.hundredths]"
  exit 2
fi


# loop through parameters and find --livefile
#PREV="$0"
#for item in "${@:1}" ; do
#	echo $PREV
#	echo $item
#	echo "--"
#	if [ x"$PREV" == x"--livefile" ]; then
#		export LIVEFILE="$item"
##      	echo "found"
#	fi
#	PREV=$item
#done


# extract 3 relevant options!
# NO CHECKING DONE so far!
export livebegintime="$2"
export liveendtime="$4"
export LIVEFILE="$6"
export livefilesnippetpath=/tmp/

extension=${LIVEFILE##*.}
SNIPPET_TEMP=sminttmp


# new filename
# /tmp/ dir, temporary filename, a "1" (appended becuase it is the split file number 1), a dot, and the original extension
LIVEFILE_new="${livefilesnippetpath}"${SNIPPET_TEMP}1.${extension}
# alternativ filename since mp3splt 2.1 creates a 001 for the first file (not 1)
LIVEFILE_new_2="${livefilesnippetpath}"${SNIPPET_TEMP}001.${extension}

echo "Parameter used: " 			>&2
echo "File:		$LIVEFILE"		>&2
echo "File snippet:	$LIVEFILE_new"		>&2
echo "Begin time:	$livebegintime"		>&2
echo "End time:	$liveendtime"			>&2



# //////////////////////// 
# // split

# TODO: other directory?!
echo						>&2
echo "mp3splt -f $LIVEFILE  $livebegintime $liveendtime -o ${SNIPPET_TEMP}@n -d $livefilesnippetpath >&2"
echo						>&2
mp3splt -f "$LIVEFILE"  "$livebegintime" "$liveendtime" -o "${SNIPPET_TEMP}@n" -d $livefilesnippetpath >&2
if [ $? -ne 0 ]; then
	echo "ERROR when splitting the file."	>&2
	exit 1
fi


# // check what files are here
if [ ! -f "$LIVEFILE_new"  ]; then
	echo "$LIVEFILE_new does not exist, we try $LIVEFILE_new_2" >&2
	LIVEFILE_new="$LIVEFILE_new_2"
fi



# //////////////////////// 
# // smafewrapd

# get dir of this file and use it for calling smafewrapd
# - unset CDPATH so that that variable does not make trouble
# - cd into the directory that this file lies in (dirname returns the path portion of a file)
# - assign pwd outcome to normalDir
normalDir=`unset CDPATH && cd "$(dirname $0)" && pwd`

# ${@:7} means all command line params starting from the 7th (skip the first 6 as they are processed earlier)
echo Executing: "${normalDir}"/smafewrapd.sh --livefile "$LIVEFILE_new" ${@:7} >&2
${normalDir}/smafewrapd.sh --live --no-daemon --livefile "$LIVEFILE_new" "${@:7}"
if [ $? -ne 0 ]; then
	echo "ERROR when executing smafewrapd."	>&2
	exit 1
fi


# //////////////////////// 
# // delete tmp file
echo "Executing: rm  $LIVEFILE_new" >&2
rm "$LIVEFILE_new"

