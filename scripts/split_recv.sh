#!/bin/bash

DONE="0"
COUNT="0"
while [ "${DONE}" == "0" ]; do
  COUNT=$(( COUNT + 1))
  if [ "${COUNT}" -gt "675" ];   then missing_pieces.exe 2; fi
#  if [ "${COUNT}" -gt "17575" ]; then missing_pieces.exe 3; fi
  mcastrecv.exe
  if [ "$?" == "99" ]; then DONE="1"; fi
done
