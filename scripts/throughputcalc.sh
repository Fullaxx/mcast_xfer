#!/bin/bash

if [ "$#" -ne "2" ]; then
  echo "$0: <MTU> <DELAY>us"
  exit 1
fi

MTU="$1"
US="$2"

BPS=`calc "${MTU}-60" | awk '{print $1}'`
PPS=`calc "1e6/${US}" | awk '{print $1}'`
BW=`calc "(${PPS}*${BPS})/1e6" | awk '{print $1}'`

echo "${PPS} pkt/s @ ${BPS} B/pkt = ${BW} MB/s"
