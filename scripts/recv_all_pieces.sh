#!/bin/bash

export BAILONMISS=1

if [ -z "${FILECOUNT}" ]; then
  >&2 echo "FILECOUNT is empty! I need to know how many files to expect!"
  exit 1
fi

COUNT="0"
COUNT=`ls -1 *.??? | wc -l`
while [ "${COUNT}" -lt "${FILECOUNT}" ]; do
  mcastrecv.exe
  COUNT=`ls -1 *.??? | wc -l`
done
