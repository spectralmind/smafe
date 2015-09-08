#!/bin/bash

echo Starts backend for Live-Smint API

#fvt 1
#with segments
smafedistd.sh --live -f 1 -s --log /home/user3a/smafedistd.log --dbconf /home/user/smafe/package/smafedbconn-localhost-smafeuser.opt --livetopk 10

echo Exit code=$?            - 0 means success but also check logfile!
