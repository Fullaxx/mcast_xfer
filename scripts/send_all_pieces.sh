#!/bin/bash

export MTU=8975
export SLEEPDELAY=20

sudo /usr/bin/rl_knet_max.sh

while [ true ]; do
  for FILE in *.???; do
    mcastsend.dbg ${FILE}
    sleep 1
  done
done
