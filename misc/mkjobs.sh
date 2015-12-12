#!/bin/bash

#  creates sql file that inserts addfile jobs into database
#  USAGE: mkjobs.sh \<absolute path to mp3 files\>
#  Path will be searched recursively
#  New: Now also handles ' in filenames! :-)


# if less than 1 argument is provided give usage
if [ $# -lt 1 ]; then
  echo "USAGE: $0 <directory with mp3 files>"
  exit 2
fi

TMPFILE=_tmpmkjobs
rm -f $TMPFILE
find "$1" -name "*.mp3" -print > $TMPFILE

cat $TMPFILE | while read key; do
     echo "insert into smafejob_addfile (file_uri, external_key, guid) VALUES('$(echo "$key" | sed "s/'/''/g")', '$(echo "$key" | sed "s/'/''/g")', md5('$(echo "$key" | sed "s/'/''/g")'));"
done

rm -f $TMPFILE
