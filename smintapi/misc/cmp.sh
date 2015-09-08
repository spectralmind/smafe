#!/bin/bash

# script to compare two databases via the smintapi 
# needs: externalkeys file 
# todo: fix hardcoded apiurls 

  echo -n "" > diff.log
  echo -n "" > cmp.log

cat externalkeys | while read key; do
  enckey="$(perl -MURI::Escape -e 'print uri_escape($ARGV[0]);' "$key")"
  
  curl -s -H "Accept:application/xml" -k https://demo.spectralmind.com/smintapi/track_external_key/$enckey  | xml_pp | sed 's/id="[^"]*"/id=""/g' > new
  curl -s -H "Accept:application/xml" -k https://demo.spectralmind.com/smintapiold/track_external_key/$enckey  | xml_pp > old

  diff new old >> diff.log    
  if [ $? -eq 1 ] ; then
    echo diff detected: $key   $enckey >> cmp.log
  fi	
done

